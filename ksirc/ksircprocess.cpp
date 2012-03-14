/*************************************************************************

 KSircProcess, sirc controller

 $$Id: ksircprocess.cpp 437911 2005-07-23 14:56:02Z binner $$

 KSircProcess cerate and controls toplevel widgets and sirc process'.
 Each sirc process has 1 and only 1 KSircProcess to control it.  KSirc
 process passes all IO to IOController which is it's friend.

 Interface:

 public:
     KSircProcess(*server=0L, *parent=0, *name=0)
       server: is the name of the server to connect to.  It must be
       provided or else start sirc will barf. :(
       parent: parent window, this _should_ be null
       name: name, passed to QObject...

     ~KSirProcess:
       kill the sirc process, and iocontrollller, emit delete_toplevel

     getWindowList:
       returns the TopList, see bellow.

   Signals:
     made_toplevel(server, window)
       made a new toplevel window for the "server" we are connected to
       with "window" as the title.

     dalete_toplevel(server, window)
       delete toplevel with server and called window.  If we emit null
       as the window name it means to destroy all info about the
       server and ksircprocess.

     changeChannel(server, old_name, new_name)
       toplevel with old_name has been changed to new_name and all
      future refrences will use new_name.

   public slots:
     new_toplevel(window):
       create a new window with name window.  This MAY only change the
       name of an existing window that's now idle.

     close_topevel(KsircTopLevel*, window):
       deletes all refrences to window and if needed finds a new
       default toplevel.

     default_window(KSricTopLevel*):
       KSircTopLevel is requesting change to !default.  Be carefull
       with this one.

     recvChangeChannel(old, new):
       window old is changing to new.  emit ChangeChannel with server
       name added.  Without server name we can uniqely id the window. :(

 Implementation:

   Bassic process is to create a new KSircProcess and it takes care of
   the rest.  It emits signals for each new window and every time a
   window is delete so you can update external display (like
   servercontroller uses).

   Startup:

   1. Creates a case insensitive TopList.  This is a list of ALL
   KSircReceivers under control of this server, and includes such
   items as "!all" and "!default".  All !name are control windows.

   2. Forks off a KProcess for sirc and passes it over to IOController
   which grabs and control's it's IO.

   3. It then opens a "!default" window.  This will receive all
   initial input and such.  It WILL change it's name on the first
   join.

   4. The IO broadcast object is created and setup.

   5. everything is put into run mode.


   Operation, see code bellow for inline comments.

*************************************************************************/



#include "baserules.h"
#include "ksopts.h"
#include "control_message.h"
#include "displayMgr.h"
#include "ioBroadcast.h"
#include "ioDCC.h"
#include "ioDiscard.h"
#include "ioLAG.h"
#include "ioNotify.h"
#include "iocontroller.h"
#include "ksircprocess.h"
#include "objFinder.h"
#include "servercontroller.h"
#include "toplevel.h"
#include "version.h"
#include "KSProgress/ksprogress.h"

#include <stdlib.h>
#include <time.h>

#include <qtimer.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>


extern DisplayMgr *displayMgr;
             
