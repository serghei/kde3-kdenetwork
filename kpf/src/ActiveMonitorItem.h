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

#ifndef KPF_ACTIVE_MONITOR_ITEM_H
#define KPF_ACTIVE_MONITOR_ITEM_H

#include <qlistview.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qpalette.h>

class QPainter;

namespace KPF
{
  class Server;

  /**
   * Used to display the status of a Server object.
   * Created and managed by an ActiveMonitor object.
   * Provides some textual information, including the requested filename
   * and the response code, plus a simple graph displaying the data transfer
   * progress of any response.
   */
  class ActiveMonitorItem : public QListViewItem
  {
    public:

      enum Column
      {
        Status,
        Progress,
        Size,
        Sent,
        Response,
        Resource,
        Host
      };

      /**
       * @param server the associated Server object.
       */
      ActiveMonitorItem(Server * server, QListView * parent);
      virtual ~ActiveMonitorItem();

      /**
       * @return the Server object passed on construction.
       */
      Server * server();

      /**
       * Called by the controlling ActiveMonitor object when the associated
       * Server object has received a request from its remote client. Queries
       * the Server object for the Request object.
       *
       * May be called more than once, if a Server object handles more than
       * one request (i.e. is working in `persistent' mode.)
       */
      void request();

      /**
       * Called by the controlling ActiveMonitor object when the associated
       * Server object has sent a response to its remote client. Queries the
       * Server object for the Response object.
       *
       * May be called more than once, if a Server object handles more than
       * one request (i.e. is working in `persistent' mode.)
       */
      void response();

      /**
       * Called by the controlling ActiveMonitor object when the associated
       * Server object has sent data to its remote client.
       *
       * This is called every time output is sent, with the total output
       * since the request began.
       */
      void output(ulong);

      /**
       * Called by the controlling ActiveMonitor object when the associated
       * Server object has completed all transactions with its remote client.
       */
      void finished();

      /**
       * @return the time of death (end of transactions with remote client)
       * of the associated Server object.
       */
      QDateTime death() const;

    protected:

      /**
       * Updates the display to reflect the current state of the connection
       * held by the associated Server object.
       */
      virtual void updateState();

      /**
       * Overridden to provide for drawing a graph in the cell which displays
       * the number of bytes sent to the remote client by the associated
       * Server object.
       */
      virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

      /**
       * Overridden to provide for giving reasonable sizes for columns which
       * do not contain text.
       */
      virtual int width(const QFontMetrics &, const QListView *, int) const;

    private:

      Server * server_;
      QDateTime death_;
      ulong size_;
      ulong sent_;
  };

} // End namespace KPF

#endif // KPF_ACTIVE_MONITOR_ITEM_H
// vim:ts=2:sw=2:tw=78:et
