/**********************************************************************

 TopLevel IRC Channel/query Window

 $$Id: toplevel.cpp 437911 2005-07-23 14:56:02Z binner $$

 This is the main window with which the user interacts.  It handles
 both normal channel conversations and private conversations.

 2 classes are defined, the UserControlMenu and KSircToplevel.  The
 user control menu is used as a list of user defineable menus used by
 KSircToplevel.

 KSircTopLevel:

 Signals:

 outputLine(QString &):
 output_toplevel(QString):

 closing(KSircTopLevel *, QString channel):

 changeChannel(QString old, QString new):

 currentWindow(KSircTopLevel *):

 Slots:



 *********************************************************************/

#include "toplevel.h"
#include "alistbox.h"
#include "chanparser.h"
#include "ksopts.h"
#include "control_message.h"
#include "displayMgr.h"
#include "NewWindowDialog.h"
#include "usercontrolmenu.h"
#include "topic.h"
#include "charSelector.h"
#include "ksview.h"
#include "logfile.h"
#include "servercontroller.h"
#include "ioDCC.h"

#include "KSTicker/ksticker.h"

#include <stdlib.h>

#include <qaccel.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qtextcodec.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kwin.h>
#include <knotifyclient.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kcharsets.h>
#include <kglobalsettings.h>
#include <kstdguiitem.h>

extern DisplayMgr *displayMgr;
//QPopupMenu *KSircTopLevel::user_controls = 0L;
QPtrList<UserControlMenu> *KSircTopLevel::user_menu = 0L;
QStringList KSircTopLevel::cmd_menu;

void
KSircTopLevel::initColors()
{
  QColorGroup cg_mainw = kapp->palette().active();
  cg_mainw.setColor(QColorGroup::Base, ksopts->backgroundColor);
  cg_mainw.setColor(QColorGroup::Text, ksopts->textColor);
  cg_mainw.setColor(QColorGroup::Link, ksopts->linkColor);
  cg_mainw.setColor(QColorGroup::Highlight, ksopts->selBackgroundColor);
  cg_mainw.setColor(QColorGroup::HighlightedText, ksopts->selForegroundColor);
  mainw->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  nicks->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  linee->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  lag->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  cg_mainw.setColor(QColorGroup::Background, ksopts->backgroundColor);
  cg_mainw.setColor(QColorGroup::Foreground, ksopts->textColor);
  ksTopic->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  selector->setFont( ksopts->defaultFont.family() );
  mainw->setFont( ksopts->defaultFont );
  nicks->setFont( ksopts->defaultFont );
  linee->setFont( ksopts->defaultFont );
  ksTopic->setFont( ksopts->defaultFont );

}

KSircTopLevel::KSircTopLevel(KSircProcess *_proc, const KSircChannel &channelInfo, const char * name)
    : KMainWindow(0, name, 0/*no WDestructiveClose !*/),
      UnicodeMessageReceiver(_proc),
      lastBeep( QTime::currentTime() ),
      m_channelInfo(channelInfo)

