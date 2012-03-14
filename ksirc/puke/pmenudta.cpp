#include "pmenudta.h"
#include "pobject.h"
#include <qpopupmenu.h>


PMenuData::PMenuData(PObject *_child)
: PObject()
{
  child = _child;
  connect(this, SIGNAL(outputMessage(int, PukeMessage*)),
          child, SIGNAL(outputMessage(int, PukeMessage*)));
}

PMenuData::~PMenuData()
{
  // We don't nuke anything since we're kind of abstract and we let the parent take care of it
}

void PMenuData::messageHandler(int , PukeMessage *)
{
  // We don't do anything since we should never ben involked directly
}

bool PMenuData::menuMessageHandler(int fd, PukeMessage *pm)
{
    PukeMessage pmRet;
    int id = 0;
  switch(pm->iCommand){
  case PUKE_MENUDATA_INSERT_TEXT:
    {
      QPopupMenu *widget = (QPopupMenu *) child->widget();
      id = widget->insertItem(pm->cArg);
      if(pm->iArg > 0){
          widget->setAccel(pm->iArg, id);
      }

      pmRet.iCommand = PUKE_MENUDATA_INSERT_TEXT_ACK;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = id;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      break;
    }
  case PUKE_MENUDATA_INSERT_PIXMAP:
    {
      QPopupMenu *widget = (QPopupMenu *) child->widget();
      id = widget->insertItem(QPixmap(pm->cArg));
      if(pm->iArg > 0){
          widget->setAccel(pm->iArg, id);
      }
    
      pmRet.iCommand = PUKE_MENUDATA_INSERT_PIXMAP_ACK;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = id;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      break;
    }
  case PUKE_MENUDATA_REMOVE_ITEM:
    {
      QPopupMenu *widget = (QPopupMenu *) child->widget();
      widget->removeItem(pm->iArg);
    
      pmRet.iCommand = PUKE_MENUDATA_REMOVE_ITEM_ACK;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = pm->iArg;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      break;
    }

  default:
    return FALSE;
  }
  return TRUE;
}

#include "pmenudta.moc"

