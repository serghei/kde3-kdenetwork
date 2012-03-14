
#ifndef dockservercontroller_included
#define dockservercontroller_included

#include <ksystemtray.h>
#include <qdict.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qstringlist.h>

class QPopupMenu;
class QWidget;
class servercontroller;
class QMouseEvent;
class KPopupMenu;
class QTimer;

class dscNickInfo
{
public:
    dscNickInfo(QString nick, QString server);
    ~dscNickInfo();

    enum status {
	isOnline,
        isOffline
    };

    QString nick() { return m_nick; }
    QString server() { return m_server; }

    void setOnline() { m_status = isOnline; m_online = QTime::currentTime(); }
    const QTime &online() { return m_online; }

    void setOffline() { m_status = isOffline; m_offline = QTime::currentTime(); }
    const QTime offline() { return m_offline; }

    void setStatus(enum status stat) { m_status = stat; }
    enum status status() { return m_status; }

private:
    QString m_nick;
    QString m_server;
    enum status m_status;
    QTime m_online;
    QTime m_offline;

};

class dockServerController : public KSystemTray
{
  Q_OBJECT
public:
  dockServerController(servercontroller *_sc, QWidget *parent = 0x0, const char *_name = 0x0);
  ~dockServerController();

  void startBlink(const QString& reason = QString::null, const QString& text = QString::null);
  void stopBlink(const QString& reason = QString::null, bool clear = false);

  void nickOnline(QString server, QString nick);
  void nickOffline(QString server, QString nick);

  void serverOpen(QString server);
  void serverClose(QString server);

protected:
    void mousePressEvent( QMouseEvent *);
    virtual void showPopupMenu( QPopupMenu *);

public slots:
    void raiseLastActiveWindow();

protected slots:
    void subItemActivated(int);
    void blinkDockedIcon(void);
    void blinkClear(void);
    void mainActivated(int id);
    void helpNotice();
    void configNotify();

    void mainPopShow();
    void mainPopHide();

private:
    int intoPopupSorted(QString, QPopupMenu *);
    void createMainPopup();
    void raiseWindow(QString server, QString name);
    QDict<dscNickInfo> m_nicks;
    servercontroller *m_sc;
    KPopupMenu *mainPop;

    QString m_last_nick;
    QString m_last_server;

    QPixmap m_pic_dock;
    QPixmap m_pic_info;

    bool m_mainPopVisible;

    bool m_blinkActive;
    bool m_blinkStatus; // true: blue icon, false: normal icon
    QTimer* m_blinkTimer;

    QStringList m_blink_reason;

};

#endif

