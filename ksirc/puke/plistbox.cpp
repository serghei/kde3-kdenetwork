#include <kdebug.h>


#include "plistbox.h"

PObject *
PListBox::createWidget(CreateArgs &ca) 
{
  PListBox *plb = new PListBox(ca.parent);
  QListBox *lb;
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("QListBox") == TRUE){
    lb = (QListBox *) ca.fetchedObj;
    plb->setDeleteAble(FALSE);
  }
  else if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    lb = new QListBox((QWidget *) ca.parent->widget());
  else
    lb = new QListBox();
  plb->setWidget(lb);
  plb->setWidgetId(ca.pwI);
  return plb;
}


PListBox::PListBox(PObject *parent) 
  : PFrame(parent)
{
  //  kdDebug(5008) << "PListBox PListBox called" << endl;
  lb = 0;
  setWidget(lb);
}

PListBox::~PListBox() 
{
  //  kdDebug(5008) << "PListBox: in destructor" << endl;
  /*
  delete widget();     // Delete the frame
  lb=0;          // Set it to 0
  setWidget(lb); // Now set all widget() calls to 0.
  */
}

void PListBox::messageHandler(int fd, PukeMessage *pm) 
{
  PukeMessage pmRet;
  switch(pm->iCommand){
  case PUKE_LISTBOX_INSERT:
    if(!checkWidget())
      return;
    
    widget()->insertItem(pm->cArg, pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->count();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_INSERT_SORT:
    if(!checkWidget())
      return;
    
    widget()->insertItem(pm->cArg);
    widget()->sort();
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->count();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_INSERT_PIXMAP:
    if(!checkWidget())
      return;
    
    widget()->insertItem(QPixmap(pm->cArg), pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->count();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_HIGHLIGHT:
    if(!checkWidget())
      return;
    
    widget()->setCurrentItem(pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->currentItem();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_REMOVE:
    if(!checkWidget())
      return;
    
    widget()->removeItem(pm->iArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_GETTEXT:
    if(!checkWidget())
       return;
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    if(widget()->text(pm->iArg) != 0x0){
      pmRet.iArg = 1;
      pmRet.iTextSize = strlen(widget()->text(pm->iArg));
      pmRet.cArg = new char[strlen(widget()->text(pm->iArg))+1];
      strcpy(pmRet.cArg, widget()->text(pm->iArg));
      emit outputMessage(fd, &pmRet);
      delete[] pmRet.cArg;
    }
    else{
      pmRet.iArg = 0;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
    }
    break;
  case PUKE_LISTBOX_SET_SCROLLBAR:
    widget()->setVScrollBarMode( (bool) pm->iArg ? QListBox::AlwaysOn : QListBox::AlwaysOff );
    pmRet.iCommand = PUKE_LISTBOX_SET_SCROLLBAR_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_SET_AUTO_SCROLLBAR:
    widget()->setVScrollBarMode( (bool) pm->iArg ? QListBox::Auto : QListBox::AlwaysOff );
    pmRet.iCommand = PUKE_LISTBOX_SET_AUTO_SCROLLBAR_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_CLEAR:
    widget()->clear();
    pmRet.iCommand = PUKE_LISTBOX_CLEAR_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  default:
    PFrame::messageHandler(fd, pm);
  }
}

void PListBox::setWidget(QObject *_lb) 
{
  if(_lb != 0 && _lb->inherits("QListBox") == FALSE)
  {
    errorInvalidSet(_lb);
    return;
  }

  lb = (QListBox *) _lb;
  if(lb != 0){
    connect(lb, SIGNAL(highlighted(int)),
            this, SLOT(highlighted(int)));
    connect(lb, SIGNAL(selected(int)),
            this, SLOT(selected(int)));
  }
  PFrame::setWidget(lb);

}


QListBox *PListBox::widget() 
{
  return lb;
}


void PListBox::highlighted(int index) { 
  PukeMessage pmRet;

  kdDebug(5008) << "Got highlight" << endl;
  pmRet.iCommand = PUKE_LISTBOX_HIGHLIGHTED_ACK;
  pmRet.iWinId = widgetIden().iWinId;
  pmRet.iArg = index;
  if(widget()->text(index) != 0){
    pmRet.iTextSize = strlen(widget()->text(index));
    pmRet.cArg = new char[strlen(widget()->text(index)) + 1];
    strcpy(pmRet.cArg, widget()->text(index));
  }
  else
    pmRet.cArg = 0;
  emit outputMessage(widgetIden().fd, &pmRet);
  delete[] pmRet.cArg;
}

void PListBox::selected(int index) { 
  PukeMessage pmRet;

  kdDebug(5008) << "Got selected" << endl;
  pmRet.iCommand = PUKE_LISTBOX_SELECTED_ACK;
  pmRet.iWinId = widgetIden().iWinId;
  pmRet.iArg = index;
  if(widget()->text(index) != 0){
    pmRet.iTextSize = strlen(widget()->text(index));
    pmRet.cArg = new char[strlen(widget()->text(index)) + 1];
    strcpy(pmRet.cArg, widget()->text(index));
  }
  else
    pmRet.cArg = 0;
  emit outputMessage(widgetIden().fd, &pmRet);
  delete[] pmRet.cArg;
}

bool PListBox::checkWidget(){ 
  if(widget() == 0){
    kdDebug(5008) << "PListBox: No Widget set" << endl;
    return FALSE;
  }
  return TRUE;
}

#include "plistbox.moc"

