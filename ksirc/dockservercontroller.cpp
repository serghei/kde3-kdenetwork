/**********************************************************************

 $$Id: dockservercontroller.cpp 278038 2004-01-09 00:48:49Z asj $$

 Dock Server Controller.  Displays server connection window, and makes
 new server connection on demand.

 *********************************************************************/

#include "dockservercontroller.h"

#include <qpopupmenu.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qcursor.h>
#include <kpassivepopup.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kaction.h>
#include <kwin.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <kmessagebox.h>
#include <kglobalaccel.h>
#include <kkeydialog.h>
#include <kdeversion.h>

#include "KSPrefs/ksprefs.h"

#include "servercontroller.h"
#include "toplevel.h"
#include "ksopts.h"

#include "objFinder.h"
#include "displayMgr.h"
#include "objFinder.h"

#include "dccNew.h"

extern DisplayMgr *displayMgr;

dscNickInfo::dscNickInfo(QString nick, QString server) :
    m_nick(nick),
    m_server(server)
{
}

dscNickInfo::~dscNickInfo()
{
}


dockServerController::dockServerController(servercontroller *sc, QWidget *parent, const char *_name)
: KSystemTray(parent, _name),
  m_sc(sc)
{

  m_nicks.setAutoDelete(true);
  KPopupMenu *pop = contextMenu();
  pop->setName("dockServerController_menu_pop");

#ifndef NDEBUG
  pop->insertItem(i18n("Dump Object Tree"), sc, SLOT(dump_obj()));
  pop->insertItem(i18n("Server Debug Window"), sc, SLOT(server_debug()));
  pop->insertSeparator();
#endif


  pop->insertItem(SmallIcon( "filter" ), i18n("&Filter Rule Editor..."),
                  m_sc, SLOT(filter_rule_editor()));

  KStdAction::preferences(m_sc, SLOT(general_prefs()),
                  m_sc->actionCollection())->plug(pop);
  KStdAction::configureNotifications(m_sc, SLOT(notification_prefs()), actionCollection())->plug(pop);

  pop->insertSeparator();
  pop->insertItem(i18n("New &Server..."),
		  m_sc, SLOT(new_connection()));
  pop->insertItem(i18n("&Do Autoconnect..."), m_sc, SLOT(start_autoconnect_check()));
  connect(this, SIGNAL(quitSelected()), m_sc, SLOT(endksirc()));

#if KDE_IS_VERSION(3,1,92)
  m_pic_dock = KSystemTray::loadIcon( "ksirc" );
#else
  m_pic_dock = UserIcon("ksirc_dock");
#endif
  m_pic_info = UserIcon("info");

  if ( !m_pic_dock.isNull() )
      setPixmap( m_pic_dock );

  mainPop = 0x0;
  m_mainPopVisible = false;
  createMainPopup();

  m_blinkTimer = new QTimer( this );
  connect( m_blinkTimer, SIGNAL(timeout()), this, SLOT( blinkDockedIcon() ) );
  m_blinkStatus = false;
  m_blinkActive = false;

  servercontroller::self()
      ->getGlobalAccel()
      ->insert("Raise Last Window", i18n("Raise Last Window"),
		i18n("If someone said your nick in a window, this action "
                     "will make that window active for you."),
	       ALT+CTRL+Key_A, KKey::QtWIN+CTRL+Key_A, this,
	       SLOT(raiseLastActiveWindow()));
  servercontroller::self()
      ->getGlobalAccel()
      ->insert("Clear Blink", i18n("Clear Blinking Dock Icon"),
		i18n("If the dock icon is blinking, but you don't want "
		     "to go to the window this will clear the blinking."),
	       ALT+CTRL+Key_Down, KKey::QtWIN+CTRL+Key_Down, this,
	       SLOT(blinkClear()));

//  mainPop = new KPopupMenu(this, "dockServerController_main_pop");
//  mainPop->setTitle(i18n("KSirc Dock Menu"));

}



dockServerController::~dockServerController()
{
kdDebug(5008) << "~dockServerController in" << endl;
    m_sc = 0x0;
kdDebug(5008) << "~dockServerController out" << endl;
}

int dockServerController::intoPopupSorted(QString str, QPopupMenu *what)
{
    uint i = 0;

    for(i = 0; i < mainPop->count(); i++){
	if(mainPop->text(mainPop->idAt(i)) > str){
	    break;
	}
    }
    return mainPop->insertItem(str, what, -1, i);

}

void dockServerController::mainPopShow()
{
    m_mainPopVisible = true;
}

void dockServerController::mainPopHide()
{
    m_mainPopVisible = false;
}