{
    // prevent us from being quitted when closing a channel-window. Only
    // closing the servercontroller shall quit.
    // KMainWindow will deref() us in closeEvent
    kapp->ref();


  m_topic = QString::null;

  QCString kstl_name = QCString(QObject::name()) + "_" + "toplevel";
  setName(kstl_name);

  if(!m_channelInfo.channel().isEmpty()) {
    setCaption(m_channelInfo.channel());
    caption = m_channelInfo.channel();
  }
  else
  {
    caption = QString::null;
  }

  Buffer = FALSE;

  have_focus = 0;
  tab_pressed = -1; // Tab (nick completion not pressed yet)
  tab_start = -1;
  tab_end = -1;

  m_gotMsgWithoutFocus = false;

  KickWinOpen = false;
  current_size = size();

  ksopts->channelSetup(ksircProcess()->serverName(), m_channelInfo.channel());
  m_channelInfo.setEncoding(ksopts->chan(m_channelInfo).encoding);

  selector = new charSelector();
  connect(selector, SIGNAL(clicked()), this, SLOT(insertText()));
  selector->setFont(ksopts->defaultFont.family());

  file = new QPopupMenu(this, QCString(QObject::name()) + "_popup_file");
  file->setCheckable(true);

  KAction *act = KStdAction::openNew( this, SLOT( newWindow() ), actionCollection() );
  act->plug( file );
  file->insertItem(i18n("New Ser&ver..."), servercontroller::self(), SLOT(new_connection()), Key_F2);
  file->insertSeparator();
  file->insertItem(i18n("&DCC Manager..."), this, SLOT(showDCCMgr()));
  file->insertItem(i18n("&Save to Logfile..."), this, SLOT(saveCurrLog()), CTRL + Key_S);

  tsitem = file->insertItem(i18n("Time St&amp"), this, SLOT(toggleTimestamp()), CTRL + Key_T);
  file->setItemChecked(tsitem, ksopts->chan(m_channelInfo).timeStamp);

  fjpitem = file->insertItem(i18n("Hide Join/Part Messages"), this, SLOT(toggleFilterJoinPart()));
  file->setItemChecked(fjpitem, ksopts->chan(m_channelInfo).filterJoinPart);

  file->insertItem(i18n("Character &Table"), selector, SLOT(show()), CTRL + Key_H);
  beepitem = file->insertItem(i18n("N&otify on Change"), this, SLOT(toggleBeep()), CTRL + Key_P);
  file->setItemChecked(beepitem, ksopts->chan(m_channelInfo).beepOnMsg);

  encodingAction = new KSelectAction( i18n( "&Encoding" ), 0, this );
  connect( encodingAction, SIGNAL( activated() ), this, SLOT( setEncoding() ) );
  QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();

  topicitem = file->insertItem(i18n("S&how Topic"), this, SLOT(toggleTopic()), CTRL + Key_O);
  if (isPrivateChat() || m_channelInfo.channel().startsWith("!no_channel")) {
      file->setItemEnabled(topicitem, false);
  }
  else {
      file->setItemChecked(topicitem, ksopts->chan(m_channelInfo).topicShow);
  }

  tickeritem = file->insertItem(i18n("Ticker &Mode"), this, SLOT(toggleTicker()));

  // remove utf16/ucs2 as it just doesn't work for IRC
  QStringList::Iterator encodingIt = encodings.begin();
  while ( encodingIt != encodings.end() ) {
    if ( ( *encodingIt ).find( "utf16" ) != -1 ||
         ( *encodingIt ).find( "iso-10646" ) != -1 )
      encodingIt = encodings.remove( encodingIt );
    else
      ++encodingIt;
  }
  encodings.prepend( i18n( "Default" ) );

  encodingAction->setItems( encodings );
  encodingAction->plug( file );

  int eindex = encodings.findIndex(ksopts->chan(m_channelInfo).encoding);
  if(eindex == -1)
      encodingAction->setCurrentItem( 0 );
  else
      encodingAction->setCurrentItem(eindex);
  setEncoding();

  file->insertSeparator();
  act = KStdAction::close( this, SLOT( terminate() ), actionCollection() );
  act->plug( file );

  kmenu = menuBar();
  kmenu->insertItem(i18n("&Channel"), file, KST_CHANNEL_ID, -1);
  kmenu->setAccel(Key_F, KST_CHANNEL_ID);

  /*
   * Ok, let's look at the basic widget "layout"
   * Everything belongs to q QFrame F, this is use so we
   * can give the KApplication a single main client widget, which is needs.
   *
   * A QVbox and a QHbox is used to ctronl the 3 sub widget
   * The Modified QListBox is then added side by side with the User list box.
   * The SLE is then fit bello.
   */

  // kstInside does not setup fonts, etc, it simply handles sizing

  top = new QVBox( this );

  ksTopic = new KSircTopic( top );
  ksTopic->setFont(ksopts->defaultFont);
  connect( ksTopic, SIGNAL( topicChange( const QString & ) ),
           this, SLOT( setTopicIntern( const QString & ) ) );

  QCString kstn = QCString(QObject::name()) + "_";

  pan = new QSplitter(QSplitter::Horizontal, top, kstn + "splitter");
#if KDE_IS_VERSION(3,1,92)
  pan->setOpaqueResize( KGlobalSettings::opaqueResize() );
#else
  pan->setOpaqueResize( true );
#endif

  mainw = new KSircView(ksircProcess(), pan, kstn + "KSircView" );
  mainw->setFocusPolicy(QWidget::NoFocus);

  nicks_box = new QVBox(pan);

  channelButtons = new chanButtons(ksircProcess(), nicks_box);
  connect(channelButtons, SIGNAL(mode(QString, int, QString)),
           this, SLOT(setMode(QString, int, QString)));

  nicks = new aListBox(nicks_box, kstn + "aListBox");
  nicks->setFocusPolicy(QWidget::NoFocus);
  //nicks->hide(); // default = only the main widget

  lag = new QLabel(nicks_box);
  lag->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  lag->setAlignment(Qt::AlignCenter | Qt::SingleLine);
  lag->setText(i18n("Lag: Wait"));


  QValueList<int> sizes;
  sizes << int(width()*0.85) << int(width()*0.15);
  pan->setSizes(sizes);
  pan->setResizeMode( mainw, QSplitter::Stretch );
  pan->setResizeMode( nicks_box, QSplitter::Stretch );

//  f = new kstInside(top, QString(QObject::name()) + "_" + "kstIFrame");
  top->setStretchFactor(pan, 1);

  setCentralWidget(top);  // Tell the KApplication what the main widget is.

  logFile = 0;
  if ( ksopts->chan(m_channelInfo).logging && (m_channelInfo.channel() != "!no_channel" ))
  {
      logFile = new LogFile( m_channelInfo.channel(), m_channelInfo.server() );
      logFile->open();
  }

  // get basic variable

//  mainw = f->mainw;
//  nicks = f->nicks;
  //  pan = f->pan;

  clearWindow();

  if(isPrivateChat()){
      KConfig conf("ksirc/winlog/" + channelInfo.server() + "-" + channelInfo.channel(),
		   false, false, "data");
      QString group = "Message-History";

      if(conf.hasGroup( group )){
	  conf.setGroup( group );

	  QStringList strlist = conf.readListEntry("History");
	  if(strlist.count() > 0){
	      mainw->enableTimeStamps(true);
	      for ( QStringList::Iterator it = strlist.begin();
		    it != strlist.end();
		    ++it ) {
		  mainw->addRichText(*it);
	      }
	  }
	  strlist.clear();
      }
  }

  mainw->enableTimeStamps(ksopts->chan(m_channelInfo).timeStamp);

  edit = new QPopupMenu(this);
  act = KStdAction::copy( mainw, SLOT( copy() ), actionCollection() );
  act->plug( edit );
  act = KStdAction::paste( this, SLOT( pasteToWindow() ), actionCollection() );
  act->plug( edit );
  edit->insertItem(i18n("C&lear Window"), this, SLOT(clearWindow()), CTRL + Key_L);
  kmenu->insertItem(i18n("&Edit"), edit, -1, -1);

  linee = new aHistLineEdit(top, "");

  initColors();

  //  ksb_main->addWidget(linee, mainw->width());
//  ksb_main->insertItem(, KSB_MAIN_LAG, true);

  // don't show the nick lists in a private chat or the default window
  if (isPrivateChat() || m_channelInfo.channel().startsWith("!no_channel"))
  {
      nicks_box->hide();
      ksTopic->hide();
      //    nicks->hide();
      //    lag->hide();
      //    channelButtons->hide();
  }
  else
  {
      nicks_box->show();

      if(file->isItemChecked(topicitem)){
          ksTopic->show();
//          channelButtons->show();
      }
      else{
          ksTopic->hide();
          //          channelButtons->hide();
      }
      // channelButtons->show();
      // lag->show();
      // nicks->show();
  }


  connect(mainw, SIGNAL(pasteReq( const QString& )),
          this, SLOT( slotTextDropped( const QString& )));

  nicks->setFont(ksopts->defaultFont);

  // setup line editor

  linee->setFocusPolicy(QWidget::StrongFocus);
  linee->setFont(ksopts->defaultFont);

  if(ksopts->oneLineEntry == true) {
      linee->setWordWrap(QTextEdit::NoWrap);
  }
  else {
      linee->setWordWrap(QTextEdit::WidgetWidth);
  }

  connect(linee, SIGNAL(gotFocus()),
          this, SLOT(gotFocus()));
  connect(linee, SIGNAL(lostFocus()),
          this, SLOT(lostFocus()));
  connect(linee, SIGNAL(pasteText(const QString&)),
          this, SLOT(slotTextDropped(const QString&)));
  connect(linee, SIGNAL(notTab()),
          this, SLOT(lineeNotTab()));

  connect( linee, SIGNAL( gotReturnPressed() ),
           this, SLOT( returnPressed() ) );

  linee->setFocus();  // Give SLE focus
  linee->slotMaybeResize();

  lines = 0;          // Set internal line counter to 0

  ticker = 0x0;

  /*
   * Set generic run time variables
   *
   */

  opami = FALSE;
  continued_line = FALSE;
//  on_root = FALSE;

  /*
   * Load basic pics and images
   * This should use on of the KDE finder commands
   */

  KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());

  /*
   * Create our basic parser object
   */

  ChanParser = new ChannelParser(this);


  /*
   * Create the user Controls popup menu, and connect it with the
   * nicks list on the lefthand side (highlighted()).
   *
   */

  if(user_menu == 0)
    user_menu = UserControlMenu::parseKConfig();

  user_controls = new QPopupMenu(this);
  kmenu->insertItem(i18n("&Users"), user_controls);

  command = new QPopupMenu(this);

  setupCommandMenu();

  int i = 0;
  QDict<QPopupMenu> cml;
  for(QStringList::Iterator it = cmd_menu.begin();
      it != cmd_menu.end();
      ++it){
      QString top, item;
      top = (*it).section('/', 0, 0);
      item = (*it).section('/', 1, 1);
      if(!cml[top]) {
          cml.insert(top, new QPopupMenu(this));
          command->insertItem(top, cml[top]);
      }
      cml[top]->insertItem(item, this, SLOT(cmd_process(int)), 0, i);

      i++;
  }

  kmenu->insertItem(i18n("C&ommand"), command);

  kmenu->insertItem( KStdGuiItem::help().text(), helpMenu( QString::null, false ));

  connect(user_controls, SIGNAL(activated(int)),
          this, SLOT(UserParseMenu(int)));

  connect(nicks, SIGNAL(contextMenuRequested(int)), this,
          SLOT(UserSelected(int)));
  connect(nicks, SIGNAL(selectedNick(const QString &)),
          this, SLOT(openQueryFromNick(const QString &)));
  connect(nicks, SIGNAL(mouseButtonClicked ( int, QListBoxItem *, const QPoint &)),
          this, SLOT(pasteToNickList(int, QListBoxItem *, const QPoint &)));
  connect(nicks, SIGNAL(textDropped( const QListBoxItem *, const QString& )),
          this, SLOT(dndTextToNickList(const QListBoxItem *, const QString&)));

  UserUpdateMenu();  // Must call to update Popup.

  accel = new QAccel(this, "accel");

  accel->connectItem(accel->insertItem(SHIFT + Key_PageUp),
                     this,
                     SLOT(AccelScrollUpPage()));
  accel->connectItem(accel->insertItem(SHIFT + Key_PageDown),
                     this,
                     SLOT(AccelScrollDownPage()));

  /*
   * Pageup/dn
   * Added for stupid wheel mice
   */

  accel->connectItem(accel->insertItem(Key_PageUp),
                     this,
                     SLOT(AccelScrollUpPage()));
  accel->connectItem(accel->insertItem(Key_PageDown),
                     this,
                     SLOT(AccelScrollDownPage()));

  /*
   * These are not presently implemented, so let's not fill the logs.
  accel->connectItem(accel->insertItem(CTRL + Key_Return),
                     this,
                     SLOT(AccelPriorMsgNick()));

  accel->connectItem(accel->insertItem(CTRL + SHIFT + Key_Return),
                     this,
                     SLOT(AccelNextMsgNick()));
  */

  accel->connectItem(accel->insertItem(Key_Tab), // adds TAB accelerator
                     this,                         // connected to the main
                     SLOT(TabNickCompletionNormal()));  // TabNickCompletion() slot
  accel->connectItem(accel->insertItem(SHIFT+Key_Tab), // adds TAB accelerator
                     this,                         // connected to the main
                     SLOT(TabNickCompletionShift()));  // TabNickCompletion() slot
  accel->connectItem(accel->insertItem(CTRL + Key_N),
                     this, SLOT(newWindow()));
