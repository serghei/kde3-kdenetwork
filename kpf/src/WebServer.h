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

#ifndef KPF_WEB_SERVER_H
#define KPF_WEB_SERVER_H

#include <dcopobject.h>
#include <qserversocket.h>

#include "Defaults.h"
#include "Request.h"
#include "Response.h"

namespace KPF
{
  class Server;

  /**
   * Listens on a port for incoming connections.
   * Creates and manages Server objects.
   * Manages bandwidth limit, dealing out bandwidth to Server objects.
   * Maintains concurrent connection limit, using a backlog to queue incoming
   * connections which cannot be served immediately.
   */
  class WebServer : public QObject, virtual public DCOPObject
  {
    K_DCOP
    Q_OBJECT

    public:

      /**
       * Only root dir specified - this causes an immediate loadConfig.
       *
       * @param root Virtual root directory for servers. Passed to all created
       *             Server objects, which much ensure that only files from the root and
       *             its child directories are served.
       */
      WebServer(const QString & root);

      /**
       * @param root Virtual root directory for servers. Passed to all created
       *             Server objects, which much ensure that only files from the root and
       *             its child directories are served.
       */
      WebServer
        (
          const QString  & root,
          uint             listenPort,
          uint             bandwidthLimit,
          uint             connectionLimit,
          bool             followSymlinks,
          const QString &  serverName
        );

      virtual ~WebServer();

      /**
       * Load the configuration, but do not kill existing connections even if
       * listen port is changed. Do not change listen port yet - only when
       * asked to restart.
       */
      void loadConfig();

    k_dcop:

      /**
       * @return virtual root.
       */
      QString root();

      /**
       * @return server name
       */
      QString serverName();
      
      /**
       * @return amount of bytes that may be sent out per second, in total
       * (adding the output of all contained Server objects.)
       */
      ulong   bandwidthLimit();

      /**
       * @return number of concurrent connections that will be served (by
       * creating Server objects. More connections may wait in a backlog.)
       */
      uint    connectionLimit();

      /**
       * @return port on which to listen for incoming connections.
       */
      uint    listenPort();

      /**
       * @return true if requests may include symbolic links in their path.
       */
      bool    followSymlinks();

      /**
       * @return true if custom error messages (set by the user) should be
       * sent.
       */
      bool    customErrorMessages();

      /**
       * Set the maximum amount of bytes that may be sent out per second, in
       * total (adding the output of all contained Server objects.)
       */
      void setBandwidthLimit      (ulong);

      /**
       * Set the number of concurrent connections that will be served (by
       * creating Server objects. More connections may wait in a backlog.)
       */
      void setConnectionLimit     (uint);

      /**
       * Set the port on which to listen for incoming connections. Does not
       * take effect until restart() is called. 
       */
      void setListenPort          (uint);

      /**
       * Set server name
       */
      void setServerName          (const QString&);
         
      /**
       * Set whether requests may include symbolic links in their path.
       */
      void setFollowSymlinks      (bool);

      /**
       * Set whether custom error messages (set by the user) should be
       * sent.
       */
      void setCustomErrorMessages (bool);

      /**
       * Convenience method for setting many attributes at once.
       */
      void set
        (
         uint   listenPort,
         ulong  bandwidthLimit,
         uint   connectionLimit,
         bool   followSymlinks,
	 const QString& serverName
        );

      /**
       * Kill all connections, stop listening on port, and start listening on
       * (possibly different) port.
       */
      void restart();

      /**
       * Start / stop accepting new connections.
       */
      void pause(bool);

      /**
       * @return true if no new connections are accepted.
       */
      bool paused();

      /**
       * @return true if this WebServer is unable to listen on the requested
       * port.
       */
      bool portContention();

      /**
       * @return number of Server objects serving requests.
       */
      uint connectionCount();

    protected slots:

      /**
       * Called repeatedly by a timer when this WebServer is in contention for
       * its listen port.
       */
      void slotBind           ();

      /**
       * Called by contained socket object when a new incoming connection is
       * made.
       */
      void slotConnection     (int);

      /**
       * Called by a Server when it has finished all transactions and is ready
       * to die.
       */
      void slotFinished       (Server *);

      /**
       * Called by a Server when it has sent data to the remote client.
       */
      void slotOutput         (Server *, ulong);

      /**
       * Called by a Server when it wishes to send data to the remote client.
       */
      void slotReadyToWrite   (Server *);

      /**
       * Called regularly by a timer to start output allocation (to Server
       * objects.)
       */
      void slotWrite          ();

      /**
       * Called regularly by a timer to check output for current time slice.
       */
      void slotCheckOutput    ();

      /**
       * Called regularly by a timer to handle connections queued in the
       * backlog.
       */
      void slotClearBacklog   ();

      /**
       * Called when this succesfully publishes via zeroconf, or there was an
       * error doing so.
       */
      void wasPublished(bool ok);
      
    protected:

      /**
       * Attempt to create a Server to handle a new connection.
       * @param fd file descriptor.
       */
      bool handleConnection(int fd);

      void saveConfig();

    signals:

      /**
       * @param bytes number of bytes sent by this server during the last
       * time slice.
       */
      void wholeServerOutput  (ulong bytes);

      /**
       * Emitted when a Server object has received a request from its remote
       * client.
       */
      void request            (Server *);

      /**
       * Emitted when a Server object has created a response which it wishes
       * to send to its remote client.
       */
      void response           (Server *);

      /**
       * Emitted when a Server object when data has been send to remote client.
       * @param bytes number of bytes sent to remote client.
       */
      void output             (Server *, ulong bytes);

      /**
       * Emitted when a new Server has been created.
       */
      void connection         (Server *);

      /**
       * Emitted when a Server has finished all transactions.
       */
      void finished           (Server *);

      /**
       * Emitted when this WebServer is now contending, or has stopped
       * contending, its listen port.
       */
      void contentionChange   (bool);

      /**
       * Emitted when this WebServer is now accepting, or has stopped
       * accepting, incoming connections.
       */
      void pauseChange        (bool);

      /**
       * Emitted when the number active Server objects (served connections)
       * changes.
       */
      void connectionCount    (uint);

    private:

      /**
       * @return number of bytes that may be sent out. Changes over time,
       * depending on bandwidth limit.
       */
      ulong bytesLeft()           const;

      /**
       * @return number of bytes that may be sent out by each Server, with the
       * total split between the existing Server objects.
       */
      ulong bandwidthPerClient()  const;


      /** publish this to dns-sd (zeroconf)
       */
      void publish();
      /**
       * Cause all existing Server objects to close their connections by
       * calling Server::cancel.
       */
      void killAllConnections();

      class Private;
      Private * d;
  };

} // End namespace KPF

#endif // WEB_SERVER_H
// vim:ts=2:sw=2:tw=78:et
