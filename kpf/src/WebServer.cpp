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

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// Qt includes
#include <qsocket.h>
#include <qdatetime.h>
#include <qtimer.h>

// KDE includes
#include "config.h"
#include <kconfig.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <dnssd/publicservice.h>

// Local includes
#include "Defines.h"
#include "Defaults.h"
#include "WebServerSocket.h"
#include "WebServer.h"
#include "Server.h"
#include "Utils.h"

namespace KPF
{
  static const uint SamplesPerSecond = 10;
  static const uint MaxBacklog = 1024;

  class WebServer::Private
  {
    public:

      Private()
        : socket              (0L),
          listenPort          (Config::DefaultListenPort),
          connectionLimit     (Config::DefaultConnectionLimit),
          bandwidthLimit      (Config::DefaultBandwidthLimit),
          lastTotalOutput     (0L),
          totalOutput         (0L),
          portContention      (true),
          paused              (false),
          followSymlinks      (Config::DefaultFollowSymlinks),
          customErrorMessages (false)
      {
      }

      ~Private()
      {
        delete socket;
      	delete service;
      	service = 0;
        socket = 0;
      }

      WebServerSocket * socket;
      uint              listenPort;
      uint              connectionLimit;
      QPtrList<Server>  serverList;
      QString           root;
      QString           serverName;
      QTimer            writeTimer;
      QTimer            resetOutputTimer;
      QTimer            bindTimer;
      QTimer            backlogTimer;
      ulong             bandwidthLimit;
      ulong             lastTotalOutput;
      ulong             totalOutput;
      bool              portContention;
      bool              paused;
      bool              followSymlinks;
      bool              customErrorMessages;
      QValueList<int>   backlog;
      DNSSD::PublicService* service;

  };

  WebServer::WebServer(const QString & root)
    : DCOPObject(QCString("WebServer_") + root.utf8()),
      QObject()
  {
    d = new Private;

    d->root = root;
    loadConfig();
    publish();
    
    connect(&d->bindTimer,        SIGNAL(timeout()), SLOT(slotBind()));
    connect(&d->writeTimer,       SIGNAL(timeout()), SLOT(slotWrite()));
    connect(&d->resetOutputTimer, SIGNAL(timeout()), SLOT(slotCheckOutput()));
    connect(&d->backlogTimer,     SIGNAL(timeout()), SLOT(slotClearBacklog()));

    d->bindTimer        .start(  0, true);
    d->resetOutputTimer .start(1000 / SamplesPerSecond, false);
  }


  WebServer::WebServer
    (
     const QString  & root,
     uint             listenPort,
     uint             bandwidthLimit,
     uint             connectionLimit,
     bool             followSymlinks,
     const QString &  serverName
    )
    : DCOPObject(QCString("WebServer_") + root.utf8()),
      QObject()
  {
    d = new Private;
    
    d->root             = root;

    d->listenPort       = listenPort;
    d->bandwidthLimit   = bandwidthLimit;
    d->connectionLimit  = connectionLimit;
    d->followSymlinks   = followSymlinks;
    d->serverName       = serverName;

    saveConfig();
    publish();
    connect(&d->bindTimer,        SIGNAL(timeout()), SLOT(slotBind()));
    connect(&d->writeTimer,       SIGNAL(timeout()), SLOT(slotWrite()));
    connect(&d->resetOutputTimer, SIGNAL(timeout()), SLOT(slotCheckOutput()));
    connect(&d->backlogTimer,     SIGNAL(timeout()), SLOT(slotClearBacklog()));

    d->bindTimer        .start(  0, true);
    d->resetOutputTimer .start(1000 / SamplesPerSecond, false);
  }

  WebServer::~WebServer()
  {
    killAllConnections();

    delete d;
    d = 0;
  }

  void WebServer::publish()
  { 
    d->service = new DNSSD::PublicService(d->serverName,"_http._tcp",d->listenPort);
    connect(d->service,SIGNAL(published(bool)),this,SLOT(wasPublished(bool)));
    d->service->publishAsync();
  }

  void WebServer::wasPublished(bool ok)
  {
    if(ok) {
      KMessageBox::information( NULL, i18n("Successfully published this new service to the network (ZeroConf)."), i18n( "Successfully Published the Service" ), "successfullypublished" );
      kpfDebug << "Published to dnssd successfully" << endl;
    }
    else {
      KMessageBox::information( NULL, i18n("Failed to publish this new service to the network (ZeroConf).  The server will work fine without this, however."), i18n( "Failed to Publish the Service" ), "failedtopublish" );
    }
  }
  
    void
  WebServer::slotBind()
  {
    if (0 != d->socket)
    {
      qWarning("Uhhh, socket isn't 0, but I'm told to bind ?");
      return;
    }

    d->socket = new WebServerSocket(d->listenPort, d->connectionLimit);

    d->portContention = !d->socket->ok();

    emit(contentionChange(d->portContention));

    if (!d->portContention)
      connect(d->socket, SIGNAL(connection(int)), SLOT(slotConnection(int)));

    else
    {
      delete d->socket;
      d->socket = 0;
      d->bindTimer.start(1000, true);
    }
  }

