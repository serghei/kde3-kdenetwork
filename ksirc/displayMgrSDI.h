#ifndef DISPLAYMGRSDI_H
#define DISPLAYMGRSDI_H

#include "displayMgr.h"

class DisplayMgrSDI : public DisplayMgr {
public:
  DisplayMgrSDI();
  virtual ~DisplayMgrSDI();

  virtual void newTopLevel(QWidget *, bool show = FALSE);
  virtual void removeTopLevel(QWidget *);

  virtual void show(QWidget *);
  virtual void hide(QWidget *);
  virtual void raise(QWidget *, bool takefocus = false);

  virtual void setCaption(QWidget *, const QString&);

private:
};


#endif

