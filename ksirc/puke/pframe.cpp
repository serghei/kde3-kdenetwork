#include "pframe.h"


PObject *
PFrame::createWidget(CreateArgs &ca)
{
  PFrame *pw = new PFrame(ca.parent);
  QFrame *tf;
  if(ca.parent != 0 &&
     ca.parent->widget()->isWidgetType() == TRUE)
    tf = new QFrame((QWidget *) ca.parent->widget());
  else
    tf = new QFrame();
  pw->setWidget(tf);
  pw->setWidgetId(ca.pwI);
  pw->setPukeController(ca.pc);  
  return pw;
}


PFrame::PFrame( PObject *parent)
  : PWidget(parent)
{
  //  kdDebug(5008) << "PFrame PFrame called" << endl;
  frame = 0;
}

PFrame::~PFrame()
{
  //  kdDebug(5008) << "PFrame: in destructor" << endl;
  /*
  delete frame; // Delete the frame
  frame=0;      // Set it to 0
  setWidget(frame); // Now set all widget() calls to 0.
  */
}

void PFrame::messageHandler(int fd, PukeMessage *pm)
{
  //  kdDebug(5008) << "PFrame handler called" << endl;
  PukeMessage pmRet;
  switch(pm->iCommand){
  case PUKE_QFRAME_SET_FRAME:
    widget()->setFrameStyle(pm->iArg);
    pmRet.iCommand = PUKE_QFRAME_SET_FRAME_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->frameStyle();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_QFRAME_SET_LINEWIDTH:
    widget()->setLineWidth(pm->iArg);
    pmRet.iCommand = PUKE_QFRAME_SET_LINEWIDTH_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->lineWidth();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  default:
    PWidget::messageHandler(fd, pm);
  }
}

void PFrame::setWidget(QObject *w)
{
  if(w != 0 && w->inherits("QFrame") == FALSE)
  {
    errorInvalidSet(w);
    return;
  }

  frame = (QFrame *) w;
  PWidget::setWidget((QWidget *) w);
}


QFrame *PFrame::widget()
{
  //  kdDebug(5008) << "PFrame widget called" << endl;
  return frame;
}

#include "pframe.moc"

