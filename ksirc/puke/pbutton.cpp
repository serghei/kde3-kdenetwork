#include <stdio.h>


#include <qpixmap.h>

#include <kdebug.h>

#include "pbutton.h"

PObject *
PButton::createWidget(CreateArgs &ca)
{
  PButton *pb = new PButton(ca.parent);
  QButton *qb;
  if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    qb = new QButton((QWidget *) ca.parent->widget());
  else
    qb = new QButton();
  pb->setWidget(qb);
  pb->setWidgetId(ca.pwI);
  return pb;
}


PButton::PButton(PObject *parent)
  : PWidget(parent)
{
  //  kdDebug(5008) << "PLineEdit PLineEdit called" << endl;
  button = 0;
  setWidget(0);
}

PButton::~PButton()
{
  //  kdDebug(5008) << "PLineEdit: in destructor" << endl;
  /*
  delete widget();     // Delete the frame
  button=0;          // Set it to 0
  setWidget(button); // Now set all widget() calls to 0.
  */
}

void PButton::messageHandler(int fd, PukeMessage *pm)
{
  PukeMessage pmRet;
  switch(pm->iCommand){
    case PUKE_BUTTON_SET_TEXT:
    if(checkWidget() == FALSE)
      return;

    widget()->setText(pm->cArg);    // set the text

    pmRet.iCommand = - pm->iCommand;// Create ack
    pmRet.iWinId = pm->iWinId;
    pmRet.cArg = (char*) widget()->text().ascii(); // It's const, but we don't mess with it anyways
    pmRet.iTextSize = strlen(pmRet.cArg);
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_BUTTON_SET_PIXMAP:
    if(checkWidget() == FALSE)
      return;
    
    widget()->setPixmap(QPixmap(pm->cArg));

    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->pixmap()->isNull();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_BUTTON_SET_AUTORESIZE:
    if(checkWidget() == FALSE)
      return;

    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = - pm->iWinId;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  default:
    PWidget::messageHandler(fd, pm);
  }
}

void PButton::setWidget(QObject *_qb)
{
  if(_qb != 0 && _qb->inherits("QButton") == FALSE)
  {
    errorInvalidSet(_qb);
    return;
  }

  button = (QButton *) _qb;
  if(button != 0){
    connect(button, SIGNAL(pressed()),
	    this, SLOT(buttonPressed()));
    connect(button, SIGNAL(released()),
	    this, SLOT(buttonReleased()));
    connect(button, SIGNAL(clicked()),
	    this, SLOT(buttonClicked()));
    connect(button, SIGNAL(toggled(bool)),
	    this, SLOT(buttonToggled(bool)));
    
  }
  PWidget::setWidget(_qb);

}


QButton *PButton::widget()
{
  return button;
}

void PButton::buttonMessage(int iCommand)
{
  PukeMessage pmRet;

  pmRet.iCommand = iCommand;
  pmRet.iArg = 0;
  pmRet.iWinId = widgetIden().iWinId;
  pmRet.cArg = 0;

  emit outputMessage(widgetIden().fd, &pmRet);
}

void PButton::buttonPressed()
{
  buttonMessage(PUKE_BUTTON_PRESSED_ACK);
}

void PButton::buttonReleased()
{
  buttonMessage(PUKE_BUTTON_RELEASED_ACK);
}

void PButton::buttonClicked()
{
  buttonMessage(PUKE_BUTTON_CLICKED_ACK);
}

void PButton::buttonToggled(bool)
{
  buttonMessage(PUKE_BUTTON_TOGGLED_ACK);
}

bool PButton::checkWidget()
{
  if(widget() == 0){
    kdDebug(5008) << "PButton: No Widget set" << endl;
    return FALSE;
  }
  return TRUE;
}

#include "pbutton.moc"

