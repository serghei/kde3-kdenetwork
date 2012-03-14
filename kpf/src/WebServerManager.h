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

#ifndef KPF_WEB_SERVER_MANAGER_H
#define KPF_WEB_SERVER_MANAGER_H

#include <dcopobject.h>
#include <dcopref.h>

#include "Defaults.h"

#include <qptrlist.h>

namespace KPF
{
  class WebServer;

  /**
   * Singleton, encapsulating a set of WebServer objects. Handles
   * creating WebServer objects at startup (based on settings) and
   * on demand. Destroys WebServer objects on demand.
   */
  class WebServerManager : public QObject, virtual public DCOPObject
  {
    Q_OBJECT
    K_DCOP

    public:

      static WebServerManager * instance();

      /**
       * Calls delete(this).
       */
      void shutdown();

      /**
       * @return a list of pointers to WebServer objects managed
       * by this object.
       */
      QPtrList<WebServer> serverListLocal();

      /**
       * @return a pointer to a new WebServer object, with the root
       * as specified, or 0 if creation was impossible. Updates
       * the configuration.
       */
      WebServer * createServerLocal
      (
       const QString  & root,
       uint             listenPort,
       uint             bandwidthLimit  = Config::DefaultBandwidthLimit,
       uint             connectionLimit = Config::DefaultConnectionLimit,
       bool             followSymlinks  = Config::DefaultFollowSymlinks,
       const QString  & serverName = QString::null
      );

      /**
       * Disables a WebServer and updates the configuration.
       */
      bool disableServer(const QString & root);

      /**
       * Loads the configuration.
       * Creates WebServer objects to match the configuration and
       * ensures each object loads its configuration.
       */
      void loadConfig();

      /**
       * Saves the configuration.
       * Also ensures each WebServer object saves its configuration.
       */
      void saveConfig() const;

      /**
       * Find a WebServer or return 0.
       */
      WebServer * server(const QString & root);

      /**
       * Ask a server to re-read its configuration.
       */
      bool reconfigureServer(const QString & root);

      /**
       * Pause/unpause a server.
       */
      bool pauseServer(const QString & root, bool);

      /**
       * @return whether the server is paused.
       */
      bool serverPaused(const QString & root);

      /**
       * Restart a server.
       */
      bool restartServer(const QString & root);

      /**
       * @return if a Server object with the specified root exists. Handles
       * the two possible variations of trailing slash, i.e. existing and not
       * existing.
       */
      bool hasServer(const QString & root);

      uint nextFreePort() const;

    k_dcop:

      QValueList<DCOPRef> serverList();

      DCOPRef createServer
        (
         QString  root,
         uint     listenPort,
         uint     bandwidthLimit,
         uint     connectionLimit,
         bool     followSymlinks,
	 QString  serverName
        );

      void disableServer(DCOPRef);

      void quit();

    protected:

      /**
       * Not used, as this is a singleton.
       */
      WebServerManager();

      virtual ~WebServerManager();

    signals:

      void serverCreated(WebServer *);
      void serverDisabled(WebServer *);

    private:

      static WebServerManager * instance_;

      void load();
      QPtrList<WebServer> serverList_;
  };

} // End namespace KPF

#endif // WEB_SERVER_MANAGER_H
// vim:ts=2:sw=2:tw=78:et