KSircProcess::KSircProcess( QString &server_id, KSircServer &kss, QObject * parent, const char * name )
  : QObject(parent, name), m_kss(kss), m_serverid(server_id)
{

  proc = new KProcess();

#ifndef NDEBUG
  if(getuid() != 0)
      proc->setRunPrivileged(true); /* make ksirc run under gdb as a user */
#endif

  //server = qstrdup(kss.server());

  QDict<KSircMessageReceiver> nTopList(17, FALSE);
  TopList = nTopList;
  //  TopList.setAutoDelete(TRUE)

  auto_create_really = FALSE;

  // Create the ksopts server structure
  ksopts->serverSetup(kss);

  // Setup the environment for KSirc
  QString qsNick, qsRealname, qsUserID, qsAltNick;
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("StartUp");
  qsNick = ksopts->serv(kss).nick;
  qsAltNick = ksopts->serv(kss).altNick;
  qsRealname = ksopts->serv(kss).realName;
  qsUserID = ksopts->serv(kss).userID;
  kdDebug(5008) << "qsNick: " << qsNick << " qsAltNick: " << qsAltNick << " qsRealname: " << qsRealname << "qsUserID: " << qsUserID << endl;

  m_nick = qsNick;

  if((qsNick.isEmpty() == FALSE)){
    proc->setEnvironment("SIRCNICK", qsNick);
  }
  if((qsAltNick.isEmpty() == FALSE)){
    proc->setEnvironment("BACKUPNICK", qsAltNick);
  }
  if((qsRealname.isEmpty() == FALSE)){
    proc->setEnvironment("SIRCNAME", qsRealname);
  }
  if((qsUserID.isEmpty() == FALSE)){
    proc->setEnvironment("SIRCUSER", qsUserID);
    kdDebug(5008) << "Set SIRCUSER to: " << qsUserID << endl;
  }

  proc->setEnvironment("SIRCLIB", KGlobal::dirs()->findResourceDir("appdata", "ksirc.pl"));
  proc->setEnvironment("SIRCWAIT", "1");

  QString env = locate("appdata", "ksircrc");
  if (!env.isEmpty())
    proc->setEnvironment("SIRCRC", env);
  env = locate("appdata", "ksircrc.pl");
  if (!env.isEmpty())
    proc->setEnvironment("SIRCRCPL", env);

  // Setup the proc now, so iocontroller can use it.  It's latter
  // though. started bellow though.

  proc->setName(QCString(name) + "_kprocess");
  objFinder::insert(proc);
//  insertChild(proc);

  // pass the server string using an environment variable, because it might contain
  // a password that could be spyed out, as the commandline option is readable to others.
  // Fixes 47157.
  proc->setEnvironment( "SIRCSERVER", "[" + kss.server() + "]:" + kss.port() + ":" + kss.password());

  QString sslopt;
  if(kss.usessl())
      sslopt = "-S";
  *proc << "perl" << KGlobal::dirs()->findExe("dsirc") << "-8" << "-r" << sslopt;

  // Finally start the iocontroller.

  iocontrol = new KSircIOController(proc, this);
  iocontrol->setName(QCString(name) + "_iocontrol");

  // Create toplevel before iocontroller so it has somewhere to write stuff.

  running_window = TRUE;        // True so we do create the default
  default_follow_focus = TRUE;
  KSircChannel ci(kss.server(), "!no_channel");
  new_toplevel(ci, true);  //
  TopList.replace("!default", TopList[ci.channel()]);

  running_window = FALSE;       // set false so next changes the first name

  // Write default commands, and open default windows.

  TopList.insert("!all", new KSircIOBroadcast(this));
  TopList.insert("!discard", new KSircIODiscard(this));

  KSircIODCC *dcc = new KSircIODCC(this);
  TopList.insert("!dcc", dcc);
  dcc = static_cast<KSircIODCC *>( TopList["!dcc"] ); // g++ bug
  connect(dcc, SIGNAL(outputLine(QCString)),
	  iocontrol, SLOT(stdin_write(QCString)));

  KSircIOLAG *lag = new KSircIOLAG(this);
  TopList.insert("!lag", lag);
  lag = static_cast<KSircIOLAG*>( TopList["!lag"] ); // g++ bug!
  connect(lag, SIGNAL(outputLine(QCString)),
	  iocontrol, SLOT(stdin_write(QCString)));

  KSircIONotify *notify = new KSircIONotify(this);
  TopList.insert("!notify", notify);
  notify = static_cast<KSircIONotify *>( TopList["!notify"] ); // g++ bug
  connect(notify, SIGNAL(notify_online(QString)),
	  this, SLOT(notify_forw_online(QString)));
  connect(notify, SIGNAL(notify_offline(QString)),
	  this, SLOT(notify_forw_offline(QString)));

  TopList.insert("!base_rules", new KSMBaseRules(this));

  // Now that all windows are up, start sirc.

  proc->start(KProcess::NotifyOnExit, KProcess::All);
  // Intial commands to load ASAP.
  // turn on sirc ssfe mode
  QCString command = "/eval $ssfe=1\n";
  iocontrol->stdin_write(command);

  command = "/eval $version .= \"+KSIRC/" + QCString(KSIRC_VERSION) + "\"\n";
  iocontrol->stdin_write(command);
  command = "/load " + locate("appdata", "filters.pl").local8Bit() + "\n";
  iocontrol->stdin_write(command);
  command = "/load " + locate("appdata", "ksirc.pl").local8Bit() + "\n";
  iocontrol->stdin_write(command);
  /*
  command = "/load " + locate("appdata", "puke.pl") + "\n";
  iocontrol->stdin_write(command);
  command = "/load " + locate("appdata",  "dcc_status.pm") + "\n";
  iocontrol->stdin_write(command);
  */
  command = "/eval $ready = 1\n";
  iocontrol->stdin_write(command);


  // Load all the filter rules.  Must be after /load filtes.pl so all
  // the functions are available

  filters_update();

  // We do this after filters_update() since filters_update loads the
  // require notify filters, etc.

  command = "/notify ";
  command += ksopts->serv(kss).notifyList.join(" ").latin1();
  command += "\n";
  kdDebug(5008) << "Notify: " << command << endl;
  iocontrol->stdin_write(command);

}

