#include <stdio.h>


#include "ptabdialog.h"
#include "commands.h"
#include <iostream>

PObject *
PTabDialog::createWidget(CreateArgs &ca)
{
  PTabDialog *ptd = new PTabDialog(ca.parent);
  QTabDialog *qtd;
  // Retreive the border and direction information out of the
  // carg string
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("QTabDialog") == TRUE){
    qtd = (QTabDialog *) ca.fetchedObj;
    ptd->setDeleteAble(FALSE);
  }
  else if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    qtd = new QTabDialog((QWidget *) ca.parent->widget());
  else
    qtd = new QTabDialog();
  ptd->setWidget(qtd);
  ptd->setWidgetId(ca.pwI);
  ptd->setPukeController(ca.pc);
  return ptd;
}

PTabDialog::PTabDialog(QObject *)
  : PWidget()
{
    // Connect slots as needed
  setWidget(0);
}

PTabDialog::~PTabDialog()
{
  //  kdDebug(5008) << "PObject: in destructor" << endl;
  /*
  delete widget();
  tab = 0;
  setWidget(0);
  */
}

void PTabDialog::messageHandler(int fd, PukeMessage *pm)
{
  PukeMessage pmRet;

  if(pm->iCommand == PUKE_TABDIALOG_ADDTAB){
    if(!(pm->iTextSize > 0)){
      qWarning("QTabDialog/addtab: incorrent cArg size, bailing out.  Needed: >0 got: %d\n", pm->iTextSize);
      pmRet.iCommand = PUKE_TABDIALOG_ADDTAB_ACK; // ack the add widget
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = 1;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      return;
    }
    widgetId wiWidget;
    wiWidget.fd = fd;
    wiWidget.iWinId = pm->iArg;
    PWidget *pw = controller()->id2pwidget(&wiWidget);
    //    kdDebug(5008) << "Adding widget with stretch: " << (int) pm->cArg[0] << " and align: " << //	  (int) pm->cArg[1] << endl;
    widget()->addTab(pw->widget(), pm->cArg);

    pmRet.iCommand = PUKE_TABDIALOG_ADDTAB_ACK; // ack the add widget
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
  }
  else {
    PWidget::messageHandler(fd, pm);
  }

}

void PTabDialog::setWidget(QObject *tb)
{
  if(tb != 0 && tb->inherits("QTabDialog") == FALSE)
  {
     errorInvalidSet(tb);
     return;
  }

  tab = (QTabDialog *) tb;
  if(tab != 0){
  }
  PObject::setWidget(tb);
  
}

QTabDialog *PTabDialog::widget()
{
  //  kdDebug(5008) << "PObject widget called" << endl;
  return tab;
}

#include "ptabdialog.moc"

