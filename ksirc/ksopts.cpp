
#include <kapplication.h>
#include <kconfig.h>

#if KDE_IS_VERSION(3,1,92)
#include <kuser.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <kdebug.h>

#include "ksopts.h"


KSOptions *KSOptions::s_options = 0;
bool KSOChannel::applyGlobally = false;

KSOGeneral::KSOGeneral()
{
    displayMode = MDI;
    autoCreateWin = true;
    autoCreateWinForNotice = false;
    nickCompletion = true;
    displayTopic = true;
    oneLineEntry = true;
    runDocked = false;
//    timeStamp = true;
//    beepNotify = true;
    colorPicker = true;
    autoRejoin = false;
    windowLength = 200;
//    logging = false;
//    beepOnMsg = false;
    publicAway = true;
    //    topicShow = true;
    useColourNickList = false;
    dockPopups = true;
    autoSaveHistory = true;

}

QPixmap KSOGeneral::backgroundPixmap()
{
    if ( m_backgroundPixmap.isNull() && !backgroundFile.isEmpty() )
	m_backgroundPixmap = QPixmap( backgroundFile );
    return m_backgroundPixmap;
}

KSOStartup::KSOStartup()
{
}

KSOServer::KSOServer()
{
#if KDE_IS_VERSION(3,1,92)
    KUser user;
    nick = user.loginName();
    userID = user.loginName();
    realName = user.fullName();
#else
    struct passwd *pw = getpwuid( getuid() );
    if ( pw )
    {
	nick = QString::fromLocal8Bit( pw->pw_name );
	userID = QString::fromLocal8Bit( pw->pw_name );
	realName = QString::fromLocal8Bit( pw->pw_gecos ).section( ',', 0, 0 );
    }
#endif
}


KSOColors::KSOColors()
{
    textColor = Qt::black;
    linkColor = Qt::blue;
    infoColor = Qt::blue;
    channelColor = Qt::green;
    errorColor = Qt::red;

    ksircColors = true;
    mircColors = false;
    nickColourization = false;
    ownNickBold = false;
    ownNickUl = true;
    ownNickRev = false;

    msg1Regex = false;
    msg2Regex = false;

    backgroundColor = kapp->palette().color( QPalette::Active, QColorGroup::Mid );
    selBackgroundColor = kapp->palette().color( QPalette::Active, QColorGroup::Highlight );
    selForegroundColor = kapp->palette().color( QPalette::Active, QColorGroup::HighlightedText );

    ircColors[0] = Qt::white;
    ircColors[1] = Qt::black;
    ircColors[2] = Qt::darkBlue;
    ircColors[3] = Qt::darkGreen;
    ircColors[4] = Qt::red;
    ircColors[5] = QColor ( 165, 42, 42 );
    ircColors[6] = Qt::darkMagenta;
    ircColors[7] = QColor( 255, 0x7b, 00 );
    ircColors[8] = Qt::yellow;
    ircColors[9] = Qt::green;
    ircColors[10] = Qt::darkCyan;
    ircColors[11] = Qt::cyan;
    ircColors[12] = Qt::blue;
    ircColors[13] = QColor( 255, 0, 255 );
    ircColors[14] = Qt::gray;
    ircColors[15] = Qt::lightGray;

    uint i;
    for(i = 0; i < numColors; i++){
        nickHighlight[i] = true;
    }

    colourTheme = "Custom";
}

KSORMBMenu::KSORMBMenu()
{
}

KSOServChan::KSOServChan()
{
}

KSOChannel::KSOChannel()
{
    timeStamp = false;
    beepNotify = false;
    beepOnMsg = false;
    logging = false;
    topicShow = true;
    filterJoinPart = false;
    applyGlobally = false;
    encoding = "Default";
}

