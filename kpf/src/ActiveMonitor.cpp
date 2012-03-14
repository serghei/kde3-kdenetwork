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

#include <qlayout.h>

#include <kiconloader.h>
#include <klocale.h>

#include "Defines.h"
#include "ActiveMonitorItem.h"
#include "ActiveMonitor.h"
#include "WebServer.h"
#include "Server.h"
#include "Utils.h"

namespace KPF
{
  ActiveMonitor::ActiveMonitor
    (
     WebServer * server,
     QWidget * parent,
     const char * name
    )
    : QWidget (parent, name),
      server_ (server)
  {
    view_ = new QListView(this);

    view_->setAllColumnsShowFocus(true);
    view_->setSelectionMode(QListView::Extended);

    view_->addColumn(i18n("Status"));
    view_->addColumn(i18n("Progress"));
    view_->addColumn(i18n("File Size"));
    view_->addColumn(i18n("Bytes Sent"));
    view_->addColumn(i18n("Response"));
    view_->addColumn(i18n("Resource"));
    view_->addColumn(i18n("Host"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    layout->addWidget(view_);

    connect
      (
       view_,
       SIGNAL(selectionChanged()),
       SLOT(slotSelectionChanged())
      );

    connect
      (
       server_,
       SIGNAL(connection(Server *)),
       SLOT(slotConnection(Server *))
      );

    connect
      (
       server_,
       SIGNAL(output(Server *, ulong)),
       SLOT(slotOutput(Server *, ulong))
      );

    connect(server_, SIGNAL(finished(Server *)),  SLOT(slotFinished(Server *)));
    connect(server_, SIGNAL(request(Server *)),   SLOT(slotRequest(Server *)));
    connect(server_, SIGNAL(response(Server *)),  SLOT(slotResponse(Server *)));

    connect(&cullTimer_, SIGNAL(timeout()), SLOT(slotCull()));

    cullTimer_.start(1000);

    // Tell whoever cares about our selection status.
    slotSelectionChanged();
  }

  ActiveMonitor::~ActiveMonitor()
  {
  }

    void
  ActiveMonitor::slotConnection(Server * s)
  {
    ActiveMonitorItem * i = new ActiveMonitorItem(s, view_);
    itemMap_[s] = i;
  }

    void
  ActiveMonitor::slotOutput(Server * s, ulong l)
  {
    ActiveMonitorItem * i = itemMap_[s];

    if (0 != i)
      i->output(l);
  }

    void
  ActiveMonitor::slotFinished(Server * s)
  {
    ActiveMonitorItem * i = itemMap_[s];

    if (0 != i)
      i->finished();

    itemMap_.remove(s);
  }

    void
  ActiveMonitor::slotRequest(Server * s)
  {
    ActiveMonitorItem * i = itemMap_[s];

    if (0 != i)
      i->request();
  }

    void
  ActiveMonitor::slotResponse(Server * s)
  {
    ActiveMonitorItem * i = itemMap_[s];

    if (0 != i)
      i->response();
  }

    void
  ActiveMonitor::slotCull()
  {
    QDateTime dt = QDateTime::currentDateTime();

    QListViewItemIterator it(view_);

    for (; it.current(); ++it)
    {
      ActiveMonitorItem * i = static_cast<ActiveMonitorItem *>(it.current());

      if ((0 == i->server()) && (i->death().secsTo(dt) > 60))
      {
        delete i;
        ++it;
      }
    }
  }

    void
  ActiveMonitor::slotSelectionChanged()
  {
    for (QListViewItemIterator it(view_); it.current(); ++it)
    {
      ActiveMonitorItem * i = static_cast<ActiveMonitorItem *>(it.current());

      if
        (
         view_->isSelected(i)
         &&
         (0 != i->server())
         &&
         (Server::Finished != i->server()->state())
        )
      {
        emit(selection(true));
        return;
      }
    }

    emit(selection(false));
  }

    void
  ActiveMonitor::slotKillSelected()
  {
    for (QListViewItemIterator it(view_); it.current(); ++it)
    {
      ActiveMonitorItem * i = static_cast<ActiveMonitorItem *>(it.current());

      if
        (
         view_->isSelected(i)
         &&
         (0 != i->server())
         &&
         (Server::Finished != i->server()->state())
        )
      {
        i->server()->cancel();
      }
    }
  }

    WebServer *
  ActiveMonitor::server()
  {
    return server_;
  }

    void
  ActiveMonitor::closeEvent(QCloseEvent * e)
  {
    QWidget::closeEvent(e);
    emit(dying(this));
  }

} // End namespace KPF

#include "ActiveMonitor.moc"
// vim:ts=2:sw=2:tw=78:et
