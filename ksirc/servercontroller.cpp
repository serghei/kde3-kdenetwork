 /**********************************************************************

 Server Controller

 $$Id: servercontroller.cpp 402276 2005-03-31 21:19:22Z woebbe $$

 Main Server Controller.  Displays server connection window, and makes
 new server connection on demand.

 Signals: NONE

 Slots:

   new_connection(): Creates popup asking for new connection

   new_ksircprocess(QString):
      Args:
         QString: new server name or IP to connect to.
      Action:
	 Creates a new sirc process and window !default connected to the
	 server.  Does nothing if a server connection already exists.

   add_toplevel(QString parent, QString child):
      Args:
	   parent: the server name that the new channel is being joined on
	   child: the new channel name
      Action:
         Adds "child" to the list of joined channles in the main
	 window.  Always call this on new window creation!

   delete_toplevel(QString parent, QString child):
      Args:
         parent: the server name of which channel is closing
	         child: the channle that is closing. IFF Emtpy, parent is
		 deleted.
      Action:
	 Deletes the "child" window from the list of connections.  If
	 the child is Empty the whole tree is removed since it is assumed
         the parent has disconnected and is closing.

   new_channel:  Creates popup asking for new channel name

   new_toplevel(QString str):
      Args:
         str: name of the new channel to be created
      Action:
         Sends a signal to the currently selected server in the tree
         list and join the requested channel.  Does nothing if nothing
         is selected in the tree list.

   recvChangeChanel(QString parent, QString old, QString new):
      Args:
         parent: parent server connection
         old: the old name for the window
         new: the new name for the window
      Action:
          Changes the old window name to the new window name in the tree
          list box.  Call for all name change!

 *********************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "servercontroller.h"
#include "KSOpenkSirc/open_ksirc.h"
#include "NewWindowDialog.h"
#include "ksopts.h"
#include "control_message.h"
#include "FilterRuleEditor.h"
#include "../config.h"
#include "version.h"
#include "KSPrefs/ksprefs.h"
#include "toplevel.h"
#include "ksircserver.h"
#include "nickColourMaker.h"

#include <stdlib.h>

#include "objFinder.h"
#include <qlabel.h>
#include <qregexp.h>

#include <kmenubar.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <kwin.h>
#include <kdebug.h>
#include <kstdaccel.h>
#include <kstandarddirs.h>
#include <khelpmenu.h>
#include <kstdaction.h>
#include <kaction.h>
#include <knotifydialog.h>
#include <netwm.h>
#include <kpassivepopup.h>
#include <kglobalaccel.h>
#include <kstdguiitem.h>

#include <qfile.h>

#include "displayMgrSDI.h"
#include "displayMgrMDI.h"

#include "dockservercontroller.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#include <klocale.h>
#include <kapplication.h>
#include <kpopupmenu.h>

DisplayMgr *displayMgr;

servercontroller *servercontroller::s_self = 0;

servercontroller::servercontroller( QWidget*, const char* name )
    : KMainWindow( 0, name )
{
  we_are_exiting = false;
  m_notificationCount = 0;

  m_ncm = new nickColourMaker();

  MenuBar = menuBar();
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );

  s_self = this;

  switch (ksopts->displayMode)
  {
    case KSOptions::SDI:
        displayMgr = new DisplayMgrSDI();
        break;
    case KSOptions::MDI:
        displayMgr = new DisplayMgrMDI();
        break;
  }

  sci = new scInside(this, QCString(name) + "_mainview");
  setCentralWidget(sci);

  sci->setFrameStyle(QFrame::Box | QFrame::Raised);
  ConnectionTree = sci->ConnectionTree;

  connect(ConnectionTree, SIGNAL(clicked( QListViewItem * )),
	  this, SLOT(WindowSelected(QListViewItem *)));

  setFrameBorderWidth(5);

  QPopupMenu *file = new QPopupMenu(this, QCString(name) + "_menu_file");
  KStdAction::quit(this, SLOT(endksirc()), actionCollection())->plug(file);
#ifndef NDEBUG
  file->insertItem(i18n("Dump Object Tree"), this, SLOT(dump_obj()));
  file->insertItem(i18n("Server Debug Window"), this, SLOT(server_debug()));
#endif
  MenuBar->insertItem(i18n("&File"), file);

  connections = new QPopupMenu(this, QCString(name) + "_menu_connections");

  server_id = connections->insertItem(i18n("&New Server..."), this, SLOT(new_connection()), Key_F2 );
  join_id = connections->insertItem(i18n("&Join Channel..."), this, SLOT(new_channel()), Key_F3 );
  connections->insertSeparator();
  connections->insertItem(i18n("&Do Autoconnect..."), this, SLOT(start_autoconnect_check()));
  connections->setItemEnabled(join_id, FALSE);
  MenuBar->insertItem(i18n("&Connections"), connections);

  options = new QPopupMenu(this, QCString(name) + "_menu_options");
  options->setCheckable(TRUE);

  options->insertItem(SmallIcon( "filter" ), i18n("&Filter Rule Editor..."),
		      this, SLOT(filter_rule_editor()));
  options->insertSeparator();
  KStdAction::configureNotifications(this, SLOT(notification_prefs()), actionCollection())->plug(options);

  KStdAction::preferences(this, SLOT(general_prefs()), actionCollection())->plug(options);

  MenuBar->insertItem(i18n("&Settings"), options);

  KHelpMenu *help = new KHelpMenu( this, kapp->aboutData() );
  MenuBar->insertItem( KStdGuiItem::help().text(), help->menu() );

  m_kga = new KGlobalAccel(this, "globalAccess");
  m_kga->insert("New Server", i18n("New Server"),
		i18n("This action allows you to open a new server more easily "
		     "when in docked mode, since you don't need to click on the "
		     "dock icon."),
		ALT+CTRL+Key_C, KKey::QtWIN+CTRL+Key_C, this,
		SLOT(new_connection()));

  open_toplevels = 0;

  pic_server = UserIcon("server");
  pic_gf = UserIcon("ksirc_a");
  pic_run = UserIcon("mini-run");
  pic_ppl = UserIcon("channels");
  pic_icon = UserIcon("ksirc_b");

  setCaption( i18n("Server Control") );
  KWin::setIcons(winId(), pic_icon, pic_server);

  resize( 450,200 );

  dockWidget = new dockServerController(this, 0x0, "servercontroller_dock");
  KWin::setSystemTrayWindowFor( dockWidget->winId(), winId() );

  m_kga->readSettings();
  m_kga->updateConnections();

  checkDocking();
}


servercontroller::~servercontroller()
{
  kdDebug(5008) << "~servercontroller in" << endl;
  s_self = 0;
  delete m_ncm;
  kdDebug(5008) << "~servercontroller out" << endl;
}

void servercontroller::checkDocking()
{
    if(ksopts->runDocked == true){
	dockWidget->show();
	hide();
        m_kga->setEnabled(true);
    }
    else {
        dockWidget->hide();
	show();
	m_kga->setEnabled(false);
    }

}

void servercontroller::new_connection()
{
  open_ksirc *w = new open_ksirc(); // Create new ksirc popup
  connect(w, SIGNAL(open_ksircprocess(KSircServer &)), // connected ok to process
          this, SLOT(new_ksircprocess(KSircServer &))); // start
  w->exec();                                       // show the sucker!
  delete w;
}

void servercontroller::new_ksircprocess(KSircServer &kss)
{
  QString server_id;
  int id = 1;
  if(kss.server().isEmpty())  // nothing entered, nothing done
    return;
  server_id = kss.server();
  while(proc_list[server_id]){   // if it already exists, quit
    server_id = QString("%1 %2").arg(kss.server()).arg(id++);
  }

  // Insert new base
  QListViewItem *rootItem = new QListViewItem( ConnectionTree, server_id );
  rootItem->setPixmap( 0, pic_server );
  rootItem->setOpen( true );

  // We do no_channel here since proc emits the signal in the
  // constructor, and we can't connect to before then, so we have to
  // do the dirty work here.
  ProcMessage(server_id, ProcCommand::addTopLevel, QString("no_channel"));

  KSircProcess *proc = new KSircProcess(server_id, kss, 0, (QString(name()) + "_" + server_id + "_ksp").ascii() ); // Create proc
  //this->insertChild(proc);                           // Add it to out inheritance tree so we can retreive child widgets from it.
  objFinder::insert(proc);
  proc_list.insert(server_id, proc);                      // Add proc to hash
  connect(proc, SIGNAL(ProcMessage(QString, int, QString)),
	  this, SLOT(ProcMessage(QString, int, QString)));
  connect(this, SIGNAL(ServMessage(QString, int, QString)),
	  proc, SLOT(ServMessage(QString, int, QString)));

  if(!ConnectionTree->currentItem()){   // If nothing's highlighted
    ConnectionTree->setCurrentItem(rootItem);     // highlight it.
  }

  connections->setItemEnabled(join_id, TRUE);

  dockWidget->serverOpen(server_id);
}

void servercontroller::new_channel()
{
  QString server;
  QListViewItem *citem = ConnectionTree->currentItem(); // get item
  if(citem){ // if it exist, ie something is highlighted, continue
    if(proc_list[citem->text(0)]){ // If it's a match with a server, ok
      server = citem->text(0);
    }
    // Otherwise, check the parent to see it's perhaps a server.
    else if ( citem->parent() ) {
      if(proc_list[citem->parent()->text(0)]){
        server = citem->parent()->text(0);
      }
    }
  }

  if(server.isEmpty())
    return;

  KSircChannel ci(server, QString::null);
  NewWindowDialog w(ci);
  connect(&w, SIGNAL(openTopLevel(const KSircChannel &)),
	  this, SLOT(new_toplevel(const KSircChannel &)));
  w.exec();
}

void servercontroller::new_toplevel(const KSircChannel &channelInfo)
{
  new_toplevel(channelInfo, false);
}

void servercontroller::new_toplevel(const KSircChannel &channelInfo, bool safe)
{
    if(proc_list[channelInfo.server()]){ // If it's a match with a server, ok
        proc_list[channelInfo.server()]->new_toplevel(channelInfo, safe);
    }
}

void servercontroller::ToggleAutoCreate()
{
    ksopts->autoCreateWin = !ksopts->autoCreateWin;
}

void servercontroller::general_prefs()
{
  KSPrefs *kp = new KSPrefs();
  connect(kp, SIGNAL(update(int)),
          this, SLOT(configChange()));
  kp->resize(550, 450);
  kp->show();
}

void servercontroller::notification_prefs()
{
  KNotifyDialog::configure(this, "Notification Configuration Dialog");
}

void servercontroller::dump_obj()
{

	objFinder::dumpTree();

}
void servercontroller::server_debug()
{
  QListViewItem *citem = ConnectionTree->currentItem(); // get item
  if(citem){ // if it exist, ie something is highlighted, continue
      QString server;
      if(proc_list[citem->text(0)]){ // If it's a match with a server, ok
          server = citem->text(0);
      }
      else if ( citem->parent() ) {
	  if(proc_list[citem->parent()->text(0)]){
	      server = citem->parent()->text(0);
	  }
      }

      if( !server.isNull() ){
          bool sh = proc_list[server]->getIOController()->isDebugTraffic();
          proc_list[server]->getIOController()->showDebugTraffic(!sh);
      }
  }


}

void servercontroller::filter_rule_editor()
{
  FilterRuleEditor *fe = new FilterRuleEditor();
  connect(fe, SIGNAL(destroyed()),
	  this, SLOT(slot_filters_update()));
  fe->show();
}

void servercontroller::font_update(const QFont &font)
{
  ksopts->defaultFont = font;
/*  configChange(); */

  KConfig *kConfig = kapp->config();
  kConfig->setGroup("GlobalOptions");
  kConfig->sync();
  QApplication::setFont( font, true, "KSirc::TextView" );
}

