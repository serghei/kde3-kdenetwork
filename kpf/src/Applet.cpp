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
#include <qtimer.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qcursor.h>

#include <dcopclient.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kurldrag.h>

#include "System.h"
#include "Defines.h"
#include "Applet.h"
#include "AppletItem.h"
#include "WebServerManager.h"
#include "WebServer.h"
#include "ServerWizard.h"

static const char kpfVersion[] = "1.0.1";

extern "C"
{
  KDE_EXPORT  KPanelApplet *
  init(QWidget * parent, const QString & configFile)
  {
    if (0 == kpf::userId() || 0 == kpf::effectiveUserId())
    {
      // Don't run as root.
      KMessageBox::detailedError
        ( 0,
          i18n("You cannot run KPF as root."),
          i18n("Running as root exposes the whole system to "
               "external attackers."),
          i18n("Running as root.")
        );
      return NULL;
    }
    else
    {
      kpf::blockSigPipe();

      KGlobal::locale()->insertCatalogue("kpf");

      return new KPF::Applet
        (
         configFile,
         KPanelApplet::Normal,
         KPanelApplet::About|KPanelApplet::Help,
         parent,
         "kpf"
        );
    }
  }
}

namespace KPF
{
  Applet::Applet
  (
   const QString  & configFile,
   Type             type,
   int              actions,
   QWidget        * parent,
   const char     * name
  )
    : KPanelApplet  (configFile, type, actions, parent, name),
      wizard_       (0L),
      popup_        (0L),
      dcopClient_   (0L)
  {
    setAcceptDrops(true);

    //setFrameStyle(QFrame::Panel | QFrame::Sunken);
    //setLineWidth(1);

    connect
      (
       WebServerManager::instance(),
       SIGNAL(serverCreated(WebServer *)),
       SLOT(slotServerCreated(WebServer *))
      );

    connect
      (
       WebServerManager::instance(),
       SIGNAL(serverDisabled(WebServer *)),
       SLOT(slotServerDisabled(WebServer *))
      );

    WebServerManager::instance()->loadConfig();

    popup_ = new QPopupMenu(this);

    popup_->insertItem
      (BarIcon("filenew"), i18n("New Server..."), NewServer, NewServer);

//    popup_->insertItem
//      (BarIcon("quit"), i18n("Quit"), Quit, Quit);

    dcopClient_ = new DCOPClient;
    dcopClient_->registerAs("kpf", false);
  }

  Applet::~Applet()
  {
    delete dcopClient_;
    WebServerManager::instance()->shutdown();
  }

    int
  Applet::widthForHeight(int h) const
  {
    uint serverCount = itemList_.count();

    if (0 == serverCount)
      serverCount = 1;

    if (Vertical == orientation())
      return h / serverCount;
    else
      return h * serverCount;
  }

    int
  Applet::heightForWidth(int w) const
  {
    uint serverCount = itemList_.count();

    if (0 == serverCount)
      serverCount = 1;

    if (Vertical == orientation())
      return w * serverCount;
    else
      return w / serverCount;
  }

    void
  Applet::help()
  {
    kapp->invokeHelp( QString::null, "kpf" );
  }

    void
  Applet::about()
  {
    KAboutData about
      (
       "kpf",
       I18N_NOOP("kpf"),
       kpfVersion,
       I18N_NOOP("KDE public fileserver"),
       KAboutData::License_Custom,
       "(C) 2001 Rik Hemsley (rikkus) <rik@kde.org>",
       I18N_NOOP(
         "File sharing applet, using the HTTP (Hyper Text Transfer Protocol)"
         " standard to serve files."
       ),
       "http://rikkus.info/kpf.html"
     );

    about.setLicenseText
      (
        I18N_NOOP
        (
"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
"of this software and associated documentation files (the \"Software\"), to\n"
"deal in the Software without restriction, including without limitation the\n"
"rights to use, copy, modify, merge, publish, distribute, sublicense, and/or\n"
"sell copies of the Software, and to permit persons to whom the Software is\n"
"furnished to do so, subject to the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be included in\n"
"all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN\n"
"ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n"
"WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
        )
      );

    KAboutApplication a(&about, this);
    a.exec();
  }

    void
  Applet::orientationChange(Orientation)
  {
    resetLayout();
  }

    void
  Applet::resizeEvent(QResizeEvent *)
  {
    resetLayout();
  }
    
    void
  Applet::moveEvent(QMoveEvent *)
  {
    QPtrListIterator<AppletItem> it(itemList_);

    for (uint i = 0; it.current(); ++it, ++i)
        it.current()->setBackground();
  }