//  accel->connectItem(accel->insertItem(CTRL + Key_S),
//                     this, SLOT(toggleTimestamp()));

  // Drag & Drop
  connect( mainw, SIGNAL( textDropped(const QString&) ),
           SLOT( slotTextDropped(const QString&) ));
  connect( mainw, SIGNAL( urlsDropped(const QStringList&) ),
           SLOT( slotDropURLs(const QStringList&) ));
  connect( nicks, SIGNAL( urlsDropped( const QStringList&, const QString& )),
           SLOT( slotDccURLs( const QStringList&, const QString& )));
  connect( this, SIGNAL( changed(bool, QString) ), this, SLOT( doChange(bool, QString) ));

  mainw->setAcceptFiles( isPrivateChat() );
  resize(600, 360);

}

KSircTopLevel::~KSircTopLevel()
{
    // Cleanup and shutdown
    //  if(this == ksircProcess()->getWindowList()["default"])
    //    write(sirc_stdin, "/quit\n", 7); // tell dsirc to close
    //
    //  if(ksircProcess()->getWindowList()[m_channelInfo.channel()])
    //    ksircProcess()->getWindowList().remove(m_channelInfo.channel());

    //  if((m_channelInfo.channel()[0] == '#') || (m_channelInfo.channel()[0] == '&')){
    //    QString str = QString("/part ") + m_channelInfo.channel() + "\n";
    //    emit outputLine(str);
    //  }

    kdDebug(5008) << "~KSircTopLevel in" << endl;
    if (  ksopts->autoSaveHistory )
    {
        if ( isPublicChat() ) {
            kdDebug(5008) << "*** parting channel: " << m_channelInfo.channel() << endl;
            QString str = QString("/part ") + m_channelInfo.channel() + "\n";
            emit outputUnicodeLine(str);
        }
        else {
            QStringList strlist;

            mainw->addLine("user|X", ksopts->channelColor, " Saved log of previous messages");

            mainw->enableTimeStamps(true);

            for(KSirc::TextParagIterator ksit = mainw->firstParag();
                ksit.atEnd() == 0;
                ++ksit) {
                QString rt = ksit.richText();
                if(rt.contains("<img src=\"user|servinfo\">"))
                    continue;

                kdDebug(5008) << rt << endl;

                strlist += rt;

            }

            KConfig conf("ksirc/winlog/" + channelInfo().server() +
                         "-" + channelInfo().channel(),
                         false, false, "data");
            QString group = "Message-History";

            conf.setGroup( group );

            conf.writeEntry("History", strlist);
            conf.sync();

        }
    }
    delete ticker;
    delete user_controls;
    delete ChanParser;
    delete selector;
    delete channelButtons;
    delete logFile;
    kdDebug(5008) << "~KSircToplevel out" << endl;
}

void KSircTopLevel::setMode(QString mode, int mode_type, QString currentNick)
{
  QString command;
  if (mode_type == 0)
    command = QString::fromLatin1("/mode %1 %2\n").arg(m_channelInfo.channel()).arg(mode);
  else
    command = QString::fromLatin1("/mode %1 %2\n").arg(currentNick).arg(mode);
  sirc_write(command);
  linee->setFocus();
}

void KSircTopLevel::setEncoding()
{
  int index = encodingAction->currentItem();
  if ( index == 0 ) {// default (locale) encoding
      ksopts->chan(m_channelInfo).encoding = QString::null;
      UnicodeMessageReceiver::setEncoding( QString::null );
  }
  else {
      ksopts->chan(m_channelInfo).encoding = encodingAction->currentText();
      UnicodeMessageReceiver::setEncoding( KGlobal::charsets()->encodingForName( encodingAction->currentText() ) );
  }
  ksopts->save(KSOptions::Channels);
}