void servercontroller::configChange()
{
  QDictIterator<KSircProcess> it( proc_list );
  while(it.current()){
    it.current()->filters_update();
    it.current()->getWindowList()["!all"]->control_message(REREAD_CONFIG, "");
    ++it;
  }
  m_kga->readSettings();
  m_kga->updateConnections();
}

void servercontroller::ProcMessage(QString server, int command, QString args)
{
  QListViewItem *serverItem = 0L;
  QListViewItem *item = ConnectionTree->firstChild();
  while ( item ) {
      if ( !item->parent() && item->text(0) == server ) {
	  serverItem = item;
	  break;
      }
      item = item->nextSibling();
  }

  if ( !serverItem ) {
      kdDebug(5008) << "* ProcMessage for non-existant server?! - " << server<< endl;
      return;
  }


  switch(command){


    // Nick offline and online both remove the nick first.
    // We remove the nick in case of an online so that we don't get
    // duplicates.
    // Args == nick comming on/offline.
  case ProcCommand::nickOffline:
      {
          QListViewItem *online_item = findChild(serverItem, i18n("Online"));
          if(online_item){
              item = findChild(online_item, args);
              delete item;
              if(online_item->childCount() == 0)
                  delete online_item;
              if(ksopts->runDocked && ksopts->dockPopups)
                  KPassivePopup::message(i18n("%1 just went offline on %2").arg(args).arg(server), dockWidget);
          }
          dockWidget->nickOffline(server, args);
          break;
      }
  case ProcCommand::nickOnline:
      {
          QListViewItem *online_item = findChild(serverItem, i18n("Online"));
          if(!online_item){
              online_item = new QListViewItem(serverItem, i18n("Online"));
              online_item->setPixmap( 0, pic_gf );
              online_item->setOpen( true );
          }
          else {
              item = findChild(online_item, args);
              if( item ){
                  delete item;
              }
          }
          item = new QListViewItem(online_item, args);
	  item->setPixmap( 0, pic_run );
	  if(ksopts->runDocked && ksopts->dockPopups)
	      KPassivePopup::message(i18n("%1 just came online on %2").arg(args).arg(server), dockWidget);
          dockWidget->nickOnline(server, args);
          break;
      }
      /*
    // Add new channel, first add the parent to the path
    path.push(&server);
    path.push(&online);
    path.push(&args);
    // Remove old one if it's there
    ConnectionTree->removeItem(&path); // Remove the item
    path.pop();
    // add a new child item with parent as its parent
    ConnectionTree->addChildItem(args, pic_run, &path);
    if (kSircConfig->BeepNotify) {
      KNotifyClient::beep();
    }
    break;
    */

    /**
      *  Args:
      *	   parent: the server name that the new channel is being joined on
      *    child:  the new channel name
      *  Action:
      *    Adds "child" to the list of joined channles in the main
      *    window.  Always call this on new window creation!
      */
  case ProcCommand::addTopLevel:
    // Add new channel
    if(args[0] == '!')
      args.remove(0, 1); // If the first char is !, it's control, remove it
    // add a new child item with parent as it's parent
    item = new QListViewItem( serverItem, args );
    item->setPixmap( 0, pic_ppl );

    open_toplevels++;
    break;
    /**
      *  Args:
      *    parent: the server name of which channel is closing
      *	   child: the channle that is closing. IFF Emtpy, parent is
      *	   deleted.
      *  Action:
      *	   Deletes the "child" window from the list of connections.  If
      *	   the child is Empty the whole tree is removed since it is assumed
      *    the parent has disconnected and is closing.
      */
  case ProcCommand::deleteTopLevel:
    // If the child is emtpy, delete the whole tree, otherwise just the child
    if(args[0] == '!')
      args.remove(0, 1); // If the first char is !, it's control, remove it

    item = findChild( serverItem, args );
    delete item;
    if ( serverItem->childCount() == 0 )
	delete serverItem;

    open_toplevels--;
    break;

  /**
      *  Args:
      *    parent: parent server connection
      *    old: the old name for the window
      *    new: the new name for the window
      *  Action:
      *    Changes the old window name to the new window name in the tree
      *    list box.  Call for all name change!
      */
  case ProcCommand::changeChannel:
    {
      char *new_s, *old_s;
      new_s = new char[args.length()+1];
      old_s = new char[args.length()+1];
      sscanf(args.ascii(), "%s %s", old_s, new_s);
      //  If the channel has a !, it's a control channel, remove the !
      if(old_s[0] == '!')
        // Even though, we want strlen() -1 characters, strlen doesn't
        // include the \0, so we need to copy one more. -1 + 1 = 0.
	memmove(old_s, old_s+1, strlen(old_s));
      if(new_s[0] == '!')
	memmove(new_s, new_s+1, strlen(new_s)); // See above for strlen()

      item = findChild( serverItem, old_s );
      delete item;
      item = new QListViewItem( serverItem, new_s );
      item->setPixmap( 0, pic_ppl );

      delete[] new_s;
      delete[] old_s;
    }
    break;
  case ProcCommand::procClose:
    dockWidget->serverClose(server);
    delete serverItem;
    proc_list.remove(server); // Remove process entry while we are at it
    if(proc_list.count() == 0){
      ConnectionTree->clear();
      connections->setItemEnabled(join_id, FALSE);
    }
    break;
  case ProcCommand::turnOffAutoCreate:
    if (ksopts->autoCreateWin) {
      ToggleAutoCreate();
    }
    break;
  case ProcCommand::turnOnAutoCreate:
    if (!ksopts->autoCreateWin) {
      ToggleAutoCreate();
    }
    break;
  default:
    kdDebug(5008) << "Unkown command: [" << command << "] from "
              << server
              << " " << args << endl;
  }
}

