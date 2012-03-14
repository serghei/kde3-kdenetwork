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
#include "WebServer.h"
#include "WebServerManager.h"
#include "KPFInterface.h"

KPFInterface::KPFInterface()
  : DCOPObject("KPFInterface")
{
  // Empty.
}

KPFInterface::~KPFInterface()
{
  // Empty.
}

  QStringList
KPFInterface::serverRootList()
{
  QList<KPF::WebServer> l(KPF::WebServerManager::instance()->serverListLocal());

  QStringList ret;

  for (QListIterator<KPF::WebServer> it(l); it.current(); ++it)
    ret << it.current()->root();

  return ret;
}

  bool
KPFInterface::createServer
(
 QString  root,
 uint     port,
 uint     bandwidthLimit,
 uint     connectionLimit,
 bool     followSymlinks
)
{
  kpfDebug << "KPFInterface::createServer(" << root << ", " <<
    port << ", " << bandwidthLimit << ", " << connectionLimit << ", "
    << (followSymlinks ? "true" : "false") << ")" << endl;

  KPF::WebServer * s =
    KPF::WebServerManager::instance()->createServer
    (
     root,
     port,
     bandwidthLimit,
     connectionLimit,
     followSymlinks
    );

  if (0 == s)
  {
    kpfDebug << "KPFInterface::createServer(): failed" << endl;
    return false;
  }
  else
  {
    kpfDebug << "KPFInterface::createServer(): ok" << endl;
    return true;
  }
}

  bool
KPFInterface::disableServer(QString root)
{
  kpfDebug << "KPFInterface::disableServer(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->disableServer(root);
}

  bool
KPFInterface::restartServer(QString root)
{
  kpfDebug << "KPFInterface::restartServer(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->restartServer(root);
}

  bool
KPFInterface::reconfigureServer(QString root)
{
  kpfDebug << "KPFInterface::reconfigureServer(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->reconfigureServer(root);
}

  bool
KPFInterface::pauseServer(QString root)
{
  kpfDebug << "KPFInterface::pauseServer(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->pauseServer(root, true);
}

  bool
KPFInterface::unpauseServer(QString root)
{
  kpfDebug << "KPFInterface::unpauseServer(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->pauseServer(root, false);
}

  bool
KPFInterface::serverPaused(QString root)
{
  kpfDebug << "KPFInterface::serverPaused(" << root << ")" << endl;
  return KPF::WebServerManager::instance()->serverPaused(root);
}
// vim:ts=2:sw=2:tw=78:et
