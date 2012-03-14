#ifndef KIOBROADCAST_H
#define KIOBROADCAST_H

#include "messageReceiver.h"

class KSircIOBroadcast : public KSircMessageReceiver
{
public:
  KSircIOBroadcast(KSircProcess *_proc) : KSircMessageReceiver(_proc)
    {
      proc = _proc;
      setBroadcast(FALSE);
    }
  virtual ~KSircIOBroadcast();

  virtual void sirc_receive(QCString str, bool broadcast);
  virtual void control_message(int, QString);

  virtual filterRuleList *defaultRules();

private:
  KSircProcess *proc;
};

#endif
