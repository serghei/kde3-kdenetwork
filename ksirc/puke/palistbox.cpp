#include <kdebug.h>


#include "palistbox.h"

PObject *
PAListBox::createWidget(CreateArgs &ca) 
{
  PAListBox *plb = new PAListBox(ca.parent);
  aListBox *lb;
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("aListBox") == TRUE){
    lb = (aListBox *) ca.fetchedObj;
    plb->setDeleteAble(FALSE);
  }
  else if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    lb = new aListBox((QWidget *) ca.parent->widget());
  else
    lb = new aListBox();
  plb->setWidget(lb);
  plb->setWidgetId(ca.pwI);
  return plb;
}


PAListBox::PAListBox(PObject *parent) 
  : PListBox(parent)
{
  //  kdDebug(5008) << "PListBox PListBox called" << endl;
  lb = 0;
  setWidget(lb);
}

PAListBox::~PAListBox() 
{
  //  kdDebug(5008) << "PListBox: in destructor" << endl;
  /*
  delete widget();     // Delete the frame
  lb=0;          // Set it to 0
  setWidget(lb); // Now set all widget() calls to 0.
  */
}

void PAListBox::messageHandler(int fd, PukeMessage *pm) 
{
  PukeMessage pmRet;
  switch(pm->iCommand){
  case PUKE_LISTBOX_INSERT_SORT:
    if(!checkWidget())
      return;

    widget()->inSort(pm->cArg, (bool) pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->count();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_ALISTBOX_ISTOP:
    if(!checkWidget())
      return;

    pmRet.iArg = widget()->isTop(pm->iArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_ALISTBOX_FIND_NICK:
    if(!checkWidget())
      return;

    pmRet.iArg = widget()->findNick(pm->cArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_ALISTBOX_SMALL_HIGHLIGHT:
    {
      if(!checkWidget())
	return;

      int index = widget()->findNick(pm->cArg);
      nickListItem *item = new nickListItem();
      *item = *widget()->item(index);
      widget()->removeItem(index);
      item->setVoice(pm->iArg);
      widget()->inSort(item);
      widget()->repaint(TRUE);

      pmRet.iCommand = - pm->iCommand;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = 0;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      break;
    }
  case PUKE_ALISTBOX_BIG_HIGHLIGHT:
    {
      if(!checkWidget())
	return;

      int index = widget()->findNick(pm->cArg);
      nickListItem *item = new nickListItem();
      *item = *widget()->item(index);
      widget()->removeItem(index);
      item->setOp(pm->iArg);
      widget()->inSort(item);
      widget()->repaint(TRUE);

      pmRet.iCommand = - pm->iCommand;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = 0;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      break;
    }

  default:
    PListBox::messageHandler(fd, pm);
  }
}

void PAListBox::setWidget(QObject *_lb) 
{
  if(_lb != 0 && _lb->inherits("aListBox") == FALSE)
  {
    errorInvalidSet(_lb);
    return;
  }

  lb = (aListBox *) _lb;
  if(lb != 0){
  }
  PListBox::setWidget(lb);

}


aListBox *PAListBox::widget() 
{
  return lb;
}

bool PAListBox::checkWidget(){ 
  if(widget() == 0){
    kdDebug(5008) << "PAListBox: No Widget set" << endl;
    return FALSE;
  }
  return TRUE;
}

#include "palistbox.moc"

