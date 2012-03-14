#ifndef PPUSHBUTTON_H
#define PPUSHBUTTON_H

class PPushButton;

#include <qpushbutton.h>
#include "pmessage.h"
#include "pbutton.h"
#include "pobject.h"
#include "controller.h"

class PPushButton : public PButton
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PPushButton ( PObject * parent );
  virtual ~PPushButton ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_b = 0);
  virtual QPushButton *widget();

public slots:
  
private:
  QPushButton *button;

};

#endif
