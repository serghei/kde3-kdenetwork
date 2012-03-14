/*
  KPF - Public fileserver for KDE

  Copyright 2001 Rik Hemsley (rikkus) <rik@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Defines.h"
#include "DirectoryLister.h"
#include "WebServer.h"
#include "Server.h"
#include "ServerPrivate.h"
#include "Utils.h"

#undef KPF_TRAFFIC_DEBUG

namespace KPF
{
  static const uint IncomingDataLimit = 8 * 1024;     // kB.
  static const uint Timeout           = 60 * 1000;    // seconds.
  static const uint MaxKeepAlive      = 20;           // transactions.

  Server::Server
    (
     const QString  & dir,
     bool             followSymlinks,
     int              socket,
     WebServer        * parent
    )
    : QObject(parent, "Server")
  {
    d = new Private;

    kpfDebug << "New server: " << d->id << endl;

    d->dir = dir;

    d->followSymlinks = followSymlinks;

    d->birth = QDateTime::currentDateTime();

    d->socket.setSocket(socket);

    connect(&(d->socket), SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    connect
      (
       &(d->socket),
       SIGNAL(bytesWritten(int)),
       SLOT(slotBytesWritten(int))
      );

    connect
      (
       &(d->socket),
       SIGNAL(connectionClosed()),
       SLOT(slotConnectionClosed())
      );

    connect
      (
       &(d->idleTimer),
       SIGNAL(timeout()),
       SLOT(slotTimeout())
      );

    connect
      (
       &(d->readTimer),
       SIGNAL(timeout()),
       SLOT(slotRead())
      );

    // If nothing happens for a bit, cancel ourselves.

    d->idleTimer.start(Timeout, true);
  }

  Server::~Server()
  {
    delete d;
    d = 0;
  }

    void
  Server::slotReadyRead()
  {
    kpfDebug << d->id << ":slotReadyRead" << endl;

    // DoS protection.

    d->dataRead += static_cast<uint>(d->socket.bytesAvailable());

    if (d->dataRead > IncomingDataLimit)
    {
      kpfDebug
        << d->id
        << ": Read would breach limit. Assuming DoS -> finished"
        << endl;

      setFinished(NoFlush /* Don't bother flushing socket */);
      return;
    }

    // Reset idle timer.

    d->idleTimer.start(Timeout, true);

    // Read all available data to incomingLineBuffer.

    while (d->socket.canReadLine())
    {
      kpfDebug << d->id << ": socket.canReadLine" << endl;

      QString line(d->socket.readLine().stripWhiteSpace());

#ifdef KPF_TRAFFIC_DEBUG
      kpfDebug
        << d->id
        << ": Adding line to incomingLineBuffer: "
        << line
        << endl;
#endif

      d->incomingLineBuffer.append(line);
    }

    if (!d->incomingLineBuffer.isEmpty())
    {
      kpfDebug
        << d->id
        << ": incomingLineBuffer isn't empty - calling slotRead directly"
        << endl;

      slotRead();
    }
    else
    {
      kpfDebug
        << d->id
        << ": incomingLineBuffer is empty. Nothing to do."
        << endl;
    }
  }

    void
  Server::slotRead()
  {
    kpfDebug << d->id << ": slotRead" << endl;

    if (d->incomingLineBuffer.isEmpty())
    {
      kpfDebug << d->id << ": incomingLineBuffer is empty !" << endl;
      return;
    }

    // There is data available in incomingLineBuffer.

    switch (d->state)
    {
      case WaitingForRequest:
        kpfDebug << d->id << ": I was waiting for a request" << endl;
        (void) readRequest(d->incomingLineBuffer.first());
        d->incomingLineBuffer.remove(d->incomingLineBuffer.begin());
        break;

      case WaitingForHeaders:
        kpfDebug << d->id << ": I was waiting for headers" << endl;
        readHeaders();
        break;

      case Responding:
      case Finished:
      default:
        kpfDebug << d->id << ": I was responding or finished" << endl;
        break;
    }
  }

    bool
  Server::readRequest(const QString & line)
  {
    ++d->requestCount;

#ifdef KPF_TRAFFIC_DEBUG
    kpfDebug
      << d->id
      << ": (request #" << d->requestCount << ") readRequest: `"
      << line << "'" << endl;
#endif

    QStringList l(QStringList::split(' ', line));

    // A request usually looks like METHOD PATH PROTOCOL but we don't
    // require PROTOCOL - we just assume HTTP/0.9 and act accordingly.

    if (l.count() == 2)
    {
      kpfDebug << d->id << ": readRequest: HTTP/0.9 ???" << endl;
      emit(request(this));
      d->state = Responding;
      respond(400);
      emit(readyToWrite(this));
      return false;
    }

    // The Request object handles setting parsing the strings we pass it here.
    // It converts GET/HEAD/whatever to an enum, fixes up the path and
    // converts the protocol string to a number.

    d->request.setMethod    (l[0]);
    d->request.setPath      (l[1]);
    d->request.setProtocol  (l.count() == 3 ? l[2] : QString::null);

    // Before we check the request, say we received it.

    emit(request(this));

    return checkRequest();
  }

    bool
  Server::checkRequest()
  {
    // We only handle METHOD of GET or HEAD.

    if (Request::Unsupported == d->request.method())
    {
      kpfDebug << d->id << ": readRequest: method unsupported" << endl;
      d->state = Responding;
      respond(501);
      emit(readyToWrite(this));
      return false;
    }

    // If there's .. or ~ in the path, we disallow. Either there's a mistake
    // or someone's trying to h@x0r us. I wouldn't have worried about ~
    // normally, because I don't do anything with it, so the resource would
    // simply not be found, but I'm worried that the QDir/QFile/QFileInfo
    // stuff might try to expand it, so I'm not taking any chances.

    if (d->request.path().contains("..") || d->request.path().contains('~'))
    {
      kpfDebug << d->id << ": readRequest: bogus path" << endl;
      d->state = Responding;
      respond(403);
      emit(readyToWrite(this));
      return false;
    }

    if (d->request.protocol() > 1.1)
    {
      if (d->request.protocol() >= 2.0)
      {
        kpfDebug
          << d->id
          << ": readRequest: unsupported protocol major number"
          << endl;

        d->request.setProtocol(1, 1);

        d->state = Responding;
        respond(505);
        emit(readyToWrite(this));
        return false;
      }
      else
      {
        kpfDebug
          << d->id
          << ": readRequest: unsupported protocol minor number"
          << endl;

        d->request.setProtocol(1, 1);
      }
    }

    if (d->request.protocol() >= 1.0)
    {
      kpfDebug
        << d->id
        << ": readRequest: need to wait for headers now"
        << endl;

      if (d->request.protocol() > 1.0)
      {
        d->request.setPersist(true);
      }

      d->state = WaitingForHeaders;
      d->readTimer.start(0, true);
    }
    else
    {
      kpfDebug
        << d->id
        << ": readRequest: immediate response"
        << endl;

      d->state = Responding;
      prepareResponse();
      emit(readyToWrite(this));
      return true;
    }

    return true;
  }

    void
  Server::readHeaders()
  {
    kpfDebug << d->id << ": readHeaders" << endl;

    // Pop lines from front of incomingLineBuffer and add to
    // incomingHeaderLineBuffer until we reach the end of the headers, when we
    // generate a response to the request.

    while (!d->incomingLineBuffer.isEmpty())
    {
      // This would be cleaner if there was a QValueQueue.
      // Basically we 'dequeue' the first line from incomingHeaderBuffer.

      QString line(d->incomingLineBuffer.first());
      d->incomingLineBuffer.remove(d->incomingLineBuffer.begin());

      // Unless the line is empty, this is (in theory) a header.

      if (!line.isEmpty())
      {
        kpfDebug << d->id << ": Header line: " << line << endl;
        d->incomingHeaderLineBuffer << line;
      }
      else
      {
        kpfDebug << d->id << ": Blank line - end of headers" << endl;

        // We have a buffer filled with all the header data received.
        // First parse those headers.

        d->request.parseHeaders(d->incomingHeaderLineBuffer);

        // Clear out the buffer because we won't need to use it again
        // and leaving all that data in memory is pointless.

        d->incomingHeaderLineBuffer.clear();

        // We've parsed the headers so the next thing we do is respond.

        d->state = Responding;
        prepareResponse();

        // When the response has been prepared, we're ready to write
        // some data back into that socket.

        kpfDebug << d->id << ": Ready to write" << endl;

        emit(readyToWrite(this));

        return;
      }
    }

    // Because we haven't found an empty line and therefore parsed
    // headers + returned, we must wait for more headers.

    kpfDebug
      << d->id
      << ": readHeaders: No lines left in header buffer."
      << " Setting state to WaitingForHeaders"
      << endl;

    d->state = WaitingForHeaders;
  }

    void
  Server::prepareResponse()
  {
    // The path to the requested resource is relative to our root.

    QString filename = d->dir + '/' + d->request.path();

    kpfDebug << d->id << ": filename: " << filename << endl;

    d->resource.setPath(d->dir, d->request.path());

    if (!d->resource.exists())
    {
      kpfDebug << d->id << ": Resource doesn't exist: %s" << filename << endl;

      // No file ? Perhaps we should give a directory listing.

      if (!(/*d->generateDirectoryListings && */ d->request.path() == "/"))
      {
        // Either index.html wasn't the file requested, or we're not supposed
        // to generated listings.

        respond(404);
        return;
      }
    }

    if (!d->followSymlinks && d->resource.symlink())
    {
      // If we're not supposed to follow symlinks and there's a symlink
      // somewhere on the path, deny.

      respond(403);
      return;
    }

    if (!d->resource.readable())
    {
      // Deny even HEAD for unreadable files.

      respond(403);
      return;
    }

//    if ((Request::Get == d->request.method()) && !d->resource.open())
    // Open resource even if we're asked for HEAD. We need to ensure
    // Content-Length is sent correctly.

    if (!d->resource.open())
    {
      // Couldn't open the file. XXX why not ?

      respond(403);
      return;
    }

    if (d->request.haveRange())
    {
      // There was a byte range specified in the request so handleRange()
      // to check that the range makes sense for the requested resource.

      kpfDebug << d->id << ": Byte range in request" << endl;

      if (!handleRange(d->request.range()))
      {
        // handleRange() takes care of sending the necessary response.
        return;
      }
    }
    else
    {
      kpfDebug
        << d->id
        << "No byte range in request."
        << endl;

      if (d->request.haveIfModifiedSince())
      {
        // If we saw an If-Modified-Since header and the resource hasn't
        // been modified since that date, we respond with '304 Not modified'

        if (toGMT(d->resource.lastModified()) <= d->request.ifModifiedSince())
        {
          kpfDebug
            << d->id
            << "Got IfModifiedSince and will respond with 304 (unmodified)"
            << endl;

          respond(304);
          // We will not serve the file, so don't the size.
        }
        else
        {
          kpfDebug
            << d->id
            << "Got IfModifiedSince and will serve whole file (modified)"
            << endl;

          // We will serve the file, so set the size.
          d->fileBytesLeft = d->resource.size();
        }
      }
      else if (d->request.haveIfUnmodifiedSince())
      {
        // As above, except the logic is reversed.

        if (toGMT(d->resource.lastModified()) > d->request.ifUnmodifiedSince())
        {
          kpfDebug 
            << d->id
            << "Got IfUnmodifiedSince and will respond with 412 (modified)"
            << endl;

          respond(412);
          // We not serve the file, so don't set the size.
        }
        else
        {
          kpfDebug
            << d->id
            << "Got IfUnmodifiedSince and will serve whole file (unmodified)"
            << endl;

          // We will serve the file, so set the size.
          d->fileBytesLeft = d->resource.size();
        }
      }
      else
      {
        // We will serve the file, so set the size.
        d->fileBytesLeft = d->resource.size();
      }
 
      // If we haven't set the response up yet, that means we are not using a
      // special response due to a modification time condition. Therefore we
      // are doing the 'usual' 200 response.

      if (0 == d->response.code())
        respond(200, d->fileBytesLeft);
    }

    kpfDebug
      << d->id
      << "Done setting up response. Code will be "
      << responseName(d->response.code())
      << endl;


    // Send some headers back to the client, but only if the protocol
    // they asked us to use is new enough to require this.

    if (d->request.protocol() >= 1.0)
    {
      writeLine("Server: kpf");
      writeLine("Date: "            + dateString());
      writeLine("Last-Modified: "   + dateString(d->resource.lastModified()));
      writeLine("Content-Type: "    + d->resource.mimeType());

      // Generate a Content-Range header if we are sending partial content.

      if (206 == d->response.code())
      {
        QString line = "Content-Range: bytes ";

        line += QString::number(d->request.range().first());

        line += '-';

        if (d->request.range().haveLast())
          line += QString::number(d->request.range().last());
        else
          line += QString::number(d->resource.size() - 1);

        line += '/';

        line += QString::number(d->resource.size());

        writeLine(line);
      }

      writeLine("Content-Length: "  + QString::number(d->fileBytesLeft));
    }

    if (d->requestCount >= MaxKeepAlive && d->request.protocol() >= 1.1)
    {
      // We have made many transactions on this connection. Time to
      // give up and let the client re-connect. If we don't do this,
      // they could keep this connection open indefinitely.

      writeLine("Connection: close");
    }
    else if (d->request.protocol() == 1.0)
    {
      // wget seems broken. If it sends keep-alive, it hangs waiting for
      // nothing at all. Ignore its keep-alive request.
      writeLine("Connection: close");
    }
    else if (d->request.protocol() == 1.1) {
      writeLine("Connection: keep-alive");
    }

    // End of headers so send a newline.

    if (d->request.protocol() >= 1.0)
    {
      writeLine("");
    }
  }

    bool
  Server::handleRange(const ByteRange & r)
  {
    // Here we check if the given ByteRange makes sense for the
    // requested resource.

    // Is the range just plain broken ?

    if (!r.valid())
    {
      kpfDebug << d->id << ": Invalid byte range" << endl;
      respond(416);
      return false;
    }

    // Does the range start before the end of our resource ?

    else if (r.first() > d->resource.size())
    {
      kpfDebug << d->id << ": Range starts after EOF" << endl;
      respond(416);
      return false;
    }

    // Does the range end after the end of our resource ?

    else if (r.haveLast() && r.last() > d->resource.size())
    {
      kpfDebug << d->id << ": Range end after EOF" << endl;
      respond(416);
      return false;
    }

    // Ok, in theory the range should be satisfiable ...

    else
    {
      // ... but maybe we can't seek to the start of the range.

      if (!d->resource.seek(r.first()))
      {
        kpfDebug << d->id << ": Invalid byte range (couldn't seek)" << endl;
        // Should this be 501 ?
        respond(416);
        return false;
      }

      kpfDebug << d->id << ": Ok, setting fileBytesLeft" << endl;

      // Work out how many bytes are left to send to the client. Careful
      // with the off-by-one errors here, eh ?

      if (r.haveLast())
      {
        d->fileBytesLeft = r.last() + 1 - r.first();
      }
      else
      {
        d->fileBytesLeft = d->resource.size() - r.first();
      }

      kpfDebug << d->id << ": fileBytesLeft = "
        << d->fileBytesLeft << "d" << endl;

      respond(206, d->fileBytesLeft);
    }

    return true;
  }

    void
  Server::slotBytesWritten(int i)
  {
    // Don't you just love it when people forget 'unsigned' ?

    if (i > 0)
      d->bytesWritten += i;

    emit(output(this, i));

    // Reset idle timer.
    d->idleTimer.start(Timeout, true);
  }

    void
  Server::slotConnectionClosed()
  {
    kpfDebug << d->id << ": slotConnectionClosed -> finished" << endl;
    setFinished(Flush);
  }

    void
  Server::writeLine(const QString & line)
  {
    // Fill a buffer. We're not allowed to send anything out until our
    // controller calls write().

    QCString s(line.utf8() + "\r\n");

    d->headerBytesLeft      += s.length();
    d->outgoingHeaderBuffer += s;
  }

    void
  Server::cancel()
  {
    kpfDebug << d->id << ": cancel -> finished" << endl;
    setFinished(NoFlush);
  }

    void
  Server::respond(uint code, ulong fileSize)
  {
    // Set the code of our Response object.

    d->response.setCode(code);

    // Request from the Response object the text that should be sent
    // back to the client.

    QCString s(d->response.text(d->request));

    // Tell our Response object how long it will be in total (it doesn't
    // know its total size until we tell it about the resource size.)

    d->response.setSize(s.length() + fileSize);

    // Tell the world we've finished setting up our response.

    emit(response(this));

    // Add the response text to the outgoing header buffer.

    d->headerBytesLeft      += s.length();
    d->outgoingHeaderBuffer += s;
  }

    void
  Server::setFinished(FlushSelect flushSelect)
  {
    if (Finished == d->state) // Already finished.
      return;

    d->state = Finished;

    kpfDebug
      << d->id
      << ": finished("
      << (Flush == flushSelect ? "flush" : "no flush")
      << ")"
      << endl;

    if (Flush == flushSelect)
      d->socket.flush();

    d->socket.close();

    d->death = QDateTime::currentDateTime();

    emit(finished(this));
  }

    QHostAddress
  Server::peerAddress() const
  {
    return d->socket.peerAddress();
  }

    ulong
  Server::bytesLeft() const
  {
    // Return the combined size of the two output buffers.

    return d->headerBytesLeft + d->fileBytesLeft;
  }

    ulong
  Server::write(ulong maxBytes)
  {
    // We must be in 'Responding' state here. If not, there's a problem
    // in the code.

    if (Responding != d->state)
    {
      kpfDebug << d->id << ": write() but state != Responding -> finished";
      setFinished(Flush);
      return 0;
    }

    // If the socket has been closed (e.g. the remote end hung up)
    // then we just give up.

    if (QSocket::Connection != d->socket.state())
    {
      kpfDebug << d->id << ": Socket closed by client -> finished" << endl;
      setFinished(Flush);
      return 0;
    }

    kpfDebug << d->id << ": Response code is " << d->response.code() << " ("
      << responseName(d->response.code()) << ")" << endl;

    ulong bytesWritten = 0;

    // Write header data.

    ulong headerBytesWritten = 0;

    if (!writeHeaderData(maxBytes, headerBytesWritten))
    {
      return 0;
    }

    maxBytes      -= headerBytesWritten;
    bytesWritten  += headerBytesWritten;

    // If we are only sending headers (response code != 2xx or request type
    // was HEAD) or we reached the end of the file we were sending, give up.

    if (d->response.code() < 200 || d->response.code() > 299)
    {
      kpfDebug << d->id << ": We are only sending headers -> finished" << endl;

      // If we're sending 'Not modified' then we don't need to drop
      // the connection just yet.

      if (d->response.code() == 304 && d->request.persist())
      {
        kpfDebug
          << d->id
          << ": 304 and persist. Not dropping connection yet."
          << endl;

        reset();
      }
      else
      {
        setFinished(Flush);
      }

      return bytesWritten;
    }

    // Just HEAD ? Ok, then if we're set to persistent mode we go back
    // and wait for another request. Otherwise we're done and can go home.

    if (Request::Head == d->request.method())
    {
      if (d->request.persist())
      {
        reset();
      }
      else
      {
        setFinished(Flush);
      }

      return bytesWritten;
    }

    // If we've written our limit then wait until next time.

    if (0 == maxBytes)
    {
      return bytesWritten;
    }

    // Write resource data.

    ulong fileBytesWritten = 0;

    // writeFileData() returns true if the op was successful and also
    // returns the number of bytes written via the second parameter.

    if (!writeFileData(maxBytes, fileBytesWritten))
    {
      return 0;
    }

    kpfDebug << "Wrote " << fileBytesWritten << " from file" << endl;

    maxBytes      -= fileBytesWritten;
    bytesWritten  += fileBytesWritten;

    // Did we finish sending the resource data ?

    if (0 == d->fileBytesLeft)
    {
      kpfDebug << d->id << ": No bytes left to write. Closing file." << endl;

      d->resource.close();

      // If we're in persistent mode, don't quit just yet.

      if (d->requestCount < MaxKeepAlive && d->request.persist())
      {
        kpfDebug
          << d->id
          << ": Request included Keep-Alive, so we set state"
          << " to WaitingForRequest and don't send finished()"
          << endl;

        reset();
      }
      else
      {
        kpfDebug
          << d->id
          << ": No keep-alive or hit MaxKeepAlive, so finished."
          << endl;

        setFinished(Flush);
      }
    }
    else
    {
      // Ok, we have some data to send over the socket. Tell the world.

      kpfDebug
        << d->id
        << "Still have data left to send."
        << endl;

      emit(readyToWrite(this));
    }

    return bytesWritten;
  }


    bool
  Server::writeHeaderData(ulong max, ulong & bytesWritten)
  {
    // Is there some header data left to write ?

    if (0 == d->headerBytesLeft)
      return true;

    // Calculate where to start reading from the buffer.

    uint headerPos =
      d->outgoingHeaderBuffer.length() - d->headerBytesLeft;

    // Calculate how many bytes we should write this session.

    uint bytesToWrite = min(d->headerBytesLeft, max);

    // Calculate how many bytes we _may_ write.

    bytesToWrite = min(bytesToWrite, d->socket.outputBufferLeft());

    // Get a pointer to the data, offset by the position we start reading.

    char * data = d->outgoingHeaderBuffer.data() + headerPos;

    // Write the data, or at least as much as the socket buffers will
    // take, and remember how much we wrote.

    int headerBytesWritten = d->socket.writeBlock(data, bytesToWrite);

    // <rant>
    //   Using -1 to signal an error is fucking evil.
    //   Return false instead or add a 'bool & ok' parameter.
    //   If you're not going to use exceptions, at least don't use
    //   crap C conventions for error handling.
    // </rant>

    if (-1 == headerBytesWritten)
    {
      // Socket error.

      kpfDebug << d->id << ": Socket error -> finished" << endl;
      setFinished(Flush);
      return false;
    }

#ifdef KPF_TRAFFIC_DEBUG
    kpfDebug
      << d->id
      << ": Wrote header data: `"
      << d->outgoingHeaderBuffer.left(headerPos)
      << "'"
      << endl;
#endif

    // Subtract the number of bytes we wrote from the number of bytes
    // left to write.

    bytesWritten        += headerBytesWritten;
    d->headerBytesLeft  -= headerBytesWritten;

    // We may be doing a long file send next, so clear the header buffer
    // because we don't need that data hanging around in memory anymore.

    if (0 == d->headerBytesLeft)
      d->outgoingHeaderBuffer.resize(0);

    return true;
  }

    bool
  Server::writeFileData(ulong maxBytes, ulong & bytesWritten)
  {
    // Nothing left in the file ?

    if (d->resource.atEnd())
    {
      d->resource.close();
      kpfDebug << d->id << ": file at end -> finished" << endl;
      setFinished(Flush);
      return false;
    }

    // Calculate how much data we may write this session.
    // If none, give up.

    uint bytesToWrite = min(d->fileBytesLeft, maxBytes);

    if (0 == bytesToWrite)
      return true;

    bytesToWrite = min(bytesToWrite, d->socket.outputBufferLeft());

    QByteArray a(bytesToWrite);

    if (0 == bytesToWrite)
      return true;

    // Read some data (maximum = bytesToWrite) from the file.

    int fileBytesRead = d->resource.readBlock(a.data(), bytesToWrite);

    // Write that data to the socket and remember how much was actually
    // written (may be less than requested if socket buffers are full.)

    int fileBytesWritten = d->socket.writeBlock(a.data(), fileBytesRead);

    // Was there an error writing to the socket ?

    if (-1 == fileBytesWritten)
    {
      // Socket error.
      kpfDebug << d->id << ": Socket error -> finished" << endl;
      d->resource.close();
      setFinished(Flush);
      return false;
    }

#ifdef KPF_TRAFFIC_DEBUG
    kpfDebug
      << d->id
      << ": Wrote file data: `"
      << QCString(a.data(), fileBytesWritten)
      << "'"
      << endl;
#endif

    // We should have been able to write the full amount to the socket,
    // because we tested d->socket.outputBufferLeft(). If we didn't
    // manage to write that much, either we have a bug or QSocket does.

    if (fileBytesWritten < fileBytesRead)
    {
      kpfDebug << d->id << ": Short write !" << endl;
      d->resource.close();
      setFinished(Flush);
      return false;
    }

    // Subtract the amount of bytes written from the number left to write.

    bytesToWrite      -= fileBytesWritten;
    bytesWritten      += fileBytesWritten;
    d->fileBytesLeft  -= fileBytesWritten;

    return true;
  }

    void
  Server::slotTimeout()
  {
    kpfDebug << d->id << ": Timeout -> finished" << endl;
    setFinished(NoFlush);
  }

    Request
  Server::request() const
  {
    return d->request;
  }

    Response
  Server::response() const
  {
    return d->response;
  }

    ulong
  Server::output() const
  {
    return d->bytesWritten;
  }

    Server::State
  Server::state() const
  {
    return d->state;
  }

    QDateTime
  Server::birth() const
  {
    return d->birth;
  }

    QDateTime
  Server::death() const
  {
    return d->death;
  }

    void
  Server::reset()
  {
    kpfDebug << d->id << ": Resetting for another request" << endl;

    d->request  .clear();
    d->response .clear();
    d->resource .clear();

    d->state = WaitingForRequest;
    d->readTimer.start(0, true);
  }

} // End namespace KPF

#include "Server.moc"
// vim:ts=2:sw=2:tw=78:et
