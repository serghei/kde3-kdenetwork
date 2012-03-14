#ifndef IOCONTROL_H
#define IOCONTROL_H

#include <qobject.h>

class KProcess;
class KSircProcess;
class QListBox;
class QListBoxItem;

class KSircIOController : public QObject
{

  Q_OBJECT
  friend class KSircProcess;
public:
  KSircIOController(KProcess *, KSircProcess *);
  virtual ~KSircIOController();

  void showDebugTraffic(bool show);
  bool isDebugTraffic();

public slots:
  virtual void stdout_read(KProcess *proc, char *_buffer, int buflen);
  virtual void stderr_read(KProcess *proc, char *_buffer, int buflen);

  virtual void stdin_write(QCString);

  virtual void sircDied(KProcess *);

  virtual void appendDebug(QString);


protected slots:
  virtual void procCTS(KProcess *);

private slots:
  void showContextMenuOnDebugWindow( QListBoxItem *, const QPoint &pos );

private:
  QCString holder;
  bool proc_CTS;
  KProcess *proc;
  KSircProcess *ksircproc;
  char *send_buf;

  QCString buffer;

  static int counter;

  QListBox *m_debugLB;
};

#endif
