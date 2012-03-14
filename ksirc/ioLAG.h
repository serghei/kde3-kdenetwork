#ifndef KIOLAG_H
#define KIOLAG_H

#include <qobject.h>

#include "messageReceiver.h"

class KSircIOLAG : public QObject,
		   public KSircMessageReceiver
{
  Q_OBJECT
public:
  KSircIOLAG(KSircProcess *_proc);
  virtual ~KSircIOLAG();

  virtual void sirc_receive(QCString, bool broadcast);
  virtual void control_message(int, QString);

protected slots:

protected:
 virtual void timerEvent ( QTimerEvent * );

signals:
  void outputLine(QCString);

private:
  KSircProcess *proc;
};

#endif