void KSircTopLevel::setupCommandMenu()
{
    if(cmd_menu.empty() == true){
        cmd_menu.append(i18n("Help") + "/" + "help");
        cmd_menu.append(i18n("Client") + "/" + "alias");
        cmd_menu.append(i18n("User") + "/" + "away");
        cmd_menu.append(i18n("Client") + "/" + "bye");
        cmd_menu.append(i18n("Client") + "/" + "cd");
        cmd_menu.append(i18n("Basic") + "/" + "ctcp");
        cmd_menu.append(i18n("Basic") + "/" + "dcc");
        cmd_menu.append(i18n("Operator") + "/" + "deop");
        cmd_menu.append(i18n("User") + "/" + "describe");
        cmd_menu.append(i18n("Client") + "/" + "eval");
        cmd_menu.append(i18n("Client") + "/" + "exit");
        cmd_menu.append(i18n("Client") + "/" + "hop");
        cmd_menu.append(i18n("Basic") + "/" + "ignore");
        cmd_menu.append(i18n("Server") + "/" + "info");
        cmd_menu.append(i18n("Channel") + "/" + "invite" + "/" + "*chan*");
        cmd_menu.append(i18n("Basic") + "/" + "join");
        cmd_menu.append(i18n("Operator") + "/" + "kick");
        cmd_menu.append(i18n("Oper") + "/" + "kill");
        cmd_menu.append(i18n("Basic") + "/" + "leave");
        cmd_menu.append(i18n("Server") + "/" + "links");
        cmd_menu.append(i18n("Basic") + "/" + "list");
        cmd_menu.append(i18n("Basic") + "/" + "ll");
        cmd_menu.append(i18n("Client") + "/" + "load");
        cmd_menu.append(i18n("Server") + "/" + "lusers");
        cmd_menu.append(i18n("Server") + "/" + "map");
        cmd_menu.append(i18n("Basic") + "/" + "me");
        cmd_menu.append(i18n("Channel") + "/" + "mode" + "/" + "*chan*");
        cmd_menu.append(i18n("Server") + "/" + "motd");
        cmd_menu.append(i18n("Basic") + "/" + "msg");
        cmd_menu.append(i18n("Basic") + "/" + "nick");
        cmd_menu.append(i18n("Basic") + "/" + "notice");
        cmd_menu.append(i18n("Basic") + "/" + "notify");
        cmd_menu.append(i18n("Operator") + "/" + "op");
        cmd_menu.append(i18n("Oper") + "/" + "oper");
        cmd_menu.append(i18n("Basic") + "/" + "query");
        cmd_menu.append(i18n("Channel") + "/" + "part"+ "/" + "*chan*");
        cmd_menu.append(i18n("Basic") + "/" + "ping");
        cmd_menu.append(i18n("Client") + "/" + "quit");
        cmd_menu.append(i18n("Server") + "/" + "quote");
        cmd_menu.append(i18n("Oper") + "/" + "rping");
        cmd_menu.append(i18n("Oper") + "/" + "rehash");
        cmd_menu.append(i18n("Basic") + "/" + "say");
        cmd_menu.append(i18n("Server") + "/" + "stats");
        cmd_menu.append(i18n("Client") + "/" + "system");
        cmd_menu.append(i18n("Server") + "/" + "time");
        cmd_menu.append(i18n("Channel") + "/" + "topic" + "/" + "*chan*");
        cmd_menu.append(i18n("Client") + "/" + "version");
        cmd_menu.append(i18n("Oper") + "/" + "wallops");
        cmd_menu.append(i18n("Channel") + "/" + "who" + "/" + "*chan*");
        cmd_menu.append(i18n("Basic") + "/" + "whois");
        cmd_menu.append(i18n("Basic") + "/" + "whowas");
        cmd_menu.append(i18n("Basic") + "/" + "wi");
        cmd_menu.append(i18n("Help") + "/" + "newuser");
        cmd_menu.append(i18n("Channel") + "/" + "ban");
        cmd_menu.append(i18n("Channel") + "/" + "unban");
        cmd_menu.append(i18n("Channel") + "/" + "clrban" + "/" + "*chan*");
        cmd_menu.append(i18n("Channel") + "/" + "banlist" + "/" + "*chan*");
        cmd_menu.append(i18n("Basic") + "/" + "pig");
        cmd_menu.append(i18n("Channel") + "/" + "wallop");
        cmd_menu.append(i18n("Client") + "/" + "exec");
        cmd_menu.append(i18n("Basic") + "/" + "url");
        cmd_menu.sort();
    }
}

void KSircTopLevel::insertText()
{
  linee->insert(selector->currentText());
}

void KSircTopLevel::show()
{
    KMainWindow::show();
    linee->setFocus();
    mainw->scrollToBottom(true);
}

enum {
    KSTUp = 1,
    KSTDown = 2
};

void KSircTopLevel::TabNickCompletionShift()
{
    TabNickCompletion(KSTDown);
}

void KSircTopLevel::TabNickCompletionNormal()
{
    TabNickCompletion(KSTUp);
}

void KSircTopLevel::TabNickCompletion(int dir)
{
  /*
   * Gets current text from lined find the last item and try and perform
   * a nick completion, then return and reset the line.
   */

  int start, end;
  int extra = 0;
  bool first = false;
  QString s;

  if(tab_pressed == -1){
    s = linee->text();
    tab_saved = s;
    end = linee->cursorPosition() - 1;
    start = s.findRev(" ", end, FALSE);
    tab_start = start;
    tab_end = end;
    first = true;
  }
  else{
    s = tab_saved;
    start = tab_start;
    end = tab_end;
  }

  if(dir == KSTDown){
      if(tab_pressed > 0)
          tab_pressed -= 1;
  }
  else {
      tab_pressed += 1;
  }


  if(s.length() == 0){
      if(tab_nick.isEmpty()) {
	  KNotifyClient::beep();
	  lineeNotTab();
	  return;
      }
      QString line = tab_nick + ": "; // tab_nick holds the last night since we haven't overritten it yet.
      linee->setText(line);
      linee->setCursorPosition(line.length());
      connect(linee, SIGNAL(notTab()),
	      this, SLOT(lineeNotTab()));
      return;
  }

  if (start == -1) {
      tab_nick = findNick(s.mid(0, end+1), tab_pressed);

      if(first && (tab_nick.isNull() == TRUE)){
	  KNotifyClient::beep();
	  lineeNotTab();
          return;
      }
      else if(tab_nick.isNull() == TRUE){
          tab_pressed -= 1;
          tab_nick = findNick(s.mid(0, end+1), tab_pressed);
      }
      s.replace(0, end + 1, "");
      if(s[0] == ':')
          s.replace(0, 2, "");
      s.prepend(tab_nick + ": ");
      extra = 2;
  }
  else {
      tab_nick = findNick(s.mid(start + 1, end - start), tab_pressed);
      if(first && (tab_nick.isNull() == TRUE)){
	  KNotifyClient::beep();
	  lineeNotTab();
	  return;
      }
      else if(tab_nick.isNull() == TRUE){
      tab_pressed -= 1;
      tab_nick = findNick(s.mid(start + 1, end - start), tab_pressed);
    }

    if((uint) end == s.length() - 1){ /* if we're at the end add a space */
      s.replace(start + 1, end - start, tab_nick + " ");
      extra = 1;
    }
    else {
      s.replace(start + 1, end - start, tab_nick);
      extra = 0;
    }
  }

  int tab = tab_pressed;

  linee->setText(s);
  linee->setCursorPosition(start + tab_nick.length() + 1 + extra);

  tab_pressed = tab; // setText causes lineeTextChanged to get called and erase tab_pressed

  connect(linee, SIGNAL(notTab()),
          this, SLOT(lineeNotTab()));

}

void KSircTopLevel::sirc_receive(QString str, bool broadcast)
{

  /*
   * read and parse output from dsirc.
   * call reader, split the read input into lines, parse the lines
   * then print line by line into the main text area.
   *
   * PROBLEMS: if a line terminates in mid line, it will get borken oddly
   *
   */

  if(!Buffer){
    if( !str.isEmpty() ){
        LineBuffer.append( BufferedLine( str, broadcast ) );
    }

    bool addressed = false;
    BufferedLine line;

    // be careful not to use a QValueList iterator here, as it is possible
    // that we enter a local event loop (think of the ssfeprompt dialog!)
    // which might trigger a socketnotifier activation which in turn
    // might result in the execution of code that modifies the LineBuffer,
    // which would invalidate iterators (Simon)
    while ( LineBuffer.begin() != LineBuffer.end() )
    {
        QString plainText("");
        line = *LineBuffer.begin();
        LineBuffer.remove( LineBuffer.begin() );

        // Get the need list box item, with colour, etc all set
        if (parse_input(line.message, plainText))
        {
            // If we should add anything, add it.
            // Don't announce server messages as they are
            // spread through all channels anyway

	    bool addressedLine = false;

	    if(line.message.contains( ">~o")){ /* highlighted with our nick */
		addressedLine = true;
	    }

	    // detect /msg's -- needed when auto-create-window is off
	    if ( line.message.find( QRegExp( "^\\[~b.+~b\\].+$" ) ) == 0 )
		addressedLine = true;

	    if (  addressedLine == true && line.message.startsWith("* " + ksircProcess()->getNick()))
		addressedLine = false;

            if ( addressedLine )
		addressed = true;

            if ( !line.wasBroadcast ) {
                // This line is addressed to me if it contains my nick, or is in a private chat
                emit changed( addressedLine || isPrivateChat(), plainText );
            }
        }
    }
    LineBuffer.clear(); // Clear it since it's been added
  }
  else{
    LineBuffer.append( BufferedLine( str, broadcast ) );
  }

  QValueList<int> list;
  QValueList<int>::iterator it;
  QString values;
  list = pan->sizes();
  it = list.begin();
  while( it != list.end()){
      values += QString("%1 ").arg(*it);
      ++it;
  }
}
void KSircTopLevel::sirc_line_return(const QString &text)
{

  /* Take line from SLE, and output if to dsirc */

  QString s = text;

  if(s.length() == 0)
    return;

  tab_pressed = -1; // new line, zero the counter.
  s += '\n'; // Append a need carriage return :)

  sirc_write(s);

  linee->setText("");
  linee->slotMaybeResize();

}

