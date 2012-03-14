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

#ifndef KPF_APPLET_ITEM_H
#define KPF_APPLET_ITEM_H

#include <qptrlist.h>
#include <qwidget.h>

class KPopupMenu;

namespace KPF
{
  class ConfigDialog;
  class BandwidthGraph;
  class ActiveMonitorWindow;
  class SingleServerConfigDialog;
  class WebServer;

  /**
   * Provides control of, and coarse-grained activity display for, a WebServer
   * object. Contains a BandwidthGraph widget and provides a context menu
   * which allows WebServer object control, plus creation of a new WebServer,
   * for user convenience.
   */
  class AppletItem : public QWidget
  {
    Q_OBJECT

    public:

      /**
       * @param server The WebServer object which will be monitored and
       * controlled by this object.
       */
      AppletItem(WebServer * server, QWidget * parent);

      ~AppletItem();

      /**
       * @return the WebServer object given on construction.
       */
      WebServer * server();
      
      void setBackground();

    protected slots:

      /**
       * Called when an ActiveMonitorWindow (created by this object) is
       * about to close.
       */
      void slotActiveMonitorWindowDying(ActiveMonitorWindow *);

      /**
       * Called when a SingleServerConfigDialog (created by this object) is
       * about to close.
       */
      void slotConfigDialogDying(SingleServerConfigDialog *);

      /**
       * Called when the user requests a new WebServer via the context menu.
       */
      void slotNewServer();

      /**
       * Called by a timer after removeServer has been called and the event
       * loop has been processed once.
       */
      void slotSuicide();

    signals:

      /**
       * Emitted when a new WebServer is requested from the context menu.
       */
      void newServer();

      /**
       * Emitted when an URL pointing to a local directory has been dropped.
       */
      void newServerAtLocation(const QString &);

    protected:

      /**
       * Overridden to provide a context menu plus DnD capabilities.
       */
      bool eventFilter(QObject *, QEvent *);

      /**
       * Called when the appropriate item is selected from the context menu.
       * Creates an ActiveMonitorWindow.
       */
      void monitorServer    ();

      /**
       * Called when the appropriate item is selected from the context menu.
       * Asks the WebServerManager instance to remove the associated
       * WebServer.
       */
      void removeServer     ();

      /**
       * Called when the appropriate item is selected from the context menu.
       * Creates a configuration dialog for the associated WebServer.
       */
      void configureServer  ();

      /**
       * Called when the appropriate item is selected from the context menu.
       * Restarts the associated WebServer.
       */
      void restartServer    ();

      /**
       * Called when the appropriate item is selected from the context menu.
       * Pauses the associated WebServer.
       */
      void pauseServer      ();

    private:

      enum
      {
        Title,
        NewServer,
        Separator,
        Monitor,
        Configure,
        Remove,
        Restart,
        Pause
      };


      WebServer                 * server_;
      SingleServerConfigDialog  * configDialog_;
      ActiveMonitorWindow       * monitorWindow_;
      BandwidthGraph            * graph_;
      KPopupMenu                * popup_;
  };
}

#endif // KPF_APPLET_H

// vim:ts=2:sw=2:tw=78:et
