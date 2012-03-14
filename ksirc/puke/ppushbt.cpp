#include "ppushbt.h"
#include <stdio.h>


PObject *
PPushButton::createWidget(CreateArgs &ca)
{
  PPushButton *pb = new PPushButton(ca.parent);
  QPushButton *qb;
  if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    qb = new QPushButton((QWidget *) ca.parent->widget());
  else
    qb = new QPushButton(0L);
  pb->setWidget(qb);
  pb->setWidgetId(ca.pwI);
  return pb;
}


PPushButton::PPushButton(PObject *parent)
  : PButton(parent)
{
  //  kdDebug(5008) << "PLineEdit PLineEdit called" << endl;
  button = 0;
  setWidget(button);
}

PPushButton::~PPushButton()
{
  //  kdDebug(5008) << "PLineEdit: in destructor" << endl;
/*  delete widget();     // Delete the frame
  button=0;          // Set it to 0
  setWidget(button); // Now set all widget() calls to 0.
  */
}

void PPushButton::messageHandler(int fd, PukeMessage *pm)
{
//  PukeMessage pmRet;
  switch(pm->iCommand){
  default:
    PButton::messageHandler(fd, pm);
  }
}

void PPushButton::setWidget(QObject *_qb)
{
  if(_qb != 0 && _qb->inherits("QPushButton") == FALSE)
  {
     errorInvalidSet(_qb);
     return;
  }

  button = (QPushButton *) _qb;
  if(_qb != 0){
  }
  PButton::setWidget(_qb);

}


QPushButton *PPushButton::widget()
{
  return button;
}



#include "ppushbt.moc"

