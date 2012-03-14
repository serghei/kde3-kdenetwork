#include <stdio.h>


#include <kdebug.h>

#include "pkfiledialog.h"
#include "pkfiledialog-cmd.h"

PObject *
PKFileDialog::createWidget(CreateArgs &ca)
{
  PKFileDialog *pw = new PKFileDialog(ca.parent);
  KFileDialog *kfbd;
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("KFileDialog") == TRUE){
    kfbd = (KFileDialog *) ca.fetchedObj;
    pw->setDeleteAble(FALSE);
  }
  else // Never takes a parent in Puke
    kfbd = new KFileDialog("/", QString::null, 0L, "PukeKFileDialog", TRUE);
  pw->setWidget(kfbd);
  pw->setWidgetId(ca.pwI);
  return pw;
}


PKFileDialog::PKFileDialog(PObject *parent)
  : PWidget(parent)
{
  kfbd = 0;
  setWidget(kfbd);
}

PKFileDialog::~PKFileDialog()
{
  //  kdDebug(5008) << "PLineEdit: in destructor" << endl;
  /*
  delete widget();     // Delete the frame
  kfbd=0;          // Set it to 0
  setWidget(kfbd); // Now set all widget() calls to 0.
  */
}

void PKFileDialog::messageHandler(int fd, PukeMessage *pm)
{
  QString selFile;
  PukeMessage pmRet;
  if(widget() == 0){
    kdDebug(5008) << "PKFileDialog: No Widget set" << endl;
    return;
  }
  switch(pm->iCommand){
  case PUKE_KBFD_SET_PATH:
    ((KFileDialog*)widget())->setURL(KURL(pm->cArg));
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.iTextSize = widget()->baseURL().path().length();
#warning check if the cast is okay
    pmRet.cArg = (char*) widget()->baseURL().path().ascii();
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_KBFD_SET_FILTER:
    widget()->setFilter(pm->cArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.iTextSize = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_KBFD_SET_SELECTION:
    widget()->setSelection(pm->cArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    selFile = widget()->selectedURL().path();
    pmRet.iTextSize = selFile.length();
#warning check if the cast is okay
    pmRet.cArg = (char*) selFile.ascii();
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_WIDGET_SHOW:
    widget()->exec();
    pmRet.iCommand = PUKE_KBFD_FILE_SELECTED_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = new char[selFile.length()];
    selFile = widget()->selectedURL().path();
    // #### HPB: using strlen() 'cause we want the length of the .ascii()
    //           string. We should probably replace in the future.
    memcpy(pmRet.cArg, selFile.ascii(), strlen(selFile.ascii()));
    pmRet.iTextSize = selFile.length();
    emit outputMessage(widgetIden().fd, &pmRet);
    delete pmRet.cArg;
    break;

  default:
    PWidget::messageHandler(fd, pm);
  }
}

void PKFileDialog::setWidget(QObject *_kbfd)
{
  if(_kbfd != 0 && _kbfd->inherits("KFileBaseDialog") == FALSE)
  {
    errorInvalidSet(_kbfd);
    return;
  }

  kfbd = (KFileDialog *) _kbfd;
  PWidget::setWidget(kfbd);
}


KFileDialog *PKFileDialog::widget()
{
  return kfbd;
}

#include "pkfiledialog.moc"