void KSircTopLevel::sirc_write(const QString &str)
{
  /*
   * Parse line forcommand we handle
   */
  QString command = str, plain = str.lower().simplifyWhiteSpace();
  if(plain.startsWith("/join ") ||
     plain.startsWith("/j ") ||
     plain.startsWith("/query ")) {

      QString s = plain.mid(plain.find(' ')+1);

      QStringList channels = QStringList::split(",", s);
      QStringList::ConstIterator it = channels.begin();

      for( ; it != channels.end(); ++it){
          QString name = *it;
	  //kdDebug(5008) << "Doing " << name << endl;
	  QRegExp rx("(\\S+)\\s*(\\S*)");
	  rx.search(name);
	  KSircChannel ci(m_channelInfo.server(),
			  rx.cap(1),   // channel
			  rx.cap(2));  // optional key
	  linee->setText(QString::null);
	  emit open_toplevel(ci);
/*          if(name[0] != '#'){
              emit open_toplevel(name);
              linee->setText(QString::null);
          }
          else {
              emit outputUnicodeLine(plain + "\n");
              emit open_toplevel(encoder()->fromUnicode(name));
	      }
*/
      }
      // Finish sending /join
      return;
  }
  else if(plain.startsWith("/server ")) {
    plain.simplifyWhiteSpace();
    QRegExp rx("/server (\\S+) *(\\S*) *(\\S*)");
    if(rx.search(str) >= 0){
      QString server = rx.cap(1);
      QString port = rx.cap(2);
      QString pass = rx.cap(3);

      bool ssl = false;
      if(server.startsWith("+")){
	server.replace(0, 1, "");
        ssl = true;
      }

      KSircServer serv(server, port, QString::null, pass, ssl);
      servercontroller::self()->new_ksircprocess(serv);

      return;
    }
  }
  else if(plain.startsWith("/part") ||
          plain.startsWith("/leave") ||
          plain.startsWith("/hop")) {
    QApplication::postEvent(this, new QCloseEvent()); // WE'RE DEAD
    linee->setText(QString::null);
    return;
  }
  else if( plain.startsWith( "/bye" ) ||
           plain.startsWith( "/exit" ) ||
           plain.startsWith( "/quit" )) {
    linee->setText( QString::null );
    emit requestQuit( command.ascii() );
    return;
  }
  else if ( plain.startsWith( "/away" ) ) {
    QString awayEvalCommand = QString::fromLatin1( "/eval $publicAway = %1\n" ).arg( ksopts->publicAway ? '1' : '0' );
    emit outputUnicodeLine( awayEvalCommand );
  }

  //
  // Look at the command, if we're assigned a channel name, default
  // messages, etc to the right place.  This include /me, etc
  //

  if(!isSpecialWindow()) { // channel or private chat
    if(plain[0].latin1() != '/'){
      command.prepend(QString::fromLatin1("/msg %1 ").arg(m_channelInfo.channel()));
    }
    else if(plain.startsWith("/me ")) {
      command.remove(0, 3);
      command.prepend(QString("/de ") + m_channelInfo.channel());
    }
  }

  // Write out line

  emit outputUnicodeLine(command);
  /*
   * as opposed to what some might think
   * it doesn't matter when you call stb
   */
  mainw->scrollToBottom(true);
}

bool KSircTopLevel::parse_input(const QString &string, QString &plainText)
{
  /*
   * Parsing routines are broken into 3 majour sections
   *
   * 1. Search and replace evil characters. The string is searched for
   * each character in turn (evil[i]), and the character string in
   * evil_rep[i] is replaced.
   *
   * 2. Parse control messages, add pixmaps, and colourize required
   * lines.  Caption is also set as required.
   *
   * 3. Create and return the ircListItem.
   *
   */

  /*
   * No-output get's set to 1 if we should ignore the line
   */

  /*
   * This is the second generation parsing code.
   * Each line type is determined by the first 3 characters on it.
   * Each line type is then sent to a parsing function.
   */
  parseResult *pResult = ChanParser->parse(string);

  parseSucc *item = dynamic_cast<parseSucc *>(pResult);
  parseError *err = dynamic_cast<parseError *>(pResult);

  QString logString;

  if(item)
  {
    parseJoinPart *jp = dynamic_cast<parseJoinPart *>(pResult);
    if(jp &&
       ksopts->chan(m_channelInfo).filterJoinPart){
      delete pResult;
      return true;
    }
    else if (!item->string.isEmpty()) {
	logString = mainw->
	    addLine( item->pm, item->colour, item->string );
        if(ticker != NULL){
            ticker->mergeString(item->string, item->colour);
        }
    } else {
      delete pResult;
      return false;
    }
  }
  else if (err)
  {
    if(err->err.isEmpty() == FALSE)
    {
      kdWarning() << err->err << ": " << string << endl;
      delete pResult;
      return false;
    }
    if (!err->str.isEmpty()) {
	logString = mainw->addLine( "user|error", ksopts->errorColor, err->str );
    }

  }
  else
  {
	logString = mainw->addLine( QString::null, ksopts->textColor, string );
        if(ticker != NULL){
	    ticker->mergeString(string);
	}
	// If it contains our nick, move the speaker to the top
	// of the nick completion list
	if(string.contains("~o")){
	    QRegExp rx("<(\\S+)>");
	    if ( (rx.search(logString) >= 0) &&
		 (rx.cap(1) != ksircProcess()->getNick()) ) {
		//kdDebug(5008) << "addCompleteNick: " << rx.cap(1) << endl;
		addCompleteNick( rx.cap(1) );
	    }
        }

  }
  delete pResult;

  if ( !logString.isEmpty() && logFile )
      logFile->log( logString );

  if(!plainText.isNull())
      plainText = logString;

  return true;
}

void KSircTopLevel::returnPressed()
{

    QString s = linee->text();

    uint i;
    QChar c;
    for(i = 0; (c = s[i]); i++){
        switch(c.unicode()){
        case 0336:
            s[i] = 002;
            break;
        case 0327:
            s[i] = 037;
            break;
        case 0237:
            s[i] = 026;
            break;
        case 0252:
            s[i] = 003;
            break;
        }
    }

    while(s.length() > IRC_SAFE_MAX_LINE){
        int wrap = s.findRev(' ', IRC_SAFE_MAX_LINE);
        if(wrap == -1)
          wrap = IRC_SAFE_MAX_LINE;
        sirc_line_return(s.left(wrap));
        s = s.mid(wrap + 1);
    }
    if(!s.stripWhiteSpace().isEmpty())
	sirc_line_return(s);
    else {
	linee->setText("");
	linee->slotMaybeResize();
    }
}

void KSircTopLevel::UserSelected(int index)
{
  if(index >= 0)
    user_controls->popup(this->cursor().pos());
}