void KSOptions::load( int sections )
{
    KConfig *conf = kapp->config();
    conf->setGroup( "ServerController" );
    geometry = conf->readRectEntry( "Size" );

    // Using the fields itself as default values as they are already
    // set to the defaults.
    if ( sections & General )
    {
        conf->setGroup( "General" );
        int mode = conf->readNumEntry( "DisplayMode", static_cast<int>(displayMode) );
        if (mode == 0)
            displayMode = SDI;
        else
            displayMode = MDI;

        autoCreateWin = conf->readBoolEntry( "AutoCreateWin", autoCreateWin );
        autoCreateWinForNotice = conf->readBoolEntry( "AutoCreateWinForNotice", autoCreateWinForNotice );
        nickCompletion = conf->readBoolEntry( "NickCompletion", nickCompletion );
        displayTopic = conf->readBoolEntry( "DisplayTopic", displayTopic );
	oneLineEntry = conf->readBoolEntry( "OneLineEntry", oneLineEntry );
	runDocked = conf->readBoolEntry( "runDocked", runDocked );
//        timeStamp = conf->readBoolEntry( "TimeStamp", timeStamp );
//        beepNotify = conf->readBoolEntry( "BeepNotify", beepNotify );
        colorPicker = conf->readBoolEntry( "ColourPicker", colorPicker );
        autoRejoin = conf->readBoolEntry( "AutoRejoin", autoRejoin );
        windowLength = conf->readNumEntry( "WindowLength", windowLength );
        backgroundFile = conf->readPathEntry( "BackgroundFile", backgroundFile );
//        logging = conf->readBoolEntry( "Logging", logging );
//      beepOnMsg = conf->readBoolEntry( "BeepOnMessage", false );
        publicAway = conf->readBoolEntry( "PublicAway", publicAway );
        //      topicShow = conf->readBoolEntry("TopicShow", topicShow);
	useColourNickList = conf->readBoolEntry("ColourNickList", useColourNickList);
	dockPopups = conf->readBoolEntry("dockPopups", dockPopups);
        autoSaveHistory = conf->readBoolEntry("autoSaveHistory", autoSaveHistory);
    }

    if ( sections & Startup )
    {
	conf->setGroup( "StartUp" );
    }


    if ( sections & Colors )
    {
        if ( !conf->hasGroup( "ColorScheme" ) && conf->hasGroup( "Colours" ) )
        {
            conf->setGroup( "Colours" );
            textColor = conf->readColorEntry( "text", &textColor );
            infoColor = conf->readColorEntry( "info", &infoColor );
            channelColor = conf->readColorEntry( "chan", &channelColor );
            errorColor = conf->readColorEntry( "error", &errorColor );
            int c = conf->readNumEntry( "uscolour", -1 );
            if ( c >= 0 && c < 16 )
                ownNickColor = ircColors[c];
            c = conf->readNumEntry( "nickfcolour", -1 );
            if ( c >= 0 && c < 16 )
                nickForeground = ircColors[c];
            c = conf->readNumEntry( "nickbcolour", -1 );
            if ( c >= 0 && c < 16 )
                nickBackground = ircColors[c];
            backgroundColor = conf->readColorEntry( "Background", &backgroundColor );
        }
        else
        {
            conf->setGroup( "ColorScheme" );
            textColor = conf->readColorEntry( "Text", &textColor );
            linkColor = conf->readColorEntry( "Link", &linkColor );
            infoColor = conf->readColorEntry( "Info", &infoColor );
            channelColor = conf->readColorEntry( "Channel", &channelColor );
            errorColor = conf->readColorEntry( "Error", &errorColor );
	    ownNickColor = conf->readColorEntry( "OwnNick", &ownNickColor );
	    ownNickBold = conf->readBoolEntry( "OwnNickBold", ownNickBold );
	    ownNickUl = conf->readBoolEntry( "OwnNickUnderline", ownNickUl );
            ownNickRev = conf->readBoolEntry( "OwnNickReverse", ownNickRev );
	    msgContainNick = conf->readColorEntry( "msgContainNick", &ownNickColor ); // default to the ownNick colour
	    msg1Contain = conf->readColorEntry( "msg1Contain", &msg1Contain );
	    msg1String = conf->readEntry( "msg1String", msg1String );
            msg1Regex = conf->readBoolEntry( "msg1Regex", msg1Regex );
	    msg2Contain = conf->readColorEntry( "msg2Contain", &msg2Contain );
	    msg2String = conf->readEntry( "msg2String", msg2String );
            msg2Regex = conf->readBoolEntry( "msg2Regex", msg2Regex );
            nickForeground = conf->readColorEntry( "NickForeground", &nickForeground );
            nickBackground = conf->readColorEntry( "NickBackground", &nickBackground );
            backgroundColor = conf->readColorEntry( "Background", &backgroundColor );
            selBackgroundColor = conf->readColorEntry( "SelBackground", &selBackgroundColor );
	    selForegroundColor = conf->readColorEntry( "SelForeground", &selForegroundColor );
	    ksircColors = conf->readBoolEntry( "ksircColors", ksircColors );
	    mircColors = conf->readBoolEntry( "mircColors", mircColors );
	    nickColourization = conf->readBoolEntry( "nickColourization", mircColors );
            colourTheme = conf->readEntry("ColourTheme", colourTheme );
	    for ( int i = 0; i < 16; ++i ) {
		ircColors[i] =
		    conf->readColorEntry(QString::fromLatin1( "IRC-%1" ).arg( i ),
					 &ircColors[i] );
                nickHighlight[i] =
		    conf->readBoolEntry(QString::fromLatin1( "nickHighlight-%1" ).arg( i ),
					 &nickHighlight[i] );
	    }
        }
	conf->setGroup( "GlobalOptions" );
        QFont df = kapp->font(0);
        defaultFont = conf->readFontEntry( "MainFont", &df);
        QApplication::setFont( defaultFont, true, "KSirc::TextView" );
    }
    if ( sections & Channels )
    {
        conf->setGroup( "Channel" );

        /*
         * do global first
         */
        if(channel.contains("global") == FALSE){
            KSOChannel global;
            ChannelOpMap coMap;
            channel.insert(QString("global"), coMap);
            channel["global"].insert("global", global);
            channel["global"]["global"].server = "global";
            channel["global"]["global"].channel = "global";
            channel["global"]["global"].timeStamp = conf->readBoolEntry("globalglobalTimeStamp", false);
            channel["global"]["global"].beepNotify = conf->readBoolEntry("globalglobalBeepNotify", false);
            channel["global"]["global"].beepOnMsg = conf->readBoolEntry("globalglobalBeepOnMsg", false);
            channel["global"]["global"].logging = conf->readBoolEntry("globalglobalLogging", false);
	    channel["global"]["global"].topicShow = conf->readBoolEntry("globalglobalTopicShow", true);
	    channel["global"]["global"].filterJoinPart = conf->readBoolEntry("globalglobalFilterJoinPart", false);
	    channel["global"]["global"].applyGlobally = conf->readBoolEntry("globalglobalApplyGlobally", false);
	    channel["global"]["global"].encoding = conf->readEntry("globalglobalEncoding", "Default");
        }

	QDateTime now(QDateTime::currentDateTime());
        QStringList servers = conf->readListEntry("Servers");
        QStringList::ConstIterator ser = servers.begin();
        for( ; ser != servers.end(); ser++){
            ChannelOpMap coMap;
            channel.insert(*ser, coMap);
            QStringList channels = conf->readListEntry(*ser);
            QStringList::ConstIterator chan = channels.begin();
	    for( ; chan != channels.end(); chan++){
		QDateTime lastUsed = conf->readDateTimeEntry(*ser + *chan + "lastUsed", &now);
		if((lastUsed.daysTo(now) < 30 )||
		   (*chan == "global" && *ser == "global"))
		{
		    KSOChannel ksoc;
		    ksoc = channel["global"]["global"];
		    ksoc.server = *ser;
		    ksoc.channel = *chan;
		    ksoc.timeStamp = conf->readBoolEntry(*ser + *chan + "TimeStamp", ksoc.timeStamp);
		    ksoc.beepNotify = conf->readBoolEntry(*ser + *chan + "BeepNotify", ksoc.beepNotify);
		    ksoc.beepOnMsg = conf->readBoolEntry(*ser + *chan + "BeepOnMsg", ksoc.beepOnMsg);
		    ksoc.logging = conf->readBoolEntry(*ser + *chan + "Logging", ksoc.logging);
		    ksoc.topicShow = conf->readBoolEntry(*ser + *chan + "TopicShow", ksoc.topicShow);
		    ksoc.encoding = conf->readEntry(*ser + *chan + "Encoding", ksoc.encoding);
		    ksoc.filterJoinPart = conf->readBoolEntry(*ser + *chan + "FilterJoinPart", ksoc.filterJoinPart);
                    ksoc.lastUsed = lastUsed;
		    channel[*ser].insert(*chan, ksoc);
		}
		else { /* expire junk */
		    conf->deleteEntry(*ser + *chan + "TimeStamp");
		    conf->deleteEntry(*ser + *chan + "BeepNotify");
		    conf->deleteEntry(*ser + *chan + "BeepOnMsg");
		    conf->deleteEntry(*ser + *chan + "Logging");
		    conf->deleteEntry(*ser + *chan + "TopicShow");
		    conf->deleteEntry(*ser + *chan + "Encoding");
		    conf->deleteEntry(*ser + *chan + "FilterJoinPart");
                    conf->deleteEntry(*ser + *chan + "lastUsed");
		}
            }
        }
    }
    if ( sections & Servers )
    {
	conf->setGroup( "Server" );

	/*
	 * do global first
	 */
	if(server.contains("global") == FALSE){
	    KSOServer global;
	    server.insert(QString("global"), global);
	    server["global"].server = "global";
	    server["global"].globalCopy = false;

	    conf->setGroup( "StartUp" );
	    server["global"].nick = conf->readEntry("Nick");
	    server["global"].altNick = conf->readEntry("altNick");
	    server["global"].realName = conf->readEntry("RealName");
	    server["global"].userID = conf->readEntry("userID");
	    server["global"].notifyList = conf->readListEntry("NotifyList");

	    conf->setGroup( "Server" );
	}

	kdDebug(5008) << "Load: in Server" << endl;

	QStringList servers = conf->readListEntry("Servers");

	QStringList::ConstIterator ser = servers.begin();
	for( ; ser != servers.end(); ser++){
	    KSOServer ksos;
	    ksos = server["global"];
	    ksos.server = *ser;
	    ksos.globalCopy = false;
	    ksos.nick = conf->readEntry(*ser + "nick");
	    ksos.altNick = conf->readEntry(*ser + "altNick");
	    ksos.realName = conf->readEntry(*ser + "realName");
	    ksos.userID = conf->readEntry(*ser + "userID");
	    ksos.notifyList = conf->readListEntry(*ser + "notifyList");
	    kdDebug(5008) << *ser << ": nick: " << ksos.nick << " altNick: " << ksos.altNick << endl;
	    server.insert(*ser, ksos);
	}
    }
    kdDebug(5008) << "Done full load" << endl;
}