void servercontroller::slot_filters_update()
{
  emit ServMessage(QString(), ServCommand::updateFilters, QString());
}

void servercontroller::saveGlobalProperties(KConfig *ksc)
{
  // ksc hos the K Session config
  // ksp == current KSircProcess
  // ksm == current KSircMessageReceiver

  // Ignore all !<name> windows

  QString group = ksc->group();

  ksc->setGroup( "KSircSession" );
  SessionConfigMap::ConstIterator it = m_sessionConfig.begin();
  for (; it != m_sessionConfig.end(); ++it ) {

      ChannelSessionInfoList infoList = *it;

      QStringList channels;
      QString port = "6667";
      QStringList desktops;

      for ( ChannelSessionInfoList::ConstIterator sessionInfoIt = infoList.begin();
          sessionInfoIt != infoList.end(); ++sessionInfoIt ) {
	  channels << ( *sessionInfoIt ).name;
	  port = ( *sessionInfoIt ).port;
	  desktops << QString::number( ( *sessionInfoIt ).desktop );
      }

      KConfigGroup( ksc, "KSircSession" ).writeEntry( it.key(), channels );
      KConfigGroup( ksc, "KSircSessionPort" ).writeEntry( it.key(), port );
      KConfigGroup( ksc, "KSircSessionDesktopNumbers" ).writeEntry( it.key(), desktops );
  }

  ksc->setGroup("ServerController");
//  ksc->writeEntry("Docked", !isVisible());
  ksc->writeEntry("Size", geometry());
  ksc->setGroup(group);
}