    void
  WebServer::slotConnection(int fd)
  {
    if (!d->backlog.isEmpty())
    {
      if (d->backlog.count() < MaxBacklog)
      {
        kpfDebug << "Adding this connection to the backlog." << endl;
        d->backlog.append(fd);
      }
      else
      {
        kpfDebug << "Backlog full. Ignoring this connection." << endl;
      }
      return;
    }

    if (!handleConnection(fd))
    {
      if (d->backlog.count() < MaxBacklog)
      {
        kpfDebug << "Adding this connection to the backlog." << endl;
        d->backlog.append(fd);
        d->backlogTimer.start(10, true);
      }
      else
      {
        kpfDebug << "Backlog full. Ignoring this connection." << endl;
      }
    }
  }

    bool
  WebServer::handleConnection(int fd)
  {
    if (d->paused)
    {
      kpfDebug << "Paused." << endl;
      return false;
    }

    if (d->serverList.count() >= d->connectionLimit)
    {
//      kpfDebug << "Hit connection limit." << endl;
      return false;
    }

    int on = 1;

    ::setsockopt
      (
        fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        ( char* )&on,
        sizeof( on ) );

    on = 0;

    ::setsockopt
      (
        fd,
        SOL_SOCKET,
        SO_LINGER,
        ( char* ) &on,
        sizeof( on ) );

    Server * s = new Server(d->root, d->followSymlinks, fd, this);

    connect
      (
       s,
       SIGNAL(output(Server *, ulong)),
       SLOT(slotOutput(Server *, ulong))
      );

    connect(s, SIGNAL(finished(Server *)),  SLOT(slotFinished(Server *)));
    connect(s, SIGNAL(request(Server *)),   SIGNAL(request(Server *)));
    connect(s, SIGNAL(response(Server *)),  SIGNAL(response(Server *)));

    d->serverList.append(s);

    connect
      (s, SIGNAL(readyToWrite(Server *)), SLOT(slotReadyToWrite(Server *)));

    emit(connection(s));

    return true;
  }

    void
  WebServer::restart()
  {
    d->bindTimer.stop();

    killAllConnections();
    delete d->socket;
    d->socket = 0;
    d->service->setServiceName(d->serverName);
    d->service->setPort(d->listenPort);
    d->bindTimer.start(0, true);
  }

    void
  WebServer::killAllConnections()
  {
    QPtrListIterator<Server> it(d->serverList);

    for (; it.current(); ++it)
      it.current()->cancel();

  }

    void
  WebServer::slotOutput(Server * s, ulong l)
  {
    emit(output(s, l));
  }

    void
  WebServer::slotFinished(Server * s)
  {
    emit(finished(s));
    d->serverList.removeRef(s);
    delete s;
    s = 0;
  }

    void
  WebServer::setBandwidthLimit(ulong l)
  {
    d->bandwidthLimit = l;
    saveConfig();
  }

    ulong
  WebServer::bandwidthLimit()
  {
    return d->bandwidthLimit;
  }

    void
  WebServer::setFollowSymlinks(bool b)
  {
    d->followSymlinks = b;
    saveConfig();
  }

    bool
  WebServer::followSymlinks()
  {
    return d->followSymlinks;
  }

    QString
  WebServer::root()
  {
    return d->root;
  }

    uint
  WebServer::connectionLimit()
  {
    return d->connectionLimit;
  }

    void
  WebServer::setConnectionLimit(uint i)
  {
    d->connectionLimit = i;
    saveConfig();
  }

    uint
  WebServer::listenPort()
  {
    return d->listenPort;
  }

    void
  WebServer::setListenPort(uint i)
  {
    d->listenPort = i;
    saveConfig();
  }

    bool
  WebServer::customErrorMessages()
  {
    return d->customErrorMessages;
  }

    void
  WebServer::setCustomErrorMessages(bool b)
  {
    d->customErrorMessages = b;
    saveConfig();
  }

    ulong
  WebServer::bytesLeft() const
  {
#if 0
    // Multiply the bandwidth limit by 10 so we can do 10 checks per second.

    ulong l =
      (d->bandwidthLimit * 10240) - (d->totalOutput - d->lastTotalOutput);
#endif

    ulong l =
      ulong(d->bandwidthLimit * (1024 / double(SamplesPerSecond)))
      - (d->totalOutput - d->lastTotalOutput);

    return l;
  }

    ulong
  WebServer::bandwidthPerClient() const
  {
    ulong l = 0L;

    if (!d->serverList.isEmpty())
    {
      l = bytesLeft() / d->serverList.count();
    }

    kpfDebug << l << endl;

    return l;
  }

    void
  WebServer::slotReadyToWrite(Server *)
  {
    d->writeTimer.stop();
    d->writeTimer.start(0, true);
  }