void KSOptions::save( int sections )
{
    KConfig *conf = kapp->config();

    if ( sections & General )
    {
        conf->setGroup( "General" );
        conf->writeEntry( "DisplayMode", static_cast<int>(displayMode) );
        conf->writeEntry( "AutoCreateWin", autoCreateWin );
        conf->writeEntry( "AutoCreateWinForNotice", autoCreateWinForNotice );
        conf->writeEntry( "NickCompletion", nickCompletion );
        conf->writeEntry( "DisplayTopic", displayTopic );
	conf->writeEntry( "OneLineEntry", oneLineEntry );
	conf->writeEntry( "runDocked", runDocked );
//      conf->writeEntry( "TimeStamp", timeStamp );
//      conf->writeEntry( "BeepNotify", beepNotify );
        conf->writeEntry( "ColourPicker", colorPicker );
        conf->writeEntry( "AutoRejoin", autoRejoin );
        conf->writeEntry( "WindowLength", windowLength );
        conf->writePathEntry( "BackgroundFile", backgroundFile );
//      conf->writeEntry( "Logging", logging );
//      conf->writeEntry( "BeepOnMessage", beepOnMsg );
        conf->writeEntry( "PublicAway", publicAway );
	conf->writeEntry( "ColourNickList", useColourNickList );
	conf->writeEntry( "dockPopups", dockPopups );
        conf->writeEntry( "autoSaveHistory", autoSaveHistory );
    }

    if ( sections & Startup )
    {
	conf->setGroup( "StartUp" );
        /*
	conf->writeEntry( "Nick", nick );
	conf->writeEntry( "AlternativeNick", altNick );
	conf->writeEntry( "RealName", realName );
	conf->writeEntry( "userID", userID );
	conf->writeEntry( "NotifyList", notifyList );
        */
    }

    if ( sections & Colors )
    {
        conf->setGroup( "ColorScheme" );
        conf->writeEntry( "Text", textColor );
        conf->writeEntry( "Link", linkColor );
        conf->writeEntry( "Info", infoColor );
        conf->writeEntry( "Channel", channelColor );
        conf->writeEntry( "Error", errorColor );
	conf->writeEntry( "OwnNick", ownNickColor );
	conf->writeEntry( "OwnNickBold", ownNickBold );
	conf->writeEntry( "OwnNickUnderline", ownNickUl );
	conf->writeEntry( "OwnNickReverse", ownNickRev );
	conf->writeEntry( "msgContainNick", msgContainNick );
	conf->writeEntry( "msg1Contain", msg1Contain );
	conf->writeEntry( "msg1String", msg1String );
	conf->writeEntry( "msg1Regex", msg1Regex );
	conf->writeEntry( "msg2Contain", msg2Contain );
	conf->writeEntry( "msg2String", msg2String );
	conf->writeEntry( "msg2Regex", msg2Regex );
        conf->writeEntry( "NickForeground", nickForeground );
        conf->writeEntry( "NickBackground", nickBackground );
        conf->writeEntry( "Background", backgroundColor );
        conf->writeEntry( "SelBackground", selBackgroundColor );
	conf->writeEntry( "SelForeground", selForegroundColor );
	conf->writeEntry( "ksircColors", (bool) ksircColors );
	conf->writeEntry( "mircColors", mircColors );
	conf->writeEntry( "nickColourization", nickColourization );
        conf->writeEntry( "ColourTheme", colourTheme );
	for ( int i = 0; i < 16; ++i ) {
	    conf->writeEntry( QString::fromLatin1( "IRC-%1" ).arg( i ), ircColors[i] );
	    conf->writeEntry( QString::fromLatin1( "nickHighlight-%1" ).arg( i ), nickHighlight[i] );
	}
        /* Store the font setting */
        conf->setGroup( "GlobalOptions" );
        conf->writeEntry("MainFont", defaultFont);
        /* end of storing the font settings */
    }
    if ( sections & Channels )
    {
	QStringList servers;

	conf->deleteGroup( "Channel" );

        conf->setGroup( "Channel" );
        ServerChannelOpMap::Iterator ser;
        for( ser = channel.begin(); ser != channel.end(); ++ser ){
            QStringList channels;

            servers << ser.key();

            ChannelOpMap::Iterator chan;
            for( chan = ser.data().begin(); chan != ser.data().end(); ++chan ){
		channels << chan.key();
                conf->writeEntry(ser.key() + chan.key() + "TimeStamp", chan.data().timeStamp);
                conf->writeEntry(ser.key() + chan.key() + "BeepNotify", chan.data().beepNotify);
                conf->writeEntry(ser.key() + chan.key() + "BeepOnMsg", chan.data().beepOnMsg);
                conf->writeEntry(ser.key() + chan.key() + "Logging", chan.data().logging);
		conf->writeEntry(ser.key() + chan.key() + "TopicShow", chan.data().topicShow);
		conf->writeEntry(ser.key() + chan.key() + "Encoding", chan.data().encoding);
		conf->writeEntry(ser.key() + chan.key() + "FilterJoinPart", chan.data().filterJoinPart);
		conf->writeEntry(ser.key() + chan.key() + "lastUsed", chan.data().lastUsed);
            }
            conf->writeEntry(ser.key(), channels);
        }
        conf->writeEntry("Servers", servers);
        conf->writeEntry("globalglobalApplyGlobally", channel["global"]["global"].applyGlobally);
    }
    if ( sections & Servers )
    {
	QStringList servers;

	conf->deleteGroup( "Servers" );

        conf->setGroup( "Server" );
        ServerOpMap::Iterator ser;
        for( ser = server.begin(); ser != server.end(); ++ser ){
	    QStringList channels;

	    if(ser.data().globalCopy == true)
                continue;

            servers << ser.key();

	    conf->writeEntry(ser.key() + "nick", ser.data().nick);
	    conf->writeEntry(ser.key() + "altNick", ser.data().altNick);
	    conf->writeEntry(ser.key() + "realName", ser.data().realName);
	    conf->writeEntry(ser.key() + "userID", ser.data().userID);
	    conf->writeEntry(ser.key() + "notifyList", ser.data().notifyList);
	}
	conf->writeEntry("Servers", servers);
    }

    conf->sync();
}

