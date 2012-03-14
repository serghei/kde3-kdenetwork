#ifndef PFRAME_H
#define PFRAME_H

class PFrame;

#include <qframe.h>
#include "pmessage.h"
#include "pwidget.h"

class PFrame : public PWidget
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PFrame ( PObject * parent );
  virtual ~PFrame ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);

  virtual void setWidget(QObject *w = 0x0);
  virtual QFrame *widget();

private:
  QFrame *frame;
};

#endif