void KSircTopLevel::UserParseMenu(int id)
{
  if(nicks->currentItem() < 0)
      return;

  QString s;
  s = QString("/eval $dest_nick='%1';\n").arg(nicks->text(nicks->currentItem()));
  sirc_write(s);
  // set $$dest_chan variable
  s = QString("/eval $dest_chan='%1';\n").arg(m_channelInfo.channel());
  sirc_write(s);
  QString action = user_menu->at(id)->action;
  if (action.length() && action[0] == '/')
      action.remove(0, 1);
  s = QString("/eval &docommand(eval{\"%1\"});\n").arg(action);
  s.replace(QRegExp("\\$\\$"), "$");
  sirc_write(s);
}

void KSircTopLevel::UserUpdateMenu()
{
  int i = 0;
  UserControlMenu *ucm;

  user_controls->clear();
  for(ucm = user_menu->first(); ucm != 0; ucm = user_menu->next(), i++){
    if(ucm->type == UserControlMenu::Seperator){
      user_controls->insertSeparator();
    }
    else{
      user_controls->insertItem(ucm->title, i);
      if(ucm->accel)
        user_controls->setAccel(i, ucm->accel);
      if((ucm->op_only == TRUE) && (opami == FALSE))
        user_controls->setItemEnabled(i, FALSE);
    }
  }
}

void KSircTopLevel::AccelScrollDownPage()
{
   mainw->verticalScrollBar()->addPage();
}

void KSircTopLevel::AccelScrollUpPage()
{
   mainw->verticalScrollBar()->subtractPage();
}

void KSircTopLevel::newWindow()
{
  NewWindowDialog w(KSircChannel(m_channelInfo.server(), QString::null));
  connect(&w, SIGNAL(openTopLevel(const KSircChannel &)), SIGNAL(open_toplevel(const KSircChannel &)));
  w.exec();
}

void KSircTopLevel::closeEvent(QCloseEvent *e)
{
  KMainWindow::closeEvent( e );
  e->accept();

   //Let's not part the channel till we are acutally delete.
  // We should always get a close event, *BUT* we will always be deleted.
  //   if( isPublicChat() ) {
  //       QString str = QString("/part ") + m_channelInfo.channel() + "\n";
  //       emit outputLine(str);
  //   }

  // Hide ourselves until we finally die
  hide();
  qApp->flushX();
  // Let's say we're closing
  kdDebug(5008) << "Toplevel closing: " << m_channelInfo.channel() << endl;
  emit closing(this, m_channelInfo.channel());
}

void KSircTopLevel::focusChange(QWidget *w)
{
    QWidget *me = this;
    if(w == me){
        gotFocus();
    }
    else {
        lostFocus();
    }
}

void KSircTopLevel::gotFocus()
{
    if(isVisible() == TRUE){
        if(have_focus == 0){
	    if(m_channelInfo.channel()[0] != '!' ){
		QString str = QString("/join %1").arg(m_channelInfo.channel());
		if(m_channelInfo.key().length() > 0){
		    str.append(" " + m_channelInfo.key());
		}
		kdDebug(5008) << "Doing join: " << str << " / " << m_channelInfo.channel() << endl;
                str.append("\n");
                emit outputUnicodeLine(str);
            }
            have_focus = 1;
            emit currentWindow(this);
        }
    }
    if(m_gotMsgWithoutFocus == true){
        m_gotMsgWithoutFocus = false;
        servercontroller::self()->decreaseNotificationCount(QString("%1 -> %2 got message").arg(ksircProcess()->serverID()).arg(m_channelInfo.channel()));
    }

}

void KSircTopLevel::lostFocus()
{
  if(have_focus == 1){
    have_focus = 0;
  }
}

void KSircTopLevel::control_message(int command, QString str)
{
  switch(command){
  case CHANGE_CHANNEL: // 001 is defined as changeChannel
    {
      KSircChannel ci(m_channelInfo.server(), QString::null);
      QRegExp rx("([^!]+)!+([^!]+)!*([^!]*)");
      if(rx.search(str) < 0){
          ci.setChannel(str);
      }
      else{
          ci.setChannel(rx.cap(2));
          ci.setKey(rx.cap(3));
      }
      //kdDebug(5008) << "CHANGE NAME: " << ci.channel() << " key: " << ci.key() << endl;
      emit changeChannel(m_channelInfo.channel(), ci.channel());
      ksopts->channelSetup(m_channelInfo.server(), m_channelInfo.channel());
      m_channelInfo.setChannel(ci.channel());
      m_channelInfo.setKey(ci.key());
      m_channelInfo.setEncoding(ksopts->chan(m_channelInfo).encoding);
      if ( !logFile && ksopts->chan(m_channelInfo).logging )
      {
          logFile = new LogFile( m_channelInfo.channel(), ksircProcess()->serverName() );
          logFile->open();
      }
      setName(m_channelInfo.server().utf8() + "_" + m_channelInfo.channel().utf8() + "_" + "toplevel");
      pan->setName(QCString(QObject::name()) + "_" + "QSplitter");
      kmenu->setName(QCString(QObject::name()) + "_ktoolframe");
      linee->setName(QCString(QObject::name()) + "_" + "LineEnter");
      kmenu->show();
      setCaption(m_channelInfo.channel());
      emit currentWindow(this);

      if(m_channelInfo.channel()[0] == '#' || m_channelInfo.channel()[0] == '&'){
	  QString str = QString("/join %1 %2\n").arg(m_channelInfo.channel()).arg(m_channelInfo.key());
	  kdDebug(5008) << "Doing /join " << m_channelInfo.channel() << " :" << str << endl;
          emit outputUnicodeLine(str);
          emit outputLine("/eval $query=''\n");
      }
      else if (m_channelInfo.channel()[0] != '!')
      {
          emit outputUnicodeLine(QString("/eval $query='%1'\n").arg(m_channelInfo.channel()));
      }

      bool isPrivate = isPrivateChat();

      mainw->setAcceptFiles( isPrivate );

      if ( isPrivate )
      {
          nicks_box->hide();
          ksTopic->hide();
          file->setItemEnabled(topicitem, false);
      }
      else
      {
          file->setItemEnabled(topicitem, true);
          file->setItemChecked(topicitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].topicShow);
          if(file->isItemChecked(topicitem)){
              ksTopic->show();
          //    channelButtons->show();
          }
          nicks_box->show();
          pan->show();
//          channelButtons->show();
//          nicks->show();
//        lag->show();
      }
      if(layout()) {
          //kdDebug(5008) << "Redoeing layout" << endl;
          layout()->invalidate();
          layout()->activate();
      }
      else {
          //kdDebug(5008) << "No layout" << endl;
      }
      pan->refresh();

      ksopts->channelSetup(m_channelInfo.server(), m_channelInfo.channel());
      file->setItemChecked(topicitem, ksopts->chan(m_channelInfo).topicShow);
      file->setItemChecked(beepitem, ksopts->chan(m_channelInfo).beepOnMsg);
      file->setItemChecked(tsitem, ksopts->chan(m_channelInfo).timeStamp);
      file->setItemChecked(fjpitem, ksopts->chan(m_channelInfo).filterJoinPart);
      int eindex = encodingAction->items().findIndex(ksopts->chan(m_channelInfo).encoding);
      //kdDebug(5008) << "in change channel we want: " << ksopts->chan(m_channelInfo).encoding << " eindex: " << eindex << endl;
      encodingAction->setCurrentItem(eindex < 0 ? 0 : eindex);
      setEncoding();
      mainw->enableTimeStamps(ksopts->chan(m_channelInfo).timeStamp);

      break;
    }
  case STOP_UPDATES:
    Buffer = TRUE;
    break;
  case RESUME_UPDATES:
    Buffer = FALSE;
    if(LineBuffer.isEmpty() == FALSE)
      sirc_receive(QString::null);
    break;
  case REREAD_CONFIG:
    emit freezeUpdates(TRUE); // Stop the list boxes update
    selector->setFont( ksopts->defaultFont.family() );
    mainw->setFont( ksopts->defaultFont );
    nicks->setFont( ksopts->defaultFont );
    linee->setFont( ksopts->defaultFont );
    ksTopic->setFont( ksopts->defaultFont );
    UserUpdateMenu();  // Must call to update Popup.
    emit freezeUpdates(FALSE); // Stop the list boxes update
    initColors();
    ksopts->channelSetup(m_channelInfo.server(), m_channelInfo.channel());
    file->setItemChecked(topicitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].topicShow);
    file->setItemChecked(beepitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].beepOnMsg);
    file->setItemChecked(tsitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp);
    {
	int eindex = encodingAction->items().findIndex(ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].encoding);
	//kdDebug(5008) << "in re-readconfig we want: " << ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].encoding << " eindex: " << eindex << endl;
	encodingAction->setCurrentItem(eindex < 0 ? 0 : eindex);
	setEncoding();
    }
    mainw->enableTimeStamps(ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp);
    if(ksopts->oneLineEntry == true) {
        linee->setWordWrap(QTextEdit::NoWrap);
    }
    else {
        linee->setWordWrap(QTextEdit::WidgetWidth);
    }
    mainw->scrollToBottom(true);
    update();
    break;
  case SET_LAG:
    if(str.isNull() == FALSE){
      bool ok = TRUE;

      str.truncate(6);
      double lag = str.toDouble(&ok);
      if(ok == TRUE){
        lag -= (lag*100.0 - int(lag*100.0))/100.0;
        QString s_lag;
        s_lag.sprintf("Lag: %.2f", lag);
        this->lag->setText(s_lag);
      }
      else{
        this->lag->setText(str);
      }
    }
    break;
  case RESET_NOTIF:
    m_gotMsgWithoutFocus = 0;
    break;
  default:
    kdDebug(5008) << "Unkown control message: " << str << endl;
  }
}