void dockServerController::createMainPopup()
{
    if(m_mainPopVisible == true)
	return;

    if(mainPop)
	delete mainPop;
    mainPop = new KPopupMenu(this, "dockservercontrller_main_pop");
    connect(mainPop, SIGNAL(activated(int)),
	    this, SLOT(mainActivated(int)));
    connect(mainPop, SIGNAL(aboutToShow()),
	    this, SLOT(mainPopShow()));
    connect(mainPop, SIGNAL(aboutToHide()),
	    this, SLOT(mainPopHide()));


    QDictIterator<dscNickInfo> it( m_nicks );
    for( ; it.current(); ++it){
	KPopupMenu *sub = new KPopupMenu(mainPop);
	if(it.current()->status() == dscNickInfo::isOnline){
	    sub->insertItem(i18n("Came Online: ") + it.current()->online().toString("hh:mm"));
	    if( ! it.current()->offline().isNull() ){
		sub->insertItem(i18n("Last Offline: ") + it.current()->offline().toString("hh:mm"));
	    }
	    int pid = sub->insertItem(i18n("Ping"));
	    int wid = sub->insertItem(i18n("Whois"));
	    int cid = sub->insertItem(i18n("Chat"));
	    int dcid = sub->insertItem(i18n("DCC Chat"));

	    int id = intoPopupSorted(it.current()->server() +" -> " + it.current()->nick() + " " + i18n("online"), sub);
	    sub->setItemParameter(pid, id);
	    sub->setItemParameter(cid, id);
	    sub->setItemParameter(wid, id);
	    sub->setItemParameter(dcid, id);
	    connect(sub, SIGNAL(activated(int)),
                    this, SLOT(subItemActivated(int)));
	}
	else {
	    sub->insertItem(i18n("Went Offline: ") + it.current()->offline().toString("hh:mm"));
	    if( ! it.current()->online().isNull() ){
		sub->insertItem(i18n("Last Online: ") + it.current()->online().toString("hh:mm"));
	    }

	    intoPopupSorted(it.current()->server() +" -> " + it.current()->nick() + i18n(" offline"), sub);
	}
    }
    if(m_blink_reason.count() > 0){
        mainPop->insertSeparator();
        for ( QStringList::Iterator it = m_blink_reason.begin(); it != m_blink_reason.end(); ++it ) {
            mainPop->insertItem(*it);
        }
    }
    if(mainPop->count() == 0){
        mainPop->insertItem(i18n("Help on Notify Popup..."), this, SLOT(helpNotice()));
	mainPop->insertItem(i18n("Configure Notify..."), this, SLOT(configNotify()));
    }
}

void dockServerController::subItemActivated(int sub_id)
{
    const QPopupMenu *sub = dynamic_cast<const QPopupMenu *>(sender());
    if(!sub){
        kdDebug(5008) << "subItemActivated by non QPopupMenu" << endl;
        return;
    }
    int main_id = sub->itemParameter(sub_id);
    QString mainText = mainPop->text(main_id);
    if(mainText.isNull()){
        kdDebug(5008) << "failed to get mainPop text" << endl;
    }
    QRegExp rx("(\\S+) -> (\\S+) ");
    if(rx.search(mainText) >= 0){
	QString mserver = rx.cap(1);
	QString mnick = rx.cap(2);

	QString ns = mserver + "/" + mnick;

	if(m_nicks[ns]){
            kdDebug(5008) << "Got " << sub->text(sub_id) << " request for " << ns << endl;
            QString cmd = sub->text(sub_id);
            QString server = m_nicks[ns]->server();
            QString nick = m_nicks[ns]->nick();

	    if(cmd == i18n("Chat")){
                servercontroller::self()->new_toplevel(KSircChannel(server, nick));
            }
	    else {
                KSircProcess *kp = servercontroller::self()->processes().find(server);
                if(kp){
                    KSircTopLevel *wm = dynamic_cast<KSircTopLevel *>(kp->getWindowList().find("!default"));
		    if(wm){
			if(cmd == i18n("Ping")){
			    wm->sirc_line_return("/ping " + nick + "\n");
			} else if(cmd == i18n("Whois")){
			    wm->sirc_line_return("/whois " + nick + "\n");
			} else if(cmd == i18n("DCC Chat")){
			    wm->sirc_line_return("/dcc chat " + nick + "\n");
			}
                    }
                }
            }
	}
    }

}

void dockServerController::mousePressEvent( QMouseEvent *e )
{
    if(e->button() == LeftButton){
	showPopupMenu(mainPop);
    }
    else {
	KSystemTray::mousePressEvent(e);
    }
}

void dockServerController::showPopupMenu( QPopupMenu *menu )
{
    Q_ASSERT( menu != 0L );

    menu->popup(QCursor::pos());

}

void dockServerController::nickOnline(QString server, QString nick)
{
    QString sn = server + "/" + nick;
    if(!m_nicks[sn])
        m_nicks.insert(sn, new dscNickInfo(nick, server));
    m_nicks[sn]->setOnline();
    createMainPopup();
}