    void
  WebServer::slotWrite()
  {
    if (d->serverList.isEmpty())
      return;

    QPtrListIterator<Server> it(d->serverList);

    for (; it.current(); ++it)
    {
      if (0 == bytesLeft())
        break;

      Server * s = it.current();

      if (0 == s->bytesLeft())
        continue;

      ulong bytesAvailable = 0;

      if (0 == bandwidthPerClient())
        bytesAvailable = bytesLeft();
      else
        bytesAvailable = min(s->bytesLeft(), bandwidthPerClient());

      if (0 != bytesAvailable)
        d->totalOutput += s->write(bytesAvailable);
    }

    d->writeTimer.start(1000 / SamplesPerSecond, true);
  }

    void
  WebServer::slotCheckOutput()
  {
    emit(connectionCount(d->serverList.count()));
    ulong lastOutput = d->totalOutput - d->lastTotalOutput;
    emit(wholeServerOutput(ulong(lastOutput * SamplesPerSecond)));
    d->lastTotalOutput = d->totalOutput;
  }

    void
  WebServer::slotClearBacklog()
  {
//    kpfDebug << "WebServer::slotClearBacklog" << endl;

    if (!d->backlog.isEmpty())
    {
      uint currentBacklogCount = d->backlog.count();

      for (uint i = 0; i < currentBacklogCount; i++)
      {
        int fd = d->backlog.first();

        if (handleConnection(fd))
        {
          kpfDebug 
            << "Ah, we can now handle this connection. Removing from backlog."
            << endl;

          d->backlog.remove(d->backlog.begin());
        }
        else
        {
//          kpfDebug
//            << "Still can't handle this connection. Leaving in backlog"
//            << endl;

          break;
        }
      }
    }

    if (!d->backlog.isEmpty())
    {
      d->backlogTimer.start(10, true);
    }
  }

    uint
  WebServer::connectionCount()
  {
    return d->serverList.count();
  }

    void
  WebServer::pause(bool b)
  {
    if(b == d->paused) return;
    
    d->paused = b;
    if (b) d->service->stop();
    else d->service->publishAsync(); //published() should be already connected
    emit pauseChange(d->paused);
    saveConfig();
  }

    bool
  WebServer::paused()
  {
    return d->paused;
  }
    QString
  WebServer::serverName()
  {
    return d->serverName;
  }
    void
  WebServer::setServerName(const QString& serverName)
  {
    d->serverName=serverName;
  }

    bool
  WebServer::portContention()
  {
    return d->portContention;
  }

    void
  WebServer::set
  (
   uint   listenPort,
   ulong  bandwidthLimit,
   uint   connectionLimit,
   bool   followSymlinks,
   const QString& serverName
  )
  {
    d->listenPort           = listenPort;
    d->bandwidthLimit       = bandwidthLimit;
    d->connectionLimit      = connectionLimit;
    d->followSymlinks       = followSymlinks;
    d->serverName           = serverName;

    saveConfig();
  }

    void
  WebServer::loadConfig()
  {
    kpfDebug << "WebServer(" << d->root << "): Loading configuration" << endl;
    KConfig c(Config::name());

    c.setGroup(Config::key(Config::GroupPrefix) + d->root);

    d->listenPort =
      c.readUnsignedNumEntry
      (Config::key(Config::ListenPort),       d->listenPort);

    d->bandwidthLimit =
      c.readUnsignedNumEntry
      (Config::key(Config::BandwidthLimit),   d->bandwidthLimit);

    d->connectionLimit =
      c.readUnsignedNumEntry
      (Config::key(Config::ConnectionLimit),  d->connectionLimit);

    d->followSymlinks =
      c.readBoolEntry
      (Config::key(Config::FollowSymlinks),   d->followSymlinks);

    d->customErrorMessages =
      c.readBoolEntry
      (Config::key(Config::CustomErrors),     d->customErrorMessages);
      
    d->paused = 
      c.readBoolEntry
      (Config::key(Config::Paused),           d->paused);
      
    d->serverName = 
      c.readEntry
      (Config::key(Config::ServerName),       d->serverName);
      
  }

    void
  WebServer::saveConfig()
  {
    kpfDebug << "WebServer(" << d->root << "): Saving configuration" << endl;
    KConfig c(Config::name());

    c.setGroup(Config::key(Config::GroupPrefix) + d->root);

    c.writeEntry(Config::key(Config::ListenPort),      d->listenPort);
    c.writeEntry(Config::key(Config::BandwidthLimit),  d->bandwidthLimit);
    c.writeEntry(Config::key(Config::ConnectionLimit), d->connectionLimit);
    c.writeEntry(Config::key(Config::FollowSymlinks),  d->followSymlinks);
    c.writeEntry(Config::key(Config::CustomErrors),    d->customErrorMessages);
    c.writeEntry(Config::key(Config::Paused),          d->paused);
    c.writeEntry(Config::key(Config::ServerName),      d->serverName);

    c.sync();
  }

} // End namespace KPF

#include "WebServer.moc"
// vim:ts=2:sw=2:tw=78:et
