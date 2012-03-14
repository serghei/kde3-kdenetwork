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

// KDE includes
#include <kconfig.h>
#include <kglobal.h>
#include <kapplication.h>

// Local includes
#include "Defines.h"
#include "WebServerManager.h"
#include "WebServer.h"
#include "WebServer_stub.h"

namespace KPF
{
  WebServerManager * WebServerManager::instance_ = 0L;

    WebServerManager *
  WebServerManager::instance()
  {
    if (0 == instance_)
      instance_ = new WebServerManager;

    return instance_;
  }

    void
  WebServerManager::shutdown()
  {
    delete instance_;
    instance_ = 0;
  }

  WebServerManager::WebServerManager()
    : DCOPObject("WebServerManager"),
      QObject()
  {
    serverList_.setAutoDelete(true);
  }

  WebServerManager::~WebServerManager()
  {
    // Empty.
  }

    QPtrList<WebServer>
  WebServerManager::serverListLocal()
  {
    return serverList_;
  }

    WebServer *
  WebServerManager::createServerLocal
  (
   const QString  & root,
   uint             listenPort,
   uint             bandwidthLimit,
   uint             connectionLimit,
   bool             followSymlinks,
   const QString  & serverName
  )
  {
    if (0 != server(root))
      return 0;
    if ( listenPort == 0)
      listenPort = nextFreePort();
    WebServer * server =
      new WebServer
      (
       root,
       listenPort,
       bandwidthLimit,
       connectionLimit,
       followSymlinks,
       serverName
      );

    serverList_.append(server);

    saveConfig();

    emit(serverCreated(server));

    return server;
  }

    void
  WebServerManager::loadConfig()
  {
    KConfig config(Config::name());

    config.setGroup("General");

    QStringList serverRootList = config.readListEntry("ServerRootList");

    QStringList::ConstIterator it;

    for (it = serverRootList.begin(); it != serverRootList.end(); ++it)
    {
      WebServer * s = new WebServer(*it);
      serverList_.append(s);
      s->loadConfig();
      emit(serverCreated(s));
    }
  }

    void
  WebServerManager::saveConfig() const
  {
    KConfig config(Config::name());

    config.setGroup("General");

    QPtrListIterator<WebServer> it(serverList_);

    QStringList serverRootList;

    for (; it.current(); ++it)
      serverRootList << it.current()->root();

    config.writeEntry("ServerRootList", serverRootList);

    config.sync();
  }

    WebServer *
  WebServerManager::server(const QString & root)
  {
    QPtrListIterator<WebServer> it(serverList_);

    for (; it.current(); ++it)
    {
      kpfDebug << "WebServerManager::server(): found root of " <<
        "\"" << it.current()->root() << "\"" << endl;

      if (it.current()->root() == root)
      {
        kpfDebug
          << "WebServerManager::server(" << root << "): found" << endl;
        return it.current();
      }
    }

    kpfDebug
      << "WebServerManager::server(" << root << "): not found" << endl;
    return 0;
  }

    bool
  WebServerManager::disableServer(const QString & root)
  {
    WebServer * existing = server(root);

    if (0 == existing)
    {
      return false;
    }
    else
    {
      emit(serverDisabled(existing));
      // Auto-deleted by list.
      serverList_.removeRef(existing);
      saveConfig();
      return true;
    }
  }

    QValueList<DCOPRef>
  WebServerManager::serverList()
  {
    QValueList<DCOPRef> l;

    QPtrListIterator<WebServer> it(serverList_);

    for (; it.current(); ++it)
      l << DCOPRef(it.current());

    return l;
  }

    DCOPRef
  WebServerManager::createServer
  (
   QString  root,
   uint     listenPort,
   uint     bandwidthLimit,
   uint     connectionLimit,
   bool     followSymlinks,
   QString  serverName
  )
  {
    WebServer * server = createServerLocal
      (root, listenPort, bandwidthLimit, connectionLimit, followSymlinks, serverName);

    if (0 == server)
      return DCOPRef();
    else
      return DCOPRef(server);
  }

    void
  WebServerManager::disableServer(DCOPRef serverRef)
  {
    if (serverRef.isNull())
      return;

    WebServer_stub webServer
      (serverRef.app(), serverRef.object());

    QString root = webServer.root();

    if (DCOPStub::CallFailed == webServer.status())
    {
      kpfDebug << "Real shitty mess here" << endl;
      return;
    }

    bool ok = disableServer(root);

    if (!ok)
    {
      kpfDebug << "Definitely a real shitty mess here" << endl;
      return;
    }
  }

    void
  WebServerManager::quit()
  {
//    kapp->quit();
  }

    bool
  WebServerManager::hasServer(const QString & s)
  {
    QString root(s);

    if ('/' == root.at(root.length() - 1))
    {
      root.truncate(root.length() - 1);
    }

    return (0 != server(root) || 0 != server(root + "/"));
  }

  uint WebServerManager::nextFreePort() const
  {
    for (uint port = Config::DefaultListenPort; port < 65536; ++port)
    {
      bool ok = true;

      for (QPtrListIterator<WebServer> it(serverList_); it.current(); ++it)
      {
        if (it.current()->listenPort() == port)
        {
          ok = false;
          break;
        }
      }

      if (ok)
      {
        return port;
      }
    }

    // Not much we can do.
    return Config::DefaultListenPort;
  }

} // End namespace KPF

#include "WebServerManager.moc"
// vim:ts=2:sw=2:tw=78:et