KSircProcess::~KSircProcess()
{
    cleanup();
}

QPtrList<KSircMessageReceiver> KSircProcess::messageReceivers() const
{
    QPtrList<KSircMessageReceiver> res;
    res.setAutoDelete( false );
    QDictIterator<KSircMessageReceiver> it( TopList );
    for (; it.current(); ++it )
        if ( it.currentKey() != "!default" &&
             it.currentKey() != "!no_channel" )
            res.append( it.current() );
    return res;
}

const QDict<KSircMessageReceiver> &KSircProcess::mrList() const
{
    return TopList;
}

void KSircProcess::cleanup()
{
  if(TopList["!default"]){
    TopList.remove("!default"); // remove default so we don't delete it twice.
  }

  TopList.setAutoDelete(true);
  TopList.clear();

  emit ProcMessage(m_serverid, ProcCommand::procClose, QString());

  // Do closing down commands, this should release all puke widgets
#if 0
  dsirc does this on SIGTERM (malte)
  QString quit_cmd = "/eval &dohooks(\"quit\");\n";
  proc->writeStdin(quit_cmd.ascii(), quit_cmd.length());
  sleep(1);
#endif
  if(proc->isRunning()){
      proc->kill(SIGTERM);
  }

  delete proc;               // Delete process, seems to kill sirc, good.
  delete iocontrol;          // Take out io controller
//  delete []server;

  proc = 0L;
  iocontrol = 0L;
//  server = 0L;
}

void KSircProcess::new_toplevel(const KSircChannel &channelInfo, bool safe)
{
  static time_t last_window_open = 0;
  static int number_open = 0;
  static bool flood_dlg = FALSE;

  if(running_window == FALSE){ // If we're not fully running, reusing
			       // !default window for next chan.
    running_window = TRUE;
    // insert and remove is done as a side effect of the control_message call
    // TopList.insert(str, TopList["!no_channel"]);
    // TopList.remove("!no_channel"); // We're no longer !no_channel
    TopList["!no_channel"]->control_message(CHANGE_CHANNEL, channelInfo.server() + "!!!" + channelInfo.channel() + "!!!" + channelInfo.key());
  }
  else if(TopList.find(channelInfo.channel()) == 0x0){ // If the window doesn't exist, continue
   // If AutoCreate windows is on, let's make sure we're not being flooded.
    if(ksopts->autoCreateWin == TRUE && safe == false){
      time_t current_time = time(NULL);
      if((channelInfo.channel()[0] != '#' || channelInfo.channel()[0] != '&') &&
	 ((current_time - last_window_open) < 5)){
	if(number_open > 4 && flood_dlg == FALSE){
	    flood_dlg = TRUE;
	    int res = KMessageBox::warningYesNo(0,
					      i18n("5 Channel windows were opened "
						   "in less than 5 seconds. Someone "
						   "may be trying to flood your X server "
						   "with windows.\n"
						   "Shall I turn off AutoCreate windows?"),
					      i18n("Flood Warning"), i18n("Turn Off"), i18n("Keep Enabled"));
	  switch(res) {
	  case KMessageBox::Yes:
	    emit ProcMessage(serverID(), ProcCommand::turnOffAutoCreate, QString());
	  }
	  last_window_open = current_time;
	  number_open = 0;
	}
	else{
	  // Joining channels can't be a flood, can it?
	  if(channelInfo.channel()[0] != '#' || channelInfo.channel()[0] != '&')
            if(!safe)
	      number_open++;
	}
	flood_dlg = FALSE;
      }
      else{
	last_window_open = current_time;
      }
    }

    // Create a new toplevel, and add it to the toplist.
    // TopList is a list of KSircReceivers so we still need wm.
    KSircTopLevel *wm = new KSircTopLevel(this, channelInfo, (serverID() +"_" + channelInfo.channel()).ascii() );
    TopList.insert(channelInfo.channel(), wm);

    // Connect needed signals.  For a message window we never want it
    // becomming the default so we ignore focusIn events into it.
    connect(wm, SIGNAL(outputLine(QCString)),
	    iocontrol, SLOT(stdin_write(QCString)));
    connect(wm, SIGNAL(open_toplevel(const KSircChannel &)),
	    this,SLOT(new_toplevel (const KSircChannel &)));
    connect(wm, SIGNAL(closing(KSircTopLevel *, QString)),
	    this,SLOT(close_toplevel(KSircTopLevel *, QString)));
    connect(wm, SIGNAL(currentWindow(KSircTopLevel *)),
	    this,SLOT(default_window(KSircTopLevel *)));
    connect(wm, SIGNAL(changeChannel(const QString &, const QString &)),
            this,SLOT(recvChangeChannel(const QString &, const QString &)));
    connect(wm, SIGNAL(destroyed(QObject *)),
            this,SLOT(clean_toplevel(QObject *)));
    connect( wm, SIGNAL( requestQuit( const QCString& ) ),
            SLOT( request_quit( const QCString& ) ) );

    default_window(wm); // Set it to the default window.
    emit ProcMessage(serverID(), ProcCommand::addTopLevel, channelInfo.channel());

    displayMgr->newTopLevel(wm, TRUE);
    displayMgr->setCaption(wm, channelInfo.channel());
//    displayMgr->show(wm);
    wm->lineEdit()->setFocus(); // Give focus back to the linee, someone takes it away on new create
  }
  else {
      QWidget *w = dynamic_cast<QWidget *>(TopList.find(channelInfo.channel()));
      if(w)
          displayMgr->raise(w);
  }
}

