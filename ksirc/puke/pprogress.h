#ifndef PPROGRESS_H
#define PPROGRESS_H

class PProgress;

#include "../KSProgress/ksprogress.h"
#include "pmessage.h"
//#include "pwidget.h"
#include "pobject.h"
#include "controller.h"

class PProgress : public PWidget {
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PProgress ( PObject * parent = 0);
  virtual ~PProgress ();

  virtual void messageHandler(int fd, PukeMessage *pm);

  virtual void setWidget(QObject *_f);
  virtual KSProgress *widget();

protected slots:
  void cancelPressed();

private:
  KSProgress *ksp;
};

#endif
