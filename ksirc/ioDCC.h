#ifndef KIODCCC_H
#define KIODCCC_H

#include <qdict.h>
#include <qobject.h>
#include <qguardedptr.h>

#include "dccManager.h"
#include "dccToplevel.h"
#include "messageReceiver.h"

class KSProgress;

struct DCCInfo {
  QString nick;
  QString cancelMessage;
  int LastSize;
  int PercentSize;
  bool inProgress;
};

class KSircIODCC : public QObject,
		   public KSircMessageReceiver
{
  Q_OBJECT
public:
  KSircIODCC(KSircProcess *_proc);
  virtual ~KSircIODCC();

  virtual void sirc_receive(QCString, bool broadcast);
  virtual void control_message(int, QString);
  virtual filterRuleList *defaultRules();

  virtual void showMgr();

protected slots:
  void cancelTransfer(QString);
  void getFile();
  void forgetFile();

  void dccConnectClicked(dccItem *);
  void dccResumeClicked(dccItem *);
  void dccRenameClicked(dccItem *);
  void dccAbortClicked(dccItem *);

  void dccRenameDone(dccItem *, QString, QString);

signals:
  void outputLine(QCString);

private:
//  QDict<QProgressDialog> DlgList;
    //QDict<KSProgress> DlgList;
    //QDict<DCCInfo> DCCStatus;
  QDict<dccItem> DCCGetItems;
  QDict<dccItem> DCCSendItems;
  QDict<dccItem> DCCChatItems;

  //dccDialog *pending;
  QGuardedPtr<dccTopLevel> mgr;

  KSircProcess *proc;
};

#endif