void KSircProcess::close_toplevel(KSircTopLevel *wm, QString name)
{
  if(auto_create_really == TRUE)
    turn_on_autocreate();

  kdDebug(5008) << "KSP: get close_toplevel: " << name << endl;

  // the removeTopLevel below also deletes the mditoplevel (in case
  // we are using mdi) , which deletes its children, which deletes
  // 'wm' , so watch out not to delete twice! (Simon)
  QGuardedPtr<KSircTopLevel> guardedwm = wm;
  // Do this now or we get junk left on the screen
  displayMgr->removeTopLevel(wm);

  while(TopList.remove(name)); // In case multiple copies exist remove them all

  bool isDefault = (wm == TopList["!default"]);

  // Ok, now if we just deleted the default we have a problem, we need
  // a new default.  BUT don't make the default "!all" or !message.
  // So let's go grab a default, and make sure it's not "!" control
  // object.

  QDictIterator<KSircMessageReceiver> it(TopList);
  for(;it.current() && it.currentKey().startsWith("!"); ++it);

  if (!it.current())
  { 
    // No top-level windows left.
    QCString command = "/quit\n"; // "/signoff" ?
    iocontrol->stdin_write(command); // kill sirc
    kdDebug(5008) << "KSP closing: " << m_kss.server() << endl;
    delete guardedwm;
    delete this; // Delete ourself, WARNING MUST RETURN SINCE WE NO
		 // LONGER EXIST!!!!
    return;      // ^^^^^^^^^^^^^^^
  }

  if (isDefault)
    TopList.replace("!default", it.current());

  // Let's let em know she's deleted!
  if(ksopts->autoCreateWin == TRUE){
      emit ProcMessage(serverID(), ProcCommand::turnOffAutoCreate, QString());
      QTimer::singleShot(5000, this, SLOT(turn_on_autocreate()));
      auto_create_really = TRUE;
  }
  else{
      auto_create_really = FALSE;
  }

  delete guardedwm;
  emit ProcMessage(serverID(), ProcCommand::deleteTopLevel, name);
}

void KSircProcess::clean_toplevel(QObject *clean){
  if(!clean){
    qWarning("Passed null to cleaner!!");
    return;
  }
  bool cont = FALSE;
  do{
    cont = FALSE;
    QDictIterator<KSircMessageReceiver> it(TopList);
    while(it.current() != 0x0){
      if((QObject *)it.current() == clean){
        QString key = it.currentKey();
        while(TopList[key] != 0x0){
          TopList.remove(key);
        }
        cont = TRUE;
        break;
      }
      ++it;
    }
  } while(cont == TRUE);
}

void KSircProcess::request_quit( const QCString& command )
{
    iocontrol->stdin_write( command );
    // Since removing the toplevels will delete the one that emitted this
    // signal as well, we need to defer this a little (malte)
    QTimer::singleShot( 0, this, SLOT( do_quit() ) );
}

