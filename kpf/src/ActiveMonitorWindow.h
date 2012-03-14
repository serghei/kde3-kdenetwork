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

#ifndef KPF_ACTIVE_MONITOR_WINDOW_H
#define KPF_ACTIVE_MONITOR_WINDOW_H

#include <kmainwindow.h>

class KAction;

namespace KPF
{
  class ActiveMonitor;
  class WebServer;

  /**
   * Wraps an ActiveMonitor (widget) in a toplevel window.
   *
   * A wrapper window is used to avoid forcing ActiveMonitor to be
   * toplevel, so it can be used elsewhere if desired.
   */
  class ActiveMonitorWindow : public KMainWindow
  {
    Q_OBJECT

    public:

      /**
       * @param server WebServer which we should connect to in order to
       * receive signals.
       */
      ActiveMonitorWindow
        (
          WebServer   * server,
          QWidget     * parent  = 0,
          const char  * name    = 0
        );

      virtual ~ActiveMonitorWindow();

      /**
       * @return WebServer object we were given at construction.
       */
      WebServer * server();

    protected slots:

      /**
       * Connected to ActiveMonitor::selection, which tells us whether we
       * should enable the 'kill connection' action.
       */
      void slotMayKill(bool);

    protected:

      /**
       * Overridden to emit a signal when this window is closed.
       */
      virtual void closeEvent(QCloseEvent *);

    signals:

      /**
       * Emitted when this window is closed.
       */
      void dying(ActiveMonitorWindow *);

      /**
       * Emitted when the selection of the  contained QListView has changed. 
       * @param selectionExists true if there is a selection.
       */
      void selection(bool selectionExists);

    private:

      ActiveMonitor * monitor_;

      KAction * killAction_;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