void KSOptions::channelSetup(QString serv, QString chan)
{
    if(channel.contains(serv) == FALSE){
        ChannelOpMap coMap;
        channel.insert(serv, coMap);
    }
    if(channel[serv].contains(chan) == FALSE){
        KSOChannel ksoc;
        ksoc = channel["global"]["global"];
        channel[serv].insert(chan, ksoc);
        channel[serv][chan].server = serv;
	channel[serv][chan].channel = chan;
	channel[serv][chan].lastUsed = QDateTime::currentDateTime();
    }
    else {
	channel[serv][chan].lastUsed = QDateTime::currentDateTime();
    }
}

void KSOptions::applyChannelGlobal(void)
{
    ServerChannelOpMap::Iterator ser;
    for( ser = channel.begin(); ser != channel.end(); ++ser ){
        ChannelOpMap::Iterator chan;
        for( chan = ser.data().begin(); chan != ser.data().end(); ++chan ){
            if((chan.key() == "global") && (ser.key() == "global"))
                continue;

            chan.data() = channel["global"]["global"];
        }
    }

}

void KSOptions::serverSetup(QString serv)
{
    if(server.contains(serv) == FALSE){
        KSOServer ksos;
        ksos = server["global"];
        server.insert(serv, ksos);
	server[serv].server = serv;
        server[serv].globalCopy = true;
	server[serv].lastUsed = QDateTime::currentDateTime();
    }
    else {
	server[serv].lastUsed = QDateTime::currentDateTime();
    }
}


/* vim: et sw=4 ts=4
 */