    void
  Applet::resetLayout()
  {
    if (0 == itemList_.count())
      return;

    switch (orientation())
    {
      case Vertical:
        {
          uint itemHeight = height() / itemList_.count();

          QPtrListIterator<AppletItem> it(itemList_);

          for (uint i = 0; it.current(); ++it, ++i)
          {
            it.current()->resize(width(), itemHeight);
            it.current()->move(0, i * itemHeight);
          }
        }
        break;

      case Horizontal:
        {
          uint itemWidth = width() / itemList_.count();

          QPtrListIterator<AppletItem> it(itemList_);

          for (uint i = 0; it.current(); ++it, ++i)
          {
            it.current()->resize(itemWidth, height());
            it.current()->move(i * itemWidth, 0);
          }
        }
      default:
        break;
    }
  }

    void
  Applet::mousePressEvent(QMouseEvent * ev)
  {
    if (Qt::RightButton != ev->button() && Qt::LeftButton != ev->button())
      return;

    switch (popup_->exec(QCursor::pos()))
    {
      case NewServer:
        slotNewServer();
        break;

      case Quit:
        slotQuit();
        break;

      default:
        break;
    }
  }

    void
  Applet::slotNewServerAtLocation(const QString & location)
  {
    if (0 != wizard_)
    {
      wizard_->setLocation(location);
      wizard_->show();
    }

    else
    {
      wizard_ = new ServerWizard;

      connect
        (
         wizard_,
         SIGNAL(dying(ServerWizard *)),
         SLOT(slotWizardDying(ServerWizard *))
        );

      wizard_->setLocation(location);
      wizard_->show();
    }
  }

    void
  Applet::slotNewServer()
  {
    if (0 != wizard_)
      wizard_->show();

    else
    {
      wizard_ = new ServerWizard;

      connect
        (
         wizard_,
         SIGNAL(dying(ServerWizard *)),
         SLOT(slotWizardDying(ServerWizard *))
        );

      wizard_->show();
    }
  }

    void
  Applet::slotWizardDying(ServerWizard * wiz)
  {
    if (QDialog::Accepted == wiz->result())
    {
      WebServerManager::instance()->createServerLocal
        (
         wiz->root(),
         wiz->listenPort(),
         wiz->bandwidthLimit(),
         wiz->connectionLimit(),
	 Config::DefaultFollowSymlinks,
	 wiz->serverName()
        );
    }

    delete wizard_;
    wizard_ = 0;
  }

    void
  Applet::drawContents(QPainter * p)
  {
    QPixmap px;

    if (width() > 48)
      px = KGlobal::iconLoader()->loadIcon("kpf", KIcon::Panel, 48);
    else if (width() > 32)
      px = KGlobal::iconLoader()->loadIcon("kpf", KIcon::Panel, 32);
    else if (width() > 16)
      px = KGlobal::iconLoader()->loadIcon("kpf", KIcon::Panel, 16);
    else
      return;

    QRect r(contentsRect());

    p->drawPixmap
      (
       r.x() + r.width()  / 2 - px.width()  / 2,
       r.y() + r.height() / 2 - px.height() / 2,
       px
      );
  }

    void
  Applet::dragEnterEvent(QDragEnterEvent * e)
  {
    KURL::List l;

    if (!KURLDrag::decode(e, l))
      return;

    if (l.count() != 1)
      return;

    const KURL &url = l[0];

    if (!url.isLocalFile() || !QFileInfo(url.path()).isDir())
      return;

    e->accept();
  }

    void
  Applet::dropEvent(QDropEvent * e)
  {
    KURL::List l;

    if (!KURLDrag::decode(e, l))
      return;

    if (l.count() != 1)
      return;

    const KURL &url = l[0];

    if (!url.isLocalFile() || !QFileInfo(url.path()).isDir())
      return;

    e->accept();

    slotNewServerAtLocation(url.path());
  }

    void
  Applet::slotServerCreated(WebServer * server)
  {
    AppletItem * i = new AppletItem(server, this);

    connect
      (
       i,
       SIGNAL(newServer()),
       SLOT(slotNewServer())
      );

    connect
      (
       i,
       SIGNAL(newServerAtLocation(const QString &)),
       SLOT(slotNewServerAtLocation(const QString &))
      );

    itemList_.append(i);
    i->show();
    emit(updateLayout());
    resetLayout();
  }

    void
  Applet::slotServerDisabled(WebServer * server)
  {
    QPtrListIterator<AppletItem> it(itemList_);

    for (; it.current(); ++it)
    {
      AppletItem * i = it.current();

      if (i->server() == server)
      {
        itemList_.removeRef(i);
        delete i;
        emit(updateLayout());
        resetLayout();
        return;
      }
    }
  }

    void
  Applet::slotQuit()
  {
    // How ?
  }

} // End namespace KPF

#include "Applet.moc"

// vim:ts=2:sw=2:tw=78:et
