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

#include <qpainter.h>
#include <kiconloader.h>

#include "Defines.h"
#include "ActiveMonitorItem.h"
#include "Server.h"
#include "Utils.h"

namespace KPF
{
  ActiveMonitorItem::ActiveMonitorItem(Server * server, QListView * parent)
    : QListViewItem (parent),
      server_       (server),
      size_         (0),
      sent_         (0)
  {
    setText(Host,       server_->peerAddress().toString());
    setText(Resource,   "...");
    setText(Response,   "...");
    setText(Size,       "...");
    setText(Sent,       "...");

    updateState();
  }

  ActiveMonitorItem::~ActiveMonitorItem()
  {
    // Empty.
  }

    void
  ActiveMonitorItem::paintCell
  (
   QPainter * p,
   const QColorGroup & g,
   int c,
   int w,
   int a
  )
  {
    if (c != Progress)
    {
      QListViewItem::paintCell(p, g, c, w, a);
      return;
    }

    p->setPen(g.dark());

    p->setPen(g.base());

    p->drawRect(0, 0, w, height());

    int maxBarLength = w - 4;
    
    int barLength = maxBarLength;
   
    if (0 != size_)
      barLength = int((sent_ / double(size_)) * maxBarLength);

    p->fillRect(2, 2, barLength, height() - 4, g.highlight());
  }

    int
  ActiveMonitorItem::width
  (
   const QFontMetrics & fm,
   const QListView * lv,
   int c
  ) const
  {
    switch (c)
    {
      case Status:
        return 16;
        break;

      case Progress:
        return 32;
        break;

      default:
        return QListViewItem::width(fm, lv, c);
        break;
    }
  }

    void
  ActiveMonitorItem::updateState()
  {
    if (0 != server_)
    {
      switch (server_->state())
      {
        case Server::WaitingForRequest:
          setPixmap(Status, SmallIcon("connect_creating"));
          break;

        case Server::WaitingForHeaders:
          setPixmap(Status, SmallIcon("connect_creating"));
          break;

        case Server::Responding:
          setPixmap(Status, SmallIcon("connect_established"));
          break;

        case Server::Finished:
          setPixmap(Status, SmallIcon("connect_no"));
          break;
      }
    }
  }

    Server *
  ActiveMonitorItem::server()
  {
    return server_;
  }

    QDateTime
  ActiveMonitorItem::death() const
  {
    return death_;
  }

    void
  ActiveMonitorItem::request()
  {
    if (0 != server_)
    {
      setText(Resource, server_->request().path());
      updateState();
    }
  }

    void
  ActiveMonitorItem::response()
  {
    if (0 != server_)
    {
      setText(Response, translatedResponseName(server_->response().code()));

      size_ = server_->response().size();

      setText(Size, QString::number(size_));

      updateState();
    }
  }

    void
  ActiveMonitorItem::output(ulong l)
  {
    if (0 != server_)
    {
      sent_ += l;
      setText(Sent, QString::number(sent_));
      updateState();
      repaint();
    }
  }

    void
  ActiveMonitorItem::finished()
  {
    if (0 != server_)
    {
      death_ = server_->death();
      updateState();
    }

    server_ = 0L;
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