void dockServerController::nickOffline(QString server, QString nick)
{
    QString sn = server + "/" + nick;
    if(!m_nicks[sn])
	m_nicks.insert(sn, new dscNickInfo(nick, server));
    m_nicks[sn]->setOffline();
    createMainPopup();
}

void dockServerController::serverOpen(QString)
{
}

void dockServerController::serverClose(QString server)
{
    QDictIterator<dscNickInfo> it( m_nicks );
    while(it.current()){
	if(it.current()->server() == server)
	    m_nicks.remove(it.currentKey());
	else
            ++it;
    }
    createMainPopup();
}

void dockServerController::startBlink(const QString& reason, const QString& text)
{
    if(m_blinkActive == false){
        // change icon (to "someone's talking to you" icon)
        setPixmap( m_pic_info );
        m_blinkStatus = true;
        m_blinkActive = true;
	m_blinkTimer->start( 500 ); // half a second
    }
    if(!reason.isNull()){
        QString br(reason);
        if(text.isNull() && ksopts->runDocked && ksopts->dockPopups)
	    KPassivePopup::message(QString("Notify: %1").arg(reason), this);
	else {
	    QStringList sl;
	    QString cutup = text;
	    int i = 0;
	    br = reason + " " + text.left(50);
	    if(text.length() > 50)
                br.append("...");
	    while(!cutup.isEmpty() && i++ < 3){
		sl.append(cutup.left(50));
                cutup.remove(0, 50);
	    }
	    cutup = sl.join("\n");
	    cutup.truncate(cutup.length()-1);
	    if(ksopts->runDocked && ksopts->dockPopups)
		KPassivePopup::message(QString("%1").arg(reason), cutup, this);
	}
	m_blink_reason.append(br);
	createMainPopup();
	kdDebug(5008) << "Blink reason: " << br << endl;

	QRegExp rx("(\\S+) -> (\\S+)");
	if(rx.search(reason) >= 0){
	    QString server = rx.cap(1);
	    QString nick = rx.cap(2);

	    m_last_server = server;
	    m_last_nick = nick;
	}
    }
}

void dockServerController::stopBlink(const QString& reason, bool clear)
{
    if(clear == true){
        m_blinkActive = false;
        m_blinkTimer->stop();
        setPixmap( m_pic_dock );
        m_blink_reason.clear();
    }
    else {
        if(!reason.isNull())
            m_blink_reason.remove(reason);
    }
    createMainPopup();
}

void dockServerController::blinkClear()
{
    m_sc->resetNotification();
}

void dockServerController::mainActivated(int id)
{
    QRegExp rx("(\\S+) -> (\\S+) ");
    if(rx.search(mainPop->text(id)) >= 0){
        QString server = rx.cap(1);
        QString nick = rx.cap(2);

        raiseWindow(server, nick);

    }
}

void dockServerController::raiseLastActiveWindow()
{
    if((!m_last_server.isNull()) && (!m_last_nick.isNull()))
        raiseWindow(m_last_server, m_last_nick);
}

void dockServerController::blinkDockedIcon()
{
    m_blinkStatus = !m_blinkStatus;
    setPixmap( m_blinkStatus ? m_pic_info : m_pic_dock );
}

void dockServerController::helpNotice()
{
    KMessageBox::information(0x0,
			     i18n("This popup menu can show a list of "
				  "people you have in your notify "
				  "list, and their status. You can configure "
				  "this list by going to Configure KSirc->"
				  "Startup->Notify and adding people to the "
				  "list. This will take effect the next "
				  "time you connect to a server. "
				  "This message appears when "
				  "there is nothing in your notify "
				  "list or when no one in your "
				  "list is online."
				 ),
			     i18n("Help for Notification Popup"));

}

void dockServerController::configNotify()
{
    KSPrefs *kp = new KSPrefs();
    connect(kp, SIGNAL(update(int)),
	    m_sc, SLOT(configChange()));
    kp->resize(550, 450);
    kp->showPage(2); /* Show auto connect page */
    kp->show();

}


void dockServerController::raiseWindow(QString server, QString name)
{
    QCString txt = server.utf8() + "_" + name.utf8() + "_toplevel";
    QWidget *obj = dynamic_cast<QWidget *>( objFinder::find(txt, "KSircTopLevel"));
    if(obj == 0x0){
	txt = server.utf8() + "_!" + name.utf8() + "_toplevel";
	obj = dynamic_cast<QWidget *>( objFinder::find(txt, "KSircTopLevel"));
    }

    if(obj != 0x0){
	displayMgr->raise(obj, true);
    }
    else {
	kdWarning() << "Did not find widget ptr to raise it" << endl;
    }
}

#include "dockservercontroller.moc"

