
#ifndef _KSOPTS_H_
#define _KSOPTS_H_
#include <qmap.h>
#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qrect.h>
#include <qpixmap.h>
#include <qdatetime.h>

#define IRC_SAFE_MAX_LINE 450

#include "boundscheckingarray.h"
#include "ksircchannel.h"
#include "ksircserver.h"

// Central place to hold KSirc's options
// and manage default values. The base classes
// are only used directly by the prefs dialog.
// The constructors initialize all options with default values.
class KSOGeneral
{
public:
    KSOGeneral();

    enum { SDI, MDI } displayMode;
    bool autoCreateWin : 1;
    bool autoCreateWinForNotice : 1;
    bool nickCompletion : 1;
    bool displayTopic : 1;
    bool oneLineEntry : 1;
    bool runDocked : 1;
//    bool timeStamp : 1;
//    bool beepNotify : 1;
    bool colorPicker : 1;
    bool autoRejoin : 1;
//    bool beepOnMsg : 1;
//    bool logging : 1;
    bool publicAway : 1;
    bool useColourNickList : 1;
    //    bool topicShow : 1;
    bool dockPopups : 1;
    bool autoSaveHistory : 1;
    int windowLength;
    QString backgroundFile;

    QPixmap backgroundPixmap();

private:
    QPixmap m_backgroundPixmap;
};

class KSOChannel
{
public:
    KSOChannel();

    QString server;
    QString channel;

    bool timeStamp : 1;
    bool beepNotify : 1;
    bool beepOnMsg : 1;
    bool logging : 1;
    bool topicShow : 1;
    bool filterJoinPart : 1;
    QString encoding;

    QDateTime lastUsed;

    static bool applyGlobally;



};

class KSOServer
{
public:
    KSOServer();

    QString server;
    bool globalCopy;

    QString nick;
    QString altNick;
    QString realName;
    QString userID;
    QStringList notifyList;

    QDateTime lastUsed;
};

class KSOStartup
{
public:
    KSOStartup();

    QRect geometry;
//    QString nick;
//    QString altNick;
//    QString realName;
//    QString userID;
//    QStringList notifyList;
};

class KSOColors
{
public:
    enum { numColors = 16 };

    KSOColors();

    QColor textColor;
    QColor linkColor;
    QColor infoColor;
    QColor channelColor;
    QColor errorColor;
    QColor ownNickColor;
    bool ownNickBold : 1;
    bool ownNickUl : 1;
    bool ownNickRev : 1;

    QColor msgContainNick; // needs config + impl
    QColor msg1Contain;    // needs config
    QString msg1String;    // needs config
    bool   msg1Regex;
    QColor msg2Contain;    // need config
    QString msg2String;    // needs config
    bool   msg2Regex;

    QColor nickForeground;
    QColor nickBackground;
    QColor backgroundColor;
    QColor selBackgroundColor;
    QColor selForegroundColor;
    //QColor ircColors[numColors];
    BoundsCheckingArray<QColor, numColors> ircColors;
    BoundsCheckingArray<bool, numColors> nickHighlight;

    QString colourTheme;

    QFont defaultFont;
    bool ksircColors : 1;
    bool mircColors : 1;
    bool nickColourization : 1;

};

class KSORMBMenu
{
public:
    KSORMBMenu();
};

class KSOServChan
{
public:
    KSOServChan();
};

typedef QMap<QString, KSOChannel> ChannelOpMap;
typedef QMap<QString, ChannelOpMap> ServerChannelOpMap;
typedef QMap<QString, KSOServer> ServerOpMap;

class KSOptions
    : public KSOGeneral,
    public KSOStartup,
    public KSOColors,
    public KSORMBMenu,
    public KSOServChan
{
public:
    KSOptions() { s_options = this; }

    void load( int sections = -1 );
    void save( int sections = -1 );

    static KSOptions *options() { return s_options; }

    ServerChannelOpMap channel;
    ServerOpMap server;

    KSOChannel &chan( const KSircChannel &chanInfo )
    {
        return channel[chanInfo.server()][chanInfo.channel()];
    }

    KSOServer &serv( const KSircServer &servInfo )
    {
        return server[servInfo.server()];
    }

    void channelSetup(QString serv, QString chan);
    void applyChannelGlobal(void);

    void serverSetup(QString server);
    void serverSetup( const KSircServer &servInfo )
    {
        serverSetup(servInfo.server());
    }

    enum {
        All = -1, General = 1, Startup = 2,
        Colors = 4, RMBMenu = 8, ServChan = 16,
        Channels = 32, Servers = 64
    };

private:
    static KSOptions *s_options;
};

#define ksopts (KSOptions::options())

#endif
