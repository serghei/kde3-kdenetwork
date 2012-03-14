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

#ifndef KPF_BANDWIDTH_GRAPH_H
#define KPF_BANDWIDTH_GRAPH_H

#include <qwidget.h>
#include <qmemarray.h>
#include <qpixmap.h>
#include <qrect.h>

class QPainter;

namespace KPF
{
  class WebServer;

  /**
   * Draws a graph of the bandwidth usage of a WebServer over time.
   * May also displays an overlayed icon to show the status of a WebServer,
   * i.e. whether it is active (no icon,) paused or in contention for a port.
   */
  class BandwidthGraph : public QWidget
  {
    Q_OBJECT

    public:

      enum OverlaySelect
      {
        UseOverlays,
        NoOverlays
      };

      /**
       * @param server WebServer to monitor.
       * @param overlaySelect if UseOverlays, draw overlay icons to reflect
       * server status.
       */
      BandwidthGraph
        (
         WebServer    * server,
         OverlaySelect  overlaySelect,
         QWidget      * parent = 0,
         const char   * name = 0
        );

      virtual ~BandwidthGraph();

      /**
       * Set the tooltip showing shared directory name and port
       */
      void setTooltip();

      /**
       * Overridden to provide reasonable default size and shape.
       */
      virtual QSize sizeHint() const;

      /**
       * Overridden to provide reasonable minimum size and shape.
       */
      virtual QSize minimumSizeHint() const;

      /**
       * @return the WebServer object given on construction.
       */
      WebServer * server();

    protected slots:

      /**
       * Connected to associated WebServer to receive notification of output.
       */
      void slotOutput(ulong);

      /**
       * Connected to associated WebServer to receive notification of port
       * contention.
       */
      void slotServerContentionChange(bool);

      /**
       * Connected to associated WebServer to receive notification of pause
       * or unpause.
       */
      void slotServerPauseChange(bool);

    protected:

      /**
       * Overridden to provide graph drawing.
       */
      virtual void paintEvent(QPaintEvent *);

      /**
       * Overridden to assist graph drawing.
       */
      virtual void resizeEvent(QResizeEvent *);

      /**
       * Draw overlay icons to reflect status of associated WebServer.
       */
      virtual void drawOverlays(QPainter &);

      /**
       * Provides a rectangle in which to draw the graph itself.
       */
      virtual QRect contentsRect() const;

      /**
       * Called when the status of the associated WebServer changes.
       */
      virtual void updateOverlayPixmap();

    signals:

      /**
       * Emitted when the maximum displayed value has changed.
       */
      void maximumChanged(ulong);

    private:

      void updateContents();

      QMemArray<ulong> history_;

      WebServer * server_;

      QPixmap buffer_, bgPix_;

      ulong max_;

      OverlaySelect overlaySelect_;

      QPixmap overlayPixmap_;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
