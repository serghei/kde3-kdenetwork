/**********************************************************************

	--- Qt Architect generated file ---

	File: servercontroller.h
	Last generated: Sat Nov 29 08:50:19 1997

 Now Under CVS control.

 $$Id: servercontroller.h 416942 2005-05-22 15:46:04Z coolo $$

 *********************************************************************/

#ifndef servercontroller_included
#define servercontroller_included

class servercontroller;
class dockServerController;
class ServMessage;
class ProcCommand;

#include <qdict.h>
#include <qpixmap.h>
#include <qheader.h>
#include <qtimer.h>

#include <klistview.h>
#include <kmainwindow.h>

#include "ksircprocess.h"
#include "ksircchannel.h"

//#include "puke/controller.h"
class QLabel;
class KMenuBar;
class KSircServer;
class KGlobalAccel;
class nickColourMaker;
class dockServerController;

class ProcCommand // ServerController message
{
 public:
  static enum {
    addTopLevel,
    deleteTopLevel,
    procClose,
    newChannel,
    changeChannel,
    nickOnline,
    nickOffline,
    turnOffAutoCreate,
    turnOnAutoCreate
  } command;
};


class ServCommand // ServerController message
{
 public:
  static enum {
    updateFilters,
    updatePrefs
  } command;
};

class scInside : QFrame
{
  Q_OBJECT
  friend class servercontroller;
 public:
  scInside ( QWidget *parent = 0L, const char * name = 0, WFlags f=0 );
  ~scInside();

 protected:
  virtual void resizeEvent ( QResizeEvent * );

 private:
  KListView *ConnectionTree;
  QLabel *ASConn;

};

class servercontroller : public KMainWindow
{
    Q_OBJECT
    friend class dockServerController;
public:

    servercontroller ( QWidget* parent = 0L, const char* name = NULL );
    virtual ~servercontroller();

    const QDict<KSircProcess> &processes() const { return proc_list; }

    static servercontroller *self() { return s_self; }

    /**
     * Someone is talking to the user (blue icon), notify him (using the docked icon).
     */
    void increaseNotificationCount(const QString& reason = QString::null, const QString& text = QString::null);

    /**
     * The channel in which the user was talked to, has been read.
     * -> decrease count of blue icons.
     */
    void decreaseNotificationCount(QString reason = QString::null);

    /**
     * This resets all notificaiton counts and allows new ones
     * this is used if we don't want to give the window
     * focus to reset focus
     */
    void resetNotification();

    void checkDocking();

    KGlobalAccel *getGlobalAccel(){ return m_kga; }

signals:
    /**
      * Filter rules have changed, need to re-read and update.
      */
    virtual void filters_update();

    void ServMessage(QString server, int command, QString args);

public slots:
    // All slots are described in servercontroll.cpp file
    /**
     * Does auto-joins on start up
     */
    virtual void do_autoconnect();
    /**
      * Creates popup asking for new connection
      */
    virtual void new_connection();
    /**
      *  Args:
      *    QString: new server name or IP to connect to.
      *  Action:
      *	 Creates a new sirc process and window !default connected to the
      *	 server.  Does nothing if a server connection already exists.
      */
    //    virtual void new_ksircprocess(QString);
    virtual void new_ksircprocess(KSircServer &);
    /**
      * Creates popup asking for new channel name
      */
    virtual void new_channel();
    /**
      *  Args:
      *    str: name of the new channel to be created
      *    server: name of the server channel is created on
      *  Action:
      *     opens a new toplevel on the requested channel and server
      */
    virtual void new_toplevel(const KSircChannel &channel);
    virtual void new_toplevel(const KSircChannel &channel, bool safe);
    /**
      * Action:
      *     Notify all ksircprocess' to update filters
      */
    virtual void slot_filters_update();
    virtual void ToggleAutoCreate();

    /**
     * Action: Popup a general preferences window which allows various
     * settings, etc.
     */
    virtual void general_prefs();
    /**
     * Opens the dialog that lets the user configure system notifications
     */
    virtual void notification_prefs();
    virtual void font_update(const QFont&);
    virtual void filter_rule_editor();
    virtual void configChange();

    virtual void ProcMessage(QString server, int command, QString args);
    /**
     * On quit we sync the config to disk and exit
     */
    virtual void endksirc();
    /**
     * Start auto-connect
     */
    void start_autoconnect();
    /**
     * Start auto-connect with check
     */
    void start_autoconnect_check();


    QListViewItem * findChild( QListViewItem *parent, const QString& text );

protected slots:
  void WindowSelected(QListViewItem *);

  void dump_obj();
  void server_debug();

protected:

  virtual void showEvent( QShowEvent *e );
  virtual void hideEvent( QHideEvent *e );
  virtual void closeEvent( QCloseEvent * );
  void saveDockingStatus();

  void saveGlobalProperties(KConfig *);
  void readGlobalProperties(KConfig *);

private:
    void saveSessionConfig();

    // La raison d'etre.  We don't run ConnectionTree ourselves, but
    // we get it from our helper class scInside.
    KListView *ConnectionTree;

    scInside *sci;

    // Menubar for the top.
    KMenuBar *MenuBar;

    // Hold a list of all KSircProcess's for access latter.  Index by server
    // name
    QDict<KSircProcess> proc_list;
    QPopupMenu *options, *connections;
    int join_id, server_id;

    KGlobalAccel *m_kga;

    int open_toplevels;

    QPixmap pic_icon;
    QPixmap pic_server;
    QPixmap pic_gf;
    QPixmap pic_run;
    QPixmap pic_ppl;

//    PukeController *PukeC;

    // Holds dockable widget
    dockServerController *dockWidget;
    bool we_are_exiting;

    // Docked icon notification
    int m_notificationCount;
    struct ChannelSessionInfo
    {
	ChannelSessionInfo()
	    : desktop( -1 ) {}
	QString name;
        QString port;
	int desktop;
    };
    typedef QValueList<ChannelSessionInfo> ChannelSessionInfoList;

    typedef QMap<QString, ChannelSessionInfoList> SessionConfigMap;
    SessionConfigMap m_sessionConfig;

    static servercontroller *s_self;

    QTimer *at;

    nickColourMaker *m_ncm;
};
#endif // servercontroller_included