void KSircTopLevel::setTopic( const QString &topic )
{
    m_topic = topic;
    ksTopic->setText( topic );
}

void KSircTopLevel::toggleTimestamp()
{
    ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp = !ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp;
    ksopts->save(KSOptions::Channels);
    mainw->enableTimeStamps( ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp );
    file->setItemChecked( tsitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].timeStamp );
}

void KSircTopLevel::toggleFilterJoinPart()
{
    ksopts->chan(m_channelInfo).filterJoinPart = !ksopts->chan(m_channelInfo).filterJoinPart;
    ksopts->save(KSOptions::Channels);
    file->setItemChecked( fjpitem, ksopts->chan(m_channelInfo).filterJoinPart );
}

QString KSircTopLevel::findNick(QString part, uint which)
{
    QStringList matches;

    for (QStringList::ConstIterator it = completeNicks.begin();
	 it != completeNicks.end();
	 ++it)
    {
	if ((*it).left(part.length()).lower() == part.lower() && nicks->findNick(*it) >= 0) {
	    matches.append(*it);
	}
    }

    for(uint i=0; i < nicks->count(); i++){
	if (matches.contains(nicks->text(i)))
	    continue;
	if(nicks->text(i).length() >= part.length()){
	    if(nicks->text(i).lower().startsWith( part.lower())){
		if(nicks->text(i) != ksircProcess()->getNick() ){ // Don't match your own nick
		    matches.append(nicks->text(i));
		}
	    }
	}
    }
    if(matches.count() > 0){
	if(which < matches.count())
	    return *(matches.at(which));
	else
	    return QString::null;
    }
    return QString::null;

}

void KSircTopLevel::removeCompleteNick(const QString &nick)
{
  QStringList::Iterator it = completeNicks.find(nick);
  if (it != completeNicks.end())
    completeNicks.remove(it);
}

void KSircTopLevel::addCompleteNick(const QString &nick)
{
  removeCompleteNick(nick);
  completeNicks.prepend(nick);
}

void KSircTopLevel::changeCompleteNick(const QString &oldNick, const QString &newNick)
{
  QStringList::Iterator it = completeNicks.find(oldNick);
  if (it != completeNicks.end())
    *it = newNick;
}

void KSircTopLevel::openQueryFromNick(const QString &nick)
{
    KSircChannel ci(m_channelInfo.server(), nick.lower());
    emit open_toplevel(ci);
}

void KSircTopLevel::pasteToWindow()
{
    // Ctrl-V
  //kdDebug(5008) << "Going to paste: " << KApplication::clipboard()->text( QClipboard::Clipboard ) << endl;
  slotTextDropped(KApplication::clipboard()->text( QClipboard::Clipboard ) );
}

void KSircTopLevel::pasteToNickList(int button,
				    QListBoxItem *item,
				    const QPoint &)
{
    if(button == Qt::MidButton && item){
	KSircChannel ci(m_channelInfo.server(), item->text().lower());
	emit open_toplevel(ci);

	QStringList lines = QStringList::split( '\n',
						KApplication::clipboard()->text( QClipboard::Selection ),
						true );
	QStringList::ConstIterator it = lines.begin();
	QStringList::ConstIterator end = lines.end();
	for (; it != end; ++it )
	{
	    if ((*it).isEmpty())
		continue;

	    QString str =
		QString("/msg ") + item->text().lower() + " " +
                *it + "\n";
	    emit outputUnicodeLine(str);

	}
    }

}

void KSircTopLevel::dndTextToNickList(const QListBoxItem *item,
				      const QString& text)
{
    if(item){
	KSircChannel ci(m_channelInfo.server(), item->text().lower());
	emit open_toplevel(ci);

	QStringList lines = QStringList::split( '\n',
						text,
						true );
	QStringList::ConstIterator it = lines.begin();
	QStringList::ConstIterator end = lines.end();
	for (; it != end; ++it )
	{
	    if ((*it).isEmpty())
		continue;

	    QString str =
		QString("/msg ") + item->text().lower() + " " +
		*it + "\n";
	    emit outputUnicodeLine(str);

	}
    }

}

enum KSircTopLevelCommands{ Ask, Parse, Escape };
void KSircTopLevel::slotTextDropped( const QString& _text )
{
  if (_text.isEmpty())
    return;
  QString text = linee->text();
  int curPos = linee->cursorPosition();

  text = text.mid( 0, curPos ) + _text + text.mid( curPos );

  if (text[text.length()-1] != '\n')
     text += "\n";
  int lines = text.contains("\n");
  int approx_lines = text.length() / 75;
  if ( lines > approx_lines )
     approx_lines = lines;
  if (lines > 4) {
      int result =  KMessageBox::warningContinueCancel(this,
                i18n("You are about to send %1 lines of text.\nDo you really want to send that much?").arg(approx_lines),
                QString::null, i18n("Send"));
      if (result != KMessageBox::Continue)
      {
//        linee->setText("");
        return;
      }
  }

  tab_pressed = -1;
  if(lines > 1){
    linee->setUpdatesEnabled(FALSE);

    QStringList lines = QStringList::split( '\n', text, true );
    QStringList::ConstIterator it = lines.begin();
    QStringList::ConstIterator end = lines.end();
    KSircTopLevelCommands commands = Ask;
    for (; it != end; ++it )
    {
        if ((*it).isEmpty())
            continue;
        QString line = *it;
        if ( line[0].latin1() == '/' )
        {
           if ( commands == Ask )
                switch ( KMessageBox::questionYesNo( this, i18n(
                    "The text you pasted contains lines that start with /.\n"
                    "Should they be interpreted as IRC commands?" ), QString::null, i18n("Interpret"), i18n("Do Not Interpret") ) )
                {
                    case KMessageBox::Yes:
                        commands = Parse;
                        break;
                    case KMessageBox::No:
                        commands = Escape;
                        break;
                }
            if ( commands == Escape )
                line.prepend( "/say " );
        }
        linee->setText( line );
        sirc_line_return(line);
    }

    linee->setText("");
    linee->setUpdatesEnabled(TRUE);
    linee->update();
  }
  else{
    text.replace(QRegExp("\n"), "");
    linee->setText(text);
    linee->setCursorPosition( curPos + _text.length() );
  }
}

