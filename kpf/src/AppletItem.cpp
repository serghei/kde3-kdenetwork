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

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qcursor.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kapplication.h>

#include "Defines.h"
#include "AppletItem.h"
#include "WebServerManager.h"
#include "WebServer.h"
#include "BandwidthGraph.h"
#include "ActiveMonitorWindow.h"
#include "SingleServerConfigDialog.h"

namespace KPF
{
  AppletItem::AppletItem(WebServer * server, QWidget * parent)
    : QWidget       (parent, "KPF::AppletItem"),
      server_       (server),
      configDialog_ (0L),
      monitorWindow_      (0L),
      graph_        (0L),
      popup_        (0L)
  {
    setBackgroundOrigin(AncestorOrigin);
    setAcceptDrops(true);

    graph_ = new BandwidthGraph(server_, BandwidthGraph::UseOverlays, this);

    graph_->setAcceptDrops(true);

    graph_->installEventFilter(this);

    (new QVBoxLayout(this))->addWidget(graph_);

    QString popupTitle(i18n("kpf - %1").arg(server_->root()));

    popup_ = new KPopupMenu(this);

    popup_->insertTitle
      (SmallIcon("kpf"),          popupTitle,             Title,     Title);

    popup_->insertItem
      (SmallIcon("filenew"),      i18n("New Server..."),  NewServer, NewServer);

    popup_->insertSeparator(Separator);

    popup_->insertItem
      (SmallIcon("viewmag"),      i18n("Monitor"),     Monitor,   Monitor);

    popup_->insertItem
      (SmallIcon("configure"),    i18n("Preferences..."),   Configure, Configure);

    popup_->insertItem
      (SmallIcon("remove"),       i18n("Remove"),         Remove,    Remove);

    popup_->insertItem
      (SmallIcon("reload"),       i18n("Restart"),        Restart,   Restart);

    popup_->insertItem
      (SmallIcon("player_pause"), i18n("Pause"),          Pause,     Pause);

    monitorWindow_ = new ActiveMonitorWindow(server_);

    connect
      (
       monitorWindow_,
       SIGNAL(dying(ActiveMonitorWindow *)),
       SLOT(slotActiveMonitorWindowDying(ActiveMonitorWindow *))
      );
  }

  AppletItem::~AppletItem()
  {
    delete configDialog_;
    configDialog_ = 0;

    delete monitorWindow_;
    monitorWindow_ = 0;
  }
  
  void AppletItem::setBackground()
  {
    QResizeEvent e(size(), size());
    kapp->sendEvent(graph_, &e);
    graph_->update();
  }

    bool
  AppletItem::eventFilter(QObject *, QEvent * ev)
  {
    switch (ev->type())
    {

      case QEvent::MouseButtonRelease:
        {
          QMouseEvent * e = static_cast<QMouseEvent *>(ev);

          if (0 == e)
          {
            kpfDebug
              << "Hmm, should have been able to static_cast here." << endl;
            break;
          }

          if (!rect().contains(e->pos()))
          {
            break;
          }
 
          if (Qt::LeftButton == e->button())
          {
            // Show monitor.

            if (0 == monitorWindow_)
              monitorServer();

            else
            {
              if (monitorWindow_->isVisible())
                monitorWindow_->hide();
              else
                monitorWindow_->show();
            }
          }

          return true;
        }
        break;

      case QEvent::MouseButtonPress:
        {
          QMouseEvent * e = static_cast<QMouseEvent *>(ev);

          if (0 == e)
          {
            kpfDebug
              << "Hmm, should have been able to static_cast here." << endl;
            break;
          }

          if (Qt::RightButton != e->button() && Qt::LeftButton != e->button())
            break;

          if (server_->paused())
            popup_->changeItem
              (Pause, SmallIcon("1rightarrow"), i18n("Unpause"));
          else
            popup_->changeItem
              (Pause, SmallIcon("player_pause"), i18n("Pause"));

          switch (popup_->exec(QCursor::pos()))
          {
            case NewServer:
              emit(newServer());
              break;

            case Monitor:
              monitorServer();
              break;

            case Configure:
              configureServer();
              break;

            case Remove:
              removeServer();
              break;

            case Restart:
              restartServer();
              break;

            case Pause:
              pauseServer();
              break;

            default:
              break;
          }

          return true;
        }
        break;

      case QEvent::DragEnter:
        {
          QDragEnterEvent * e = static_cast<QDragEnterEvent *>(ev);

          if (0 == e)
          {
            kpfDebug
              << "Hmm, should have been able to static_cast here." << endl;
            break;
          }

          KURL::List l;

          if (!KURLDrag::decode(e, l))
            break;

          if (l.count() != 1)
            break;
          
          const KURL &url = l[0];

          if (!url.isLocalFile() || !QFileInfo(url.path()).isDir())
            break;

          e->accept();
          return true;
        }
        break;

      case QEvent::Drop:
        {
          QDropEvent * e = static_cast<QDropEvent *>(ev);

          if (0 == e)
          {
            kpfDebug
              << "Hmm, should have been able to static_cast here." << endl;
            break;
          }

          KURL::List l;

          if (!KURLDrag::decode(e, l))
            break;

          if (l.count() != 1)
            break;
          
          const KURL &url = l[0];

          if (!url.isLocalFile() || !QFileInfo(url.path()).isDir())
            break;

          e->accept();
          emit(newServerAtLocation(url.path()));
          return true;
        }
        break;

      default:
        break;
    }

    return false;
  }

    void
  AppletItem::slotActiveMonitorWindowDying(ActiveMonitorWindow *)
  {
    // We used to delete it here, but let's not. See if this is a CPU hog.
#if 0
    delete monitorWindow_;
    monitorWindow_ = 0;
#endif
    monitorWindow_->hide();
  }

    void
  AppletItem::slotConfigDialogDying(SingleServerConfigDialog *)
  {
    graph_->setTooltip();

    configDialog_->delayedDestruct();
    configDialog_ = 0;
  }

    void
  AppletItem::slotNewServer()
  {
    emit(newServer());
  }

    void
  AppletItem::monitorServer()
  {
    // We used to delete it here, but let's not. See if this is a CPU hog.
#if 0
    if (0 != monitorWindow_)
    {
      monitorWindow_->show();
      return;
    }

    monitorWindow_ = new ActiveMonitorWindow(server_);

    connect
      (
       monitorWindow_,
       SIGNAL(dying(ActiveMonitorWindow *)),
       SLOT(slotActiveMonitorWindowDying(ActiveMonitorWindow *))
      );
#endif

    monitorWindow_->show();
    monitorWindow_->raise();
  }

    void
  AppletItem::removeServer()
  {
    QTimer::singleShot(0, this, SLOT(slotSuicide()));
  }

    void
  AppletItem::configureServer()
  {
    if (0 != configDialog_)
    {
      configDialog_->show();
      return;
    }

    configDialog_ = new SingleServerConfigDialog(server_, 0);

    connect
      (
       configDialog_,
       SIGNAL(dying(SingleServerConfigDialog *)),
       SLOT(slotConfigDialogDying(SingleServerConfigDialog *))
      );

    configDialog_->show();
  }

    void
  AppletItem::slotSuicide()
  {
    WebServerManager::instance()->disableServer(server_->root());
  }

    void
  AppletItem::restartServer()
  {
    server_->restart();
  }

    void
  AppletItem::pauseServer()
  {
    server_->pause(!server_->paused());
  }

    WebServer *
  AppletItem::server()
  {
    return server_;
  }
}

#include "AppletItem.moc"
// vim:ts=2:sw=2:tw=78:et
