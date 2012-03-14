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

#ifndef KPF_ACTIVE_MONITOR_H
#define KPF_ACTIVE_MONITOR_H

#include <qmap.h>
#include <qtimer.h>
#include <qwidget.h>

class QListView;
class QPainter;
class QPushButton;

namespace KPF
{
  class WebServer;
  class Server;
  class ActiveMonitorItem;

  /**
   * Shows a list of ActiveMonitorItem objects.
   *
   * Proxies signals from Server objects to ActiveMonitorItem objects.
   * This is done to avoid making ActiveMonitorItem inherit QObject.
   */
  class ActiveMonitor : public QWidget
  {
    Q_OBJECT

    public:

      /**
       * @param server WebServer which we should connect to in order to
       * receive signals.
       */
      ActiveMonitor
        (
          WebServer   * server,
          QWidget     * parent  = 0,
          const char  * name    = 0
        );

      virtual ~ActiveMonitor();

      /**
       * @return WebServer object we were given at construction.
       */
      WebServer * server();

    public slots:

      /**
       * Look for selected ActiveMonitorItem objects and kill their
       * connections immediately.
       */
      void slotKillSelected();

    protected slots:

      /**
       * Called when a Server object has been created to handle an incoming
       * connection.
       * Creates an ActiveMonitorItem and associates it with the server.
       * @param server The Server which was created.
       */
      void slotConnection(Server * server);

      /**
       * Called when a Server object has sent data to the remote client.
       * Updates the associated ActiveMonitorItem.
       * @param server The Server which is handling the connection.
       * @param bytes Number of bytes sent by the server object.
       */
      void slotOutput(Server * server, ulong bytes);

      /**
       * Called when a Server object has finished all transactions with its
       * remote client and is about to die. Marks the associated
       * ActiveMonitorItem as defunct, for later culling via slotCull.
       * @param server The Server which is handling the connection.
       */
      void slotFinished(Server *);

      /**
       * Called when a Server object has received a response from its remote
       * client. Updates the associated ActiveMonitorItem.
       * @param server The Server which is handling the connection.
       */
      void slotRequest(Server *);

      /**
       * Called when a Server object has sent a response to its remote
       * client. Updates the associated ActiveMonitorItem.
       * @param server The Server which is handling the connection.
       */
      void slotResponse(Server *);

      /**
       * Called periodically to remove ActiveMonitorItem objects which are no
       * longer associated with a Server object.
       */
      void slotCull();

      /**
       * Connected to the relevant signal of the contained QListView and used
       * to update the enabled/disabled state of the button which allows
       * killing connections.
       */
      void slotSelectionChanged();

    protected:

      /**
       * Overridden to emit a signal when this window is closed.
       */
      virtual void closeEvent(QCloseEvent *);

    signals:

      /**
       * Emitted when this window is closed.
       */
      void dying(ActiveMonitor *);

      /**
       * Emitted when the selection of the  contained QListView has changed. 
       * @param selectionExists true if there is a selection.
       */
      void selection(bool selectionExists);

    private:

      QListView * view_;
      WebServer * server_;

      QMap<Server *, ActiveMonitorItem *> itemMap_;

      QTimer cullTimer_;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
