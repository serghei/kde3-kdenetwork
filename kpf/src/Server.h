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

#ifndef KPF_SERVER_H
#define KPF_SERVER_H

#include <qdatetime.h>
#include <qstring.h>
#include <qhostaddress.h>

#include "Request.h"
#include "Response.h"

namespace KPF
{
  class WebServer;

  /**
   * Converses with a remote client. Handles requests and generates responses.
   * Bandwidth is controlled by parent (WebServer).
   */
  class Server : public QObject
  {
    Q_OBJECT

    public:

      /**
       * A Server can be in one of four states, enumerated here.
       */
      enum State { WaitingForRequest, WaitingForHeaders, Responding, Finished };

      /**
       * @param dir Root directory. Files not contained in this directory
       * and subdirectories thereof will not be server.
       *
       * @param followSymlinks If false, an expensive algorithm will ensure
       * that no symbolic links are present anywhere in the path from /
       * to the requested resource.
       *
       * @param socket The system's socket device, used for communication.
       *
       * @param parent A WebServer, which will manage this Server.
       */
      Server
        (
         const QString  & dir,
         bool             followSymlinks,
         int              socket,
         WebServer      * parent
        );

      /**
       * Free internal data and close connection if still open.
       */
      virtual ~Server();

      /**
       * @return address of client that has connected to us.
       */
      QHostAddress    peerAddress()   const;

      /**
       * @return Request object associated with this connection.
       * When persistent connections are used, the object may
       * differ depending on the current state.
       */
      Request         request()       const;

      /**
       * @return Response object associated with this connection.
       * When persistent connections are used, the object may
       * differ depending on the current state.
       */
      Response        response()      const;

      /**
       * @return number of bytes sent out over the socket since this object
       * was created.
       */
      ulong   output()        const;

      /**
       * @return current state.
       * @see State.
       */
      State           state()         const;

      /**
       * @return date and time this object was created.
       */
      QDateTime       birth()         const;

      /**
       * @return date and time all activity was completed.
       */
      QDateTime       death()         const;

      /**
       * @return number of bytes remaining to send to client.
       */
      ulong   bytesLeft()     const;

      /**
       * Send no more than maxBytes to the client.
       *
       * @return number of bytes sent.
       */
      ulong   write(ulong maxBytes);

      /**
       * Stop negotiating with client and sending data. Emit @ref finished.
       * Do not flush output.
       */
      void            cancel();

    protected slots:

      void slotReadyRead        ();
      void slotRead             ();
      void slotBytesWritten     (int);
      void slotConnectionClosed ();
      void slotTimeout          ();

    signals:

      void readyToWrite (Server *);
      void output       (Server *, ulong);
      void finished     (Server *);
      void response     (Server *);
      void request      (Server *);

    private:

      // Disable copying.

      Server(const Server &);
      Server & operator = (const Server &);

      enum FlushSelect
      {
        Flush,
        NoFlush
      };

      void setFinished      (FlushSelect);
      void writeLine        (const QString &);
      void prepareResponse  ();
      void respond          (uint code, ulong size = 0);
      bool readRequest      (const QString &);
      bool checkRequest     ();
      void handleRequest    ();
      void readHeaders      ();
      bool handleRange      (const ByteRange &);
      bool writeHeaderData  (ulong, ulong &);
      bool writeFileData    (ulong, ulong &);
      void reset            ();

      class Private;
      Private * d;
  };

} // End namespace KPF

#endif // KPF_SERVER_H
// vim:ts=2:sw=2:tw=78:et