void servercontroller::readGlobalProperties(KConfig *ksc)
{
    QString group = ksc->group();

    // ksc == K Session Config

    // KMainWindow silently disables our menubar, when we quit in a docked
    // state, so we have to force showing it here.
    menuBar()->show();

    // commented in for testing...
    ksc->setGroup( "KSircSession" );
    QMap<QString,QString> keyMap = ksc->entryMap( ksc->group() );
    QMap<QString,QString>::Iterator it = keyMap.begin();

    while(it != keyMap.end()) {
	QString server = it.key();
	QString port = KConfigGroup( ksc, "KSircSessionPort" ).readEntry( server );
	//	debug("%s", it.key().latin1());
        KSircServer kss(server, port);
	new_ksircprocess( kss ); // sets up proc_list
	QStringList channels = ksc->readListEntry( server );

	QStringList desktops = KConfigGroup( ksc, "KSircSessionDesktopNumbers" ).readListEntry( server );

	for(uint i = 0; i < channels.count(); i++){

	    QString channel = channels[ i ];

	    proc_list[ server ]->new_toplevel( KSircChannel(server, channel), true );

	    KSircTopLevel *topLevel = dynamic_cast<KSircTopLevel *>( proc_list[ server ]->getWindowList()[ channel ] );
	    if ( !topLevel || !topLevel->isTopLevel() )
		continue;

	    QStringList::ConstIterator desktopNumberIt = desktops.at( i );
	    if ( desktopNumberIt == desktops.end() )
		continue;

	    int desktop = ( *desktopNumberIt ).toInt();
	    if ( desktop == -1 )
		continue;
#ifdef Q_WS_X11
	    NETWinInfo winInfo( qt_xdisplay(), topLevel->winId(), qt_xrootwin(), NET::WMDesktop );
	    winInfo.setDesktop( desktop );
#endif
	}
	++it;
    }

    QRect geom;

//    ksc->setGroup("ServerController");
//    bool docked = ksc->readBoolEntry("Docked", FALSE);
//    if ( !docked )
    //      show();
    if(ksopts->runDocked == false)
        show();

    geom = ksc->readRectEntry("Size");
    if(! geom.isEmpty())
    	setGeometry(geom);

    ksc->setGroup(group);
}