void KSircTopLevel::clearWindow()
{
    kdDebug(5008) << "Doing clear on main window" << endl;
    mainw->clear();
}

void KSircTopLevel::lineeNotTab()
{
  tab_pressed = -1;
  disconnect(linee, SIGNAL(notTab()),
	     this, SLOT(lineeNotTab()));
  addCompleteNick(tab_nick);
}

void KSircTopLevel::toggleRootWindow()
{
}

bool KSircTopLevel::event( QEvent *e)
{
    if(e->type() == QEvent::ApplicationPaletteChange ||
       e->type() == QEvent::ApplicationFontChange)
    {
	QTimer::singleShot(750, this, SLOT(initColors()));
	initColors();
    }
    return KMainWindow::event(e);
}

void KSircTopLevel::saveCurrLog()
{
    KURL url = KURL( KFileDialog::getSaveFileName(QString::null,
                                            "*.log", 0L,
                                            i18n("Save Chat/Query Logfile")));
    if( url.isEmpty())
      return;

    KTempFile temp;
    QTextStream *str = temp.textStream();

    *str << mainw->plainText();

    temp.close();
#if KDE_IS_VERSION(3,1,92)
    KIO::NetAccess::upload(temp.name(), url, this);
#else
    KIO::NetAccess::upload(temp.name(), url);
#endif
}


void KSircTopLevel::slotDropURLs( const QStringList& urls )
{
    if ( !isPrivateChat() )
        return;

    slotDccURLs( urls, m_channelInfo.channel() );
}

// sends the list of urls to $dest_nick
void KSircTopLevel::slotDccURLs( const QStringList& urls, const QString& nick )
{
    if ( urls.isEmpty() || nick.isEmpty() )
        return;

    QStringList::ConstIterator it = urls.begin();
    // QString s("/eval &docommand(eval{\"dcc send " + nick + " %1\"});\n");
    QString s("/dcc send " + nick + " %1\n");
    for ( ; it != urls.end(); ++it ) {
        QString file( *it );
        kdDebug(5008) << "........ " << file << endl;
        if ( !file.isEmpty() )
            sirc_write(s.arg( file ));
    }
}

bool KSircTopLevel::isPrivateChat() const
{
    return ((m_channelInfo.channel()[0] != '!') && (m_channelInfo.channel()[0] != '&') &&
            (m_channelInfo.channel()[0] != '#'));
}

bool KSircTopLevel::isPublicChat() const
{
    return ((m_channelInfo.channel()[0] == '#') || (m_channelInfo.channel()[0] == '&'));
}

bool KSircTopLevel::isSpecialWindow() const
{
    return (m_channelInfo.channel()[0] == '!');
}

void KSircTopLevel::outputUnicodeLine( const QString &message )
{
    emit outputLine( encoder()->fromUnicode( message ) );
}

void KSircTopLevel::setTopicIntern( const QString &topic )
{
    QString command = QString::fromLatin1( "/topic %1 %2\n" ).arg( m_channelInfo.channel() ).arg( topic );
    sirc_write( command );
    linee->setFocus();
}

void KSircTopLevel::doChange(bool pers, QString text)
{
    QTime ctime = QTime::currentTime();
    if (ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].beepOnMsg) {
        // beep only every 2 seconds otherwise it'll get very noisy in the room ;)
        if ( QABS( lastBeep.secsTo( ctime ) ) > 1 ) {
	    if ( pers ) {
		int winId = !ticker ? topLevelWidget()->winId() : ticker->winId();
		KNotifyClient::event(winId,
				     QString::fromLatin1("ChannelPersonal"),
				     i18n("Your nick appeared on channel %1").arg(m_channelInfo.channel()));
	    } else {
		int winId = !ticker ? topLevelWidget()->winId() : ticker->winId();
		KNotifyClient::event(winId,
				     QString::fromLatin1("ChannelChanged"),
				     i18n("Channel %1 changed").arg(m_channelInfo.channel()));
	    }
	    lastBeep = ctime;
        }
    }

    if(have_focus == 0 && pers == true && m_gotMsgWithoutFocus == false){
	m_gotMsgWithoutFocus = true;
        servercontroller::self()->increaseNotificationCount(QString("%1 -> %2").arg(ksircProcess()->serverID()).arg(m_channelInfo.channel()), text);
    }
}

void KSircTopLevel::toggleBeep()
{
    ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].beepOnMsg = !ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].beepOnMsg;
    ksopts->save(KSOptions::Channels);
    file->setItemChecked(beepitem, ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].beepOnMsg);
}

void KSircTopLevel::toggleTopic()
{
	if(file->isItemChecked(topicitem)){
		ksTopic->hide();
//                channelButtons->hide();
                ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].topicShow = false;
	}
	else {
		ksTopic->show();
//                channelButtons->show();
                ksopts->channel[m_channelInfo.server()][m_channelInfo.channel()].topicShow = true;
	}
        file->setItemChecked(topicitem, !file->isItemChecked(topicitem));
        ksopts->save(KSOptions::Channels);
}

void KSircTopLevel::toggleTicker()
{
    //kdDebug(5008) << "Got toggle ticker" << endl;
    if(ticker){
        show();
        displayMgr->show(this);
        linee->setFocus(); // Give focus back to the linee, someone takes it away on new create
        tickerpoint = ticker->pos();
        tickersize = ticker->size();
        delete ticker;
        ticker = 0x0;
    }
    else {
        ticker = new KSTicker(0x0, m_channelInfo.channel().utf8() + "_ticker");
        ticker->setCaption(m_channelInfo.channel());
        if(tickerpoint.isNull() == false)
            ticker->move(tickerpoint);
        if(tickersize.isEmpty() == false)
            ticker->resize(tickersize);
        ticker->show();
        displayMgr->hide(this);

        connect(ticker, SIGNAL(doubleClick()), this, SLOT(toggleTicker()));
    }
}

bool KSircTopLevel::atBottom()
{
    return mainw->verticalScrollBar()->maxValue() - mainw->verticalScrollBar()->value() < 20;
}

void KSircTopLevel::cmd_process(int id)
{
    if(cmd_menu.at(id) != cmd_menu.end()) {
        QString item, arg, out;
        item = cmd_menu[id].section('/', 1, 1);
        arg = cmd_menu[id].section('/', 2, 2);

        out = "/" + item;

        if(arg == "*chan*")
            out.append(" " + m_channelInfo.channel());

        out.append(" ");

        linee->insertAt(out, 0, 0);
        linee->setCursorPosition(linee->cursorPosition() + out.length());
    }
}

void KSircTopLevel::showDCCMgr()
{
    KSircMessageReceiver *kmr = ksircProcess()->mrList().find("!dcc");
    if(kmr){
	KSircIODCC *dcc = static_cast<KSircIODCC *>(kmr);
	if(dcc){
            dcc->showMgr();
	}
    }
}

#include "toplevel.moc"

// vim: ts=2 sw=2 et
