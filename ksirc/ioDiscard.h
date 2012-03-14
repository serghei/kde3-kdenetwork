#ifndef KIODISCARD_H
#define KIODISCARD_H

#include "messageReceiver.h"

class KSircIODiscard : public KSircMessageReceiver
{
public:
  KSircIODiscard(KSircProcess *_proc) : KSircMessageReceiver(_proc)
    {
    }
  virtual ~KSircIODiscard();

  virtual void sirc_receive(QCString, bool broadcast);
  virtual void control_message(int, QString);

private:
};

#endif