void servercontroller::saveSessionConfig()
{
    QDictIterator<KSircProcess> ksp(proc_list);
    for (; ksp.current(); ++ksp ) {
	ChannelSessionInfoList channels;

	QDictIterator<KSircMessageReceiver> ksm(ksp.current()->getWindowList());
	for (; ksm.current(); ++ksm )
	    if(ksm.currentKey()[0] != '!') { // Ignore !ksm's (system created)
		ChannelSessionInfo sessionInfo;

		sessionInfo.name = ksm.currentKey();
		sessionInfo.port = ksp.current()->serverPort();
		KSircTopLevel *topLev = dynamic_cast<KSircTopLevel *>( ksm.current() );
		if ( topLev && topLev->isTopLevel() ) {
#ifdef Q_WS_X11
		    NETWinInfo winInfo( qt_xdisplay(), topLev->winId(), qt_xrootwin(), NET::WMDesktop );
		    sessionInfo.desktop = winInfo.desktop();
#endif
		}

		channels << sessionInfo;
	    }

	if ( !channels.isEmpty() )
	    m_sessionConfig[ ksp.currentKey() ] = channels;
    }
}

void servercontroller::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  if ( !e->spontaneous() )
    saveDockingStatus();
}

void servercontroller::hideEvent( QHideEvent * )
{
    /*
  QWidget::hideEvent( e );
  if ( !e->spontaneous() )
    saveDockingStatus();
  if(QWidget::isMinimized()){
    hide();
    KWin::setState(winId(), NET::SkipTaskbar);
    }
    */
}

