#ifndef KBASERULES_H
#define KBASERULES_H

#include "messageReceiver.h"

class KSMBaseRules : public KSircMessageReceiver
{
public:
  KSMBaseRules(KSircProcess *proc)
    : KSircMessageReceiver(proc)
    {
      broadcast = FALSE;
    }
  virtual ~KSMBaseRules()
    {
    }

  virtual void sirc_receive(QCString str, bool broadcast);

  virtual void control_message(int, QString);

  filterRuleList *defaultRules();

private:
  bool broadcast;
  char to[101];
  char match_us[101];
  char to_us[101];
  char msg1_match[101];
  char msg1_to[101];
  char msg2_match[101];
  char msg2_to[101];
};

#endif
