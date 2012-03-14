#ifndef POBJFINDER_H
#define POBJFINDER_H

#include "pobject.h"
#include "../objFinder.h"
#include "pmessage.h"
#include "controller.h"


class PObjFinder : public PObject
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PObjFinder ( PObject * parent );
  virtual ~PObjFinder ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_obj = 0);
  virtual objFinder *widget();

public slots:
  virtual void newObject(QObject *obj);

protected:
  bool checkWidget();

private:
};

#endif