void KSircProcess::do_quit()
{
    for ( QDictIterator< KSircMessageReceiver > it( TopList ); it.current(); ++it )
    {
        if ( it.currentKey() == "!default" ) continue;
        if ( KSircTopLevel* topLevel = dynamic_cast< KSircTopLevel* >( it.current() ) )
        {
            QGuardedPtr< KSircTopLevel > guardedTL = topLevel;
            displayMgr->removeTopLevel( topLevel );
            delete guardedTL;
        }
        else delete it.current();
    }
    // cleanup() would otherwise delete them a second time
    TopList.clear();
    delete this;
}

void KSircProcess::default_window(KSircTopLevel *w)
{

  //
  // If we want to track the default as it goes around, change the
  // window on focus changes.
  //

  if(w && (default_follow_focus == TRUE))
    TopList.replace("!default", w);

}

void KSircProcess::recvChangeChannel(const QString &old_chan, const QString &new_chan)
{
  //
  // Channel changed name, add our own name and off we go.
  // ServerController needs our name so it can have a uniq handle for
  // the window name.
  //

    if(TopList[old_chan]) {
        kdDebug(5008) << "In change channel, found it" << endl;
        TopList.insert(new_chan, TopList.take(old_chan));
    }
    else {
        kdDebug(5008) << "In change channel, didn;t find it" << endl;
    }
  emit ProcMessage(serverID(), ProcCommand::changeChannel,
		   old_chan + " " + new_chan);
}

void KSircProcess::filters_update()
{
  QString command, next_part, key, data;
  command = "/crule\n";
  iocontrol->stdin_write(command.ascii());
  QDictIterator<KSircMessageReceiver> it(TopList);
  KSircMessageReceiver *cur, *br;
  filterRuleList *frl;
  filterRule *fr;
  cur = TopList["!base_rules"];
  br = cur;
  while(cur){
    frl = cur->defaultRules();
    for ( fr=frl->first(); fr != 0; fr=frl->next() ){
      command.truncate(0);
      command += "/ksircappendrule DESC==";
      command += fr->desc;
      command += " !!! SEARCH==";
      command += fr->search;
      command += " !!! FROM==";
      command += fr->from;
      command += " !!! TO==\"";
      command += fr->to;
      command += "\"\n";
      iocontrol->stdin_write(command.local8Bit());
    }
    delete frl;
    ++it;
    cur = it.current();
    if(cur == br){
      ++it;
      cur = it.current();
    }
  }
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  int max = kConfig->readNumEntry("Rules", 0);
  for(int number = 1; number <= max; number++){
    command.truncate(0);
    key.sprintf("name-%d", number);
    next_part.sprintf("/ksircappendrule DESC==%s !!! ", kConfig->readEntry(key).ascii());
    command += next_part;
    key.sprintf("search-%d", number);
    next_part.sprintf("SEARCH==%s !!! ", kConfig->readEntry(key).ascii());
    command += next_part;
    key.sprintf("from-%d", number);
    next_part.sprintf("FROM==%s !!! ", kConfig->readEntry(key).ascii());
    command += next_part;
    key.sprintf("to-%d", number);
    next_part.sprintf("TO==\"%s\"\n", kConfig->readEntry(key).ascii());
    command += next_part;
    iocontrol->stdin_write(command.ascii());
  }
}


void KSircProcess::notify_forw_online(QString nick)
{
  emit ProcMessage(serverID(), ProcCommand::nickOnline, nick);
}

void KSircProcess::notify_forw_offline(QString nick)
{
  emit ProcMessage(serverID(), ProcCommand::nickOffline, nick);
}

void KSircProcess::ServMessage(QString dst_server, int command, QString args)
{
  if(dst_server.isEmpty() || (dst_server == serverID())){
    switch(command){
    case ServCommand::updateFilters:
      filters_update();
      break;
    default:
      kdDebug(5008) << "Unkown command: " << command << " to " << command << " args " << args << endl;
      break;
    }
  }
}

void KSircProcess::turn_on_autocreate()
{
    emit ProcMessage(serverID(), ProcCommand::turnOnAutoCreate, QString());
    auto_create_really = FALSE;
}

void KSircProcess::setNick(const QString nick)
{
    QString new_nick = nick;
    while (!new_nick.isEmpty() &&
          (new_nick[0].latin1() == '@' || new_nick[0].latin1() == '*'))
	new_nick.remove(0, 1);
    if(new_nick != m_nick){
	m_nick = new_nick;
	/*
	 * redo the filter rules since they use
	 * our nick
	 */
        kdDebug(5008) << "Redo filters" << endl;
	filters_update();
    }

}

const QString KSircProcess::getNick() const
{
    return m_nick;
}

#include "ksircprocess.moc"

// vim: ts=4 sw=4 et