void servercontroller::saveDockingStatus()
{
    if ( we_are_exiting ) // we are hidden by closeEvent
	return;

//    KConfig *kConfig = kapp->config();
//    KConfigGroupSaver s( kConfig, "ServerController" );
//    kConfig->writeEntry("Docked", !isVisible());
//    kConfig->sync();
}

void servercontroller::endksirc(){
    kapp->config()->sync();
    exit(0);
}

void servercontroller::closeEvent( QCloseEvent *e )
{
    we_are_exiting = true;
    saveSessionConfig();
    KMainWindow::closeEvent( e );
}

void servercontroller::WindowSelected(QListViewItem *item)
{
    if ( !item )
	return;

  QListViewItem *parent_server = item->parent();
  if(!parent_server)
    return;

  QString txt = QString(parent_server->text(0)) + "_" + item->text(0) + "_toplevel";
  QWidget *obj = dynamic_cast<QWidget *>( objFinder::find(txt.utf8(), "KSircTopLevel"));
  if(obj == 0x0){
    txt =QString(parent_server->text(0)) + "_!" + item->text(0) + "_toplevel";
    obj = dynamic_cast<QWidget *>( objFinder::find(txt.utf8(), "KSircTopLevel"));
  }

  if(obj != 0x0){
    displayMgr->raise(obj);
  }
  else {
    kdWarning() << "Did not find widget ptr to raise it" << endl;
  }
}


QListViewItem * servercontroller::findChild( QListViewItem *parent,
					     const QString& text )
{
    if ( !parent || parent->childCount() == 0 ) {
	return 0L;
    }

    QListViewItem *item = parent->firstChild();
    while ( item ) {
	if ( item->text(0) == text ) {
	    return item;
	}
	item = item->nextSibling();
    }

    return 0L;
}

void servercontroller::increaseNotificationCount(const QString& reason, const QString& text)
{
    dockWidget->startBlink(reason, text);
    m_notificationCount++;
}

void servercontroller::decreaseNotificationCount(QString reason)
{
    m_notificationCount--;
    if ( m_notificationCount == 0 )
    {
        dockWidget->stopBlink(reason, true);
    }
    else {
        dockWidget->stopBlink(reason, false);
    }
}

void servercontroller::resetNotification()
{
    m_notificationCount = 0;
    dockWidget->stopBlink(QString::null, true);
    QDictIterator<KSircProcess> it( proc_list );
    while(it.current()){
	it.current()->filters_update();
	it.current()->getWindowList()["!all"]->control_message(RESET_NOTIF, "");
	++it;
    }
}

