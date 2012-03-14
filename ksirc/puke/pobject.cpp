#include "pobject.h"
#include "commands.h"


PObject::PObject(QObject *pobject, const char *name) 
  : QObject(pobject, name)
{
  // Connect slots as needed
  obj = 0;
  setWidget(0);
  manualTerm = FALSE;
  deleteAble = TRUE;
  m_hasError = false;
}

PObject::~PObject() 
{
  if(isDeleteAble())
    delete widget();
  obj = 0;
  setWidget(0);
}

PObject *PObject::createWidget(CreateArgs &ca) 
{
  PObject *pw = new PObject(ca.parent);
  QObject *o;
  if(ca.parent != 0)
    o = new QObject(ca.parent->widget());
  else
    o = new QObject();
  pw->setWidget(o);
  pw->setWidgetId(ca.pwI);
  pw->setPukeController(ca.pc);
  return pw;
}

void PObject::messageHandler(int fd, PukeMessage *pm) 
{
  PukeMessage pmRet;
  if(pm->iCommand == PUKE_WIDGET_DELETE){
    /**
     * Emit the ack before the delete since we don't exist afterwards.
     */
    pmRet.iCommand = PUKE_WIDGET_DELETE_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);

    manTerm();
    delete this;
  }
  if(pm->iCommand == PUKE_RELEASEWIDGET){
    /**
     * Emit the ack before the delete since we don't exist afterwards.
     */
    pmRet.iCommand = PUKE_RELEASEWIDGET_ACK;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);

    /**
     * By setting the widget to 0 we loose the pointer and then don't delete it
     */
    setWidget(0);
    manTerm();
    delete this;
  }
  else {
    qWarning("PObject: Unkown Command: %d", pm->iCommand);
    pmRet.iCommand = PUKE_INVALID;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
  }
}

void PObject::setWidget(QObject *_o) 
{
  // Disconnect everything from the object we where listning too
  // Just in case it fires something off, we don't want to get it
  if(widget() != 0){
    disconnect(widget(), SIGNAL(destroyed()),
               this, SLOT(swidgetDestroyed()));
  }
  
  obj = _o;
  if(obj != 0){
    connect(widget(), SIGNAL(destroyed()),
	    this, SLOT(swidgetDestroyed()));
  }
}

QObject *PObject::widget() 
{
  //  kdDebug(5008) << "PObject widget called" << endl;
  return obj;
}

void PObject::setWidgetId(widgetId *pwI) 
{
  wI = *pwI;
  //  kdDebug(5008) << "PObject: set widget id " << wI.iWinId << endl;
}

widgetId PObject::widgetIden() 
{
  //  kdDebug(5008) << "PObject: called widget id " << wI.iWinId << endl;
  return wI;
}

void PObject::swidgetDestroyed(){ 
  setWidget(0x0);
  if(manualTerm == FALSE){
    manTerm();
    delete this;
  }
}

PukeController *PObject::controller() { 
  
  return pController;
}

void PObject::manTerm() {
  manualTerm = TRUE;
}

void PObject::errorInvalidSet(QObject *_w)
{ 
  m_error = QString("Tried setting a %1 to %2").arg(_w->className()).arg(className());
  m_hasError = true;
}
#include "pobject.moc"

