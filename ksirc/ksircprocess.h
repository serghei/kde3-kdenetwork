#ifndef KSIRCPROCESS
#define KSIRCPROCESS

class KSircProcess;
class KSircTopLevel;

#include <qdict.h>
#include <qobject.h>

#include "iocontroller.h"
#include "ksircserver.h"
#include "ksircchannel.h"

class KProcess;
class KSircIOController;
class KSircMessageReceiver;
class KSircIOController;

class KSircProcess : public QObject
{
  Q_OBJECT
  friend class KSircIOController;
public:
  KSircProcess(QString &server_id, KSircServer &kss, QObject * parent=0, const char * name=0);
  virtual ~KSircProcess();

  QDict<KSircMessageReceiver> &getWindowList()
    {
      return TopList;
    }

  QPtrList<KSircMessageReceiver> messageReceivers() const;
  const QDict<KSircMessageReceiver> &mrList() const;

  QString serverName() const { return m_kss.server(); }
  QString serverPort() const { return m_kss.port(); }
  QString serverID() const { return m_serverid; }

  KSircIOController *getIOController() { return iocontrol; }

  const QString getNick() const;
  void setNick(const QString nick);


signals:
  void ProcMessage(QString, int, QString);

public slots:
 virtual void ServMessage(QString server, int command, QString args);
 virtual void close_toplevel(KSircTopLevel *, QString);
 virtual void default_window(KSircTopLevel *);
 virtual void recvChangeChannel(const QString &, const QString &);
 virtual void new_toplevel(const KSircChannel &, bool safe = false);

 virtual void filters_update();

 virtual void notify_forw_online(QString);
 virtual void notify_forw_offline(QString);

protected slots:
  virtual void clean_toplevel(QObject *clean);
  virtual void request_quit( const QCString& command );
  virtual void do_quit();

  virtual void turn_on_autocreate();


private:
  void base_filters();
  void cleanup();

  KSircIOController *iocontrol;
  KProcess *proc;

  QDict<KSircMessageReceiver> TopList;

  bool running_window;
  bool default_follow_focus;
  bool auto_create_really;

  KSircServer m_kss;
  const QString m_serverid;

  /*
   * m_nick is our nick as reported by the lower levels
   */
  QString m_nick;

//  char *server;
};

#endif