void servercontroller::do_autoconnect()
{
    static int stime = 0;
    static int ctime = 0;
    int loop;

    kdDebug(5008) << "Doing AUTOCONNECT" << endl;

    KConfig *conf = kapp->config();
    conf->setGroup("AutoConnect");
    QStringList servers = conf->readListEntry("Servers");
    servers.sort();
    QStringList::ConstIterator ser = servers.begin();

    loop = 0;

    for( ; ser != servers.end(); ser++){
        if(loop++ == stime){
		stime++;
		QString server = *ser;
		QString port = "6667";
		bool usessl = false;
		QString pass = QString::null;

		QRegExp rx("(.+) \\(SSL\\)(.*)");
		if(rx.search(server) >= 0){
		    server = rx.cap(1) + rx.cap(3);
                    usessl = true;
		}
		rx.setPattern("(.+) \\(pass: (\\S+)\\)(.*)");
		if(rx.search(server) >= 0){
		    server = rx.cap(1) + rx.cap(3);
		    pass = rx.cap(2);
		}
		rx.setPattern("([^: ]+):(\\d+)");
		if(rx.search(server) >= 0){
		    server = rx.cap(1);
		    port = rx.cap(2);
		}
		kdDebug(5008) << server << ": Done " << port << " " << usessl << " " << pass << endl;
                KSircServer kss(server, port, "", pass, usessl);
		new_ksircprocess(kss);
                return;
        }
    }

    loop = 0;

    ser = servers.begin();
    for( ; ser != servers.end(); ser++){
        QStringList channels = conf->readListEntry(*ser);
        if(channels.empty() == FALSE){
            channels.sort();
            QStringList::ConstIterator chan = channels.begin();
            for(; chan != channels.end(); chan++){
                if(loop++ == ctime){
		    ctime++;
		    QString channel = *chan;
		    QString key = QString::null;
		    QRegExp crx("(.+) \\(key: (\\S+)\\)");
		    if(crx.search(channel) >= 0){
			channel = crx.cap(1);
			key = crx.cap(2);
		    }
		    QString server = *ser;

		    QRegExp rx("^([^ :]+)");
		    if(rx.search(server) >= 0){
			server = rx.cap(1);
		    }
		    kdDebug(5008) << server << ": Channed: " << channel << " key: " << key << endl;
		    new_toplevel(KSircChannel(server, channel, key), true);
		    return;
		}
            }
        }
    }

    ctime = 0;
    stime = 0;
    at->stop();
    delete at;

}

void servercontroller::start_autoconnect() {
    at = new QTimer(this);
    connect(at, SIGNAL(timeout()), this, SLOT(do_autoconnect()));
    at->start(250, FALSE);
}

void servercontroller::start_autoconnect_check() {

    KConfig *conf = kapp->config();
    conf->setGroup("AutoConnect");
    QStringList servers = conf->readListEntry("Servers");

    if(servers.count() == 0){
	KSPrefs *kp = new KSPrefs();
	connect(kp, SIGNAL(update(int)),
		this, SLOT(configChange()));
	kp->resize(550, 450);
        kp->showPage(7); /* Show auto connect page */
	kp->show();
    }
    else {
	at = new QTimer(this);
	connect(at, SIGNAL(timeout()), this, SLOT(do_autoconnect()));
	at->start(250, FALSE);
    }

}


scInside::scInside ( QWidget * parent, const char * name, WFlags
		     f )
  : QFrame(parent, name, f)
{
  ASConn = new QLabel(i18n("Active server connections:"), this, "servercontroller_label");
  QFont asfont = ASConn->font();
  asfont.setBold(TRUE);
  ASConn->setFont(asfont);

  ConnectionTree = new KListView(this, "connectiontree");
  ConnectionTree->addColumn(QString::null);
  ConnectionTree->setRootIsDecorated( true );
  ConnectionTree->setSorting( 0 );
  ConnectionTree->header()->hide();
}

scInside::~scInside()
{
  delete ASConn;
  delete ConnectionTree;
}

void scInside::resizeEvent ( QResizeEvent *e )
{
  QFrame::resizeEvent(e);
  ASConn->setGeometry(10,10, width() - 20,
		      ASConn->fontMetrics().height()+5);
  ConnectionTree->setGeometry(10, 10 + ASConn->height(),
                              width() - 20, height() - 20 - ASConn->height());
}

#include "servercontroller.moc"
