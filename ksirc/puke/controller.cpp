#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/stat.h>

using namespace std; // iostream.h include cstring which puts strlen into 
                     // std:: namespace, which breaks Qt's strlen() call 
                     // in qcstring.h (in gcc3's libstdc++)

#include <klibloader.h>
#include <kdebug.h>


#include "controller.h"

#include "../../config.h"
#include "../objFinder.h"

#include "palistbox.h"
#include "pbutton.h"
#include "pframe.h"
#include "pkfiledialog.h"
#include "plabel.h"
#include "playout.h"
#include "plined.h"
#include "plistbox.h"
#include "pmenudta.h"
#include "pmessage.h"
#include "pobject.h"
#include "pobjfinder.h"
#include "ppopmenu.h"
#include "pprogress.h"
#include "ppushbt.h"
#include "ptabdialog.h"
#include "ptablevw.h"
#include "pwidget.h"

#undef DEBUG

uint PukeController::uiBaseWinId = 10; // Gives a little seperation from the controller id

PukeController::PukeController(QString sock, QObject *parent, const char *name) : PObject( parent, name )
{
  int len, prev_umask;
  struct sockaddr_un unix_addr;

  running = FALSE; // Running has to be true before we do any work
  bClosing = FALSE; // we're not trying to close so set this false.

  // Set the umask to something sane that doesn't allow others to take over ksirc
  prev_umask = umask(0177);

  if(sock.length() == 0){
    qsPukeSocket = getenv("HOME");
    if(qsPukeSocket.length() == 0){
      qsPukeSocket = "/tmp";
    }
    qsPukeSocket += "/.ksirc.socket";
  }
  else{
    qsPukeSocket = sock;
  }

  unlink(qsPukeSocket);
  iListenFd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(iListenFd < 0){
    perror("PUKE: Unix Domain Socket create failed");
    return;
  }
  memset(&unix_addr, 0, sizeof(unix_addr));
  unix_addr.sun_family = AF_UNIX;
  strlcpy(unix_addr.sun_path, qsPukeSocket, sizeof(unix_addr.sun_path));

  len = sizeof(unix_addr.sun_family) + qsPukeSocket.length();
#if defined(__FreeBSD__)
  if( bind(iListenFd, (struct sockaddr *) &unix_addr, len+1) < 0){
#else
  if( bind(iListenFd, (struct sockaddr *) &unix_addr, len) < 0){
#endif
    perror("PUKE: Could not bind to Unix Domain Socket");
    return;
  }

  if(listen(iListenFd, 5) < 0){
    perror("PUKE: Could not listen for inbound connections");
    return;
  }

  running = TRUE;

  fcntl(iListenFd, F_SETFL, O_NONBLOCK);  // Set it non-block so that
                                          // accept() never blocks.

  qsnListen = new QSocketNotifier(iListenFd, QSocketNotifier::Read, this, QString(name) + "_iListen");
  connect(qsnListen, SIGNAL(activated(int)),
	  this, SLOT(NewConnect(int)));

  connect(objFind, SIGNAL(inserted(QObject *)),
          this, SLOT(slotInserted(QObject *)));

  qidConnectFd.setAutoDelete(TRUE);

  qidCommandTable.setAutoDelete(TRUE);

  /*
   * Setup widget data trees
   */
  WidgetList.setAutoDelete(TRUE);
  revWidgetList.setAutoDelete(TRUE);
  widgetCF.setAutoDelete(TRUE);

  /*
   * Connect outputMessage to the acutal write buffer function
   * outputMessage signals from pobjects are chained until they finally reach us.
   */
  connect(this, SIGNAL(outputMessage(int, PukeMessage *)),
	  this,  SLOT(writeBuffer(int, PukeMessage *)));

  initHdlr(); // Setup message command handlers.

  // Set umask back so it doesn't affect dcc's and so forth.
  umask(prev_umask);

  /*
   * We are a PObject so do some init code
   */
  // We're always terminate by someone else so set manTerm() right now
  manTerm();
  setWidget(0x0);

}

void
PukeController::slotInserted(QObject *obj)
{
  emit inserted(obj);
}

PukeController::~PukeController() 
{
  close(iListenFd);
  disconnect(this); // We call disconnect this so don't listen to our own destroy() signal go out
  unlink(qsPukeSocket);
}

QStrList PukeController::allObjects()
{
  return objFinder::allObjects();
}

void PukeController::NewConnect(int) 
{
  int cfd;
  ksize_t len = 0;
  struct sockaddr_un unix_addr;

  cfd = accept(iListenFd, (struct sockaddr *)&unix_addr, &len);
  if(cfd < 0){
    perror("PUKE: NewConnect fired, but no new connect");
    return;
  }
  fcntl(cfd, F_SETFL, O_NONBLOCK);  // Set it non-block so that
                                    // cfd() never blocks.

  fdStatus *fds = new fdStatus();
  fds->sr = new QSocketNotifier(cfd, QSocketNotifier::Read, this);
  fds->sw = new QSocketNotifier(cfd, QSocketNotifier::Write, this);
  connect(fds->sr, SIGNAL(activated(int)),
	  this, SLOT(Traffic(int)));
  connect(fds->sw, SIGNAL(activated(int)),
	  this, SLOT(Writeable(int)));
  qidConnectFd.insert(cfd, fds);
  qsnListen->setEnabled(TRUE);

  /*
   * Now we add ourselves as a client to the fd so we can process messages going to us
   */
  WidgetS *ws = new WidgetS;
  ws->pwidget = this;
  ws->type = 1;
  insertPObject(cfd, ControllerWinId, ws);

}


void PukeController::Writeable(int fd) 
{
  if(qidConnectFd[fd]){
    qidConnectFd[fd]->writeable = TRUE;
    qidConnectFd[fd]->sw->setEnabled(FALSE);
    //
    // Insert buffer flushing code here.
    //
  }
  else{
    kdDebug(5008) << "PUKE: Unkonwn fd: " << fd << endl;
  }
}

void PukeController::writeBuffer(int fd, PukeMessage *message) 
{
  if(qidConnectFd[fd]){
    //    if(qidConnectFd[fd]->writeable == FALSE){
    //      kdDebug(5008) << "PUKE: Writing to FD that's not writeable: " << fd << endl;
      //    }
    if(message != 0){
      int bytes = 0;
      message->iHeader = iPukeHeader;
      if(message->iTextSize == 0 || message->cArg == 0){
        message->iTextSize = 0;
        message->cArg = 0;
#ifdef DEBUG
        printf("Traffic on: %d <= %d %d %d %d 0x%x\n",
               fd,
               message->iCommand,
               message->iWinId,
               message->iArg,
               message->iTextSize,
               message->cArg);
#endif
        bytes = write(fd, message, 5 * sizeof(int));
      }
      else{
        /*
        struct OutMessageS {
          unsigned int iHeader;
          int iCommand;
          int iWinId;
          int iArg;
          int iTextSize;
          char cArg[message->iTextSize];
        } OutMessage;
        OutMessage.iHeader = iPukeHeader;
        OutMessage.iCommand = message->iCommand;
        OutMessage.iWinId = message->iWinId;
        OutMessage.iArg = message->iArg;
        OutMessage.iTextSize = message->iTextSize;
        memcpy(OutMessage.cArg, message->cArg, OutMessage.iTextSize);
        //        OutMessage.cArg[OutMessage.iTextSize] = 0; // Don't need to null out the last character
        bytes = write(fd, &OutMessage, 5*sizeof(int) + (OutMessage.iTextSize) * sizeof(char));
        */
#ifdef DEBUG
        printf("Traffic on: %d <= %d %d %d %d 0x%x\n",
               fd,
               message->iCommand,
               message->iWinId,
               message->iArg,
               message->iTextSize,
               message->cArg);
#endif /* DEBUG */

        struct iovec iov[2];
        iov[0].iov_base = (char *) message;
        iov[0].iov_len = 5*sizeof(int);
        iov[1].iov_base = (char *) message->cArg;
        iov[1].iov_len = message->iTextSize;
        bytes = writev(fd, iov, 2);
      }
      //      kdDebug(5008) << "Wrote: " << bytes << endl;
      if(bytes <= 0){
	switch(errno){
	case EAGAIN: // Don't do anything for try again
	  break;
//	default:
//	  perror("Puke: write on socket failed");
	  // Don't call closefd() since deletes are called on write's
	  // since write is being called from the destructors, etc of
	  // the widgets.  (bad things happend when you call write
	  // then your return; path ceasaes to exist.
	  //	  closefd(fd);
	}
      }
    }
  }
  else{
    closefd(fd);
    kdDebug(5008) << "PUKE: Attempt to write to unkown fd:" << fd << endl;
  }
}

void PukeController::Traffic(int fd) 
{
  PukeMessage pm;
  int bytes = -1;
  memset(&pm, 0, sizeof(pm));
  while((bytes = read(fd, &pm, 5*sizeof(int))) > 0){
    if(bytes != 5*sizeof(int)){
      kdDebug(5008) << "Short message, Got: " << bytes << " Wanted: " << sizeof(PukeMessage) << " NULL Padded" << endl;
    }
#ifdef DEBUG
    printf("Traffic on: %d => %d %d %d %d",
           fd,
	   pm.iCommand,
	   pm.iWinId,
	   pm.iArg,
           pm.iTextSize);
    if(pm.iCommand == 0x0){
      abort();
    }
#endif /* DEBUG */
    if(pm.iHeader != iPukeHeader){
      qWarning("Invalid packet received, discarding!");
      return;
    }
    if(pm.iTextSize > 0){
      pm.cArg = new char[pm.iTextSize + 1];
      read(fd, pm.cArg, pm.iTextSize * sizeof(char));
      pm.cArg[pm.iTextSize] = 0x0; // Null terminate the string.
//      printf(" %s\n", pm.cArg);
    }
    else {
        pm.cArg = 0;
//        printf("\n");
    }
    MessageDispatch(fd, &pm);
    delete[] pm.cArg; // Free up cArg is used
    memset(&pm, 0, 5*sizeof(int));
  }
  if(bytes <= 0){ // Shutdown the socket!
    switch(errno){
    case EAGAIN: // Don't do anything for try again
      break;
      //    case 0:
      //      break;     // We just read nothing, don't panic
    case EIO:
    case EISDIR:
    case EBADF:
    case EINVAL:
    case EFAULT:
    default:
       //      perror("PukeController: read failed");
      closefd(fd);
      close(fd);
    }
  }
  else{
    qidConnectFd[fd]->sr->setEnabled(TRUE);
  }
}


void PukeController::ServMessage(QString, int, QString) 
{

}

// Message Dispatcher is in messagedispatcher.cpp


void PukeController::MessageDispatch(int fd, PukeMessage *pm) 
{
    try {

        /*
         * Get the object id, this may produce a errorNuSuchWidget
         */
        PObject *obj = id2pobject(fd, pm->iWinId);

        /*
         * Call the message hanlder for the widget
         */
        obj->messageHandler(fd, pm);
    }
    catch(errorNoSuchWidget &err){
      PukeMessage pmRet;
      pmRet.iCommand = PUKE_INVALID;
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = 0;
      pmRet.iTextSize = 0;
      emit outputMessage(fd, &pmRet);
      return;
    }
   catch (errorCommandFailed &err){
      PukeMessage pmRet;
      pmRet.iCommand = err.command();
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = err.iarg();
      pmRet.iTextSize = 0;
      emit outputMessage(fd, &pmRet);
      return;
    }
}

void PukeController::initHdlr() 
{

  widgetCreate *wc;

  wc = new widgetCreate;
  wc->wc = PWidget::createWidget;
  widgetCF.insert(PWIDGET_WIDGET, wc);

  wc = new widgetCreate;
  wc->wc = PObject::createWidget;
  widgetCF.insert(PWIDGET_OBJECT, wc);

  wc = new widgetCreate;
  wc->wc = PLayout::createWidget;
  widgetCF.insert(POBJECT_LAYOUT, wc);

  wc = new widgetCreate;
  wc->wc = PFrame::createWidget;
  widgetCF.insert(PWIDGET_FRAME, wc);

  wc = new widgetCreate;
  wc->wc = PLineEdit::createWidget;
  widgetCF.insert(PWIDGET_LINED, wc);

  wc = new widgetCreate;
  wc->wc = PButton::createWidget;
  widgetCF.insert(PWIDGET_BUTTON, wc);

  wc = new widgetCreate;
  wc->wc = PPushButton::createWidget;
  widgetCF.insert(PWIDGET_PUSHBT, wc);

  wc = new widgetCreate;
  wc->wc = PProgress::createWidget;
  widgetCF.insert(PWIDGET_KSPROGRESS, wc);

  wc = new widgetCreate;
  wc->wc = PTableView::createWidget;
  widgetCF.insert(PWIDGET_TABLEVW, wc);

  wc = new widgetCreate;
  wc->wc = PListBox::createWidget;
  widgetCF.insert(PWIDGET_LISTBOX, wc);

  wc = new widgetCreate;
  wc->wc = PLabel::createWidget;
  widgetCF.insert(PWIDGET_LABEL, wc);

  wc = new widgetCreate;
  wc->wc = PPopupMenu::createWidget;
  widgetCF.insert(PWIDGET_POPMENU, wc);

  wc = new widgetCreate;
  wc->wc = PAListBox::createWidget;
  widgetCF.insert(PWIDGET_ALISTBOX, wc);

  wc = new widgetCreate;
  wc->wc = PTabDialog::createWidget;
  widgetCF.insert(PWIDGET_TABDIALOG, wc);

  wc = new widgetCreate;
  wc->wc = PKFileDialog::createWidget;
  widgetCF.insert(PWIDGET_KFILEDIALOG, wc);

  wc = new widgetCreate;
  wc->wc = PObjFinder::createWidget;
  widgetCF.insert(PWIDGET_OBJFINDER, wc);

  // Each function handler gets an entry in the qidCommandTable
  commandStruct *cs;


  // Invalid is the default invalid handler
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeInvalid;
  cs->library = 0;
  qidCommandTable.insert(PUKE_INVALID, cs);


  // Setup's handled by the setup handler
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeSetup;
  cs->library = 0;
  qidCommandTable.insert(PUKE_SETUP, cs);

  // We don't receive PUKE_SETUP_ACK's we send them.
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeInvalid;
  cs->library = 0;
  qidCommandTable.insert(PUKE_SETUP_ACK, cs);

  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeEcho;
  cs->library = 0;
  qidCommandTable.insert(PUKE_ECHO, cs);

  // Fetch widget gets the requested widget from the ServerController
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeFetchWidget;
  cs->library = 0;
  qidCommandTable.insert(PUKE_FETCHWIDGET, cs);

  // Fetch widget gets the requested widget from the ServerController
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeDumpTree;
  cs->library = 0;
  qidCommandTable.insert(PUKE_DUMPTREE, cs);

  // Fetch widget gets the requested widget from the ServerController
  cs = new commandStruct;
  cs->cmd = &PukeController::hdlrPukeDeleteWidget;
  cs->library = 0;
  qidCommandTable.insert(PUKE_WIDGET_DELETE, cs);

}

// Start message handlers

void PukeController::hdlrPukeInvalid(int fd, PukeMessage *) 
{
  PukeMessage pmOut;
  memset(&pmOut, 0, sizeof(pmOut));
  this->writeBuffer(fd, &pmOut);
}


void PukeController::hdlrPukeSetup(int fd, PukeMessage *pm) 
{
  PukeMessage pmOut;
  memset(&pmOut, 0, sizeof(pmOut));
  pmOut.iCommand = PUKE_SETUP_ACK;
  pmOut.iArg = 1;
  if((strlen(pm->cArg) > 0) &&
     (this->qidConnectFd[fd] != NULL)){
    this->qidConnectFd[fd]->server = qstrdup(pm->cArg);
    pmOut.iWinId = pm->iWinId;
    pmOut.iArg = sizeof(PukeMessage) - sizeof(char *);
  }
  this->writeBuffer(fd, &pmOut);
}

void PukeController::hdlrPukeEcho(int fd, PukeMessage *pm) 
{
  PukeMessage pmOut;
  memcpy(&pmOut, pm, sizeof(PukeMessage));
  pmOut.iCommand = PUKE_ECHO_ACK;
  pmOut.iWinId = pm->iWinId;
  pmOut.iArg = pm->iArg;
  this->writeBuffer(fd, &pmOut);
}

void PukeController::hdlrPukeDumpTree(int fd, PukeMessage *pm) 
{
  objFinder::dumpTree();

  PukeMessage pmOut;
  memcpy(&pmOut, pm, sizeof(PukeMessage));
  pmOut.iCommand = -pm->iCommand;
  pmOut.iWinId = pm->iWinId;
  pmOut.iArg = pm->iArg;
  this->writeBuffer(fd, &pmOut);
}


void PukeController::hdlrPukeFetchWidget(int fd, PukeMessage *pm) 
{
  widgetId wIret;

  /*
   * The parent widget ID and type are packed into the iArg
   * the pattern is 2 shorts.
   */

  int iParent=-1, iType=-1;

  char rand[50],name[50];
  int found = sscanf(pm->cArg, "%d\t%d\t%49s\t%49s", &iParent, &iType, rand, name);
  if(found != 4){
      throw(errorCommandFailed(PUKE_INVALID,6));
  }

  uiBaseWinId++; // Get a new base win id

  // wIret holds the current widget id for the new widget
  wIret.iWinId = uiBaseWinId;
  wIret.fd = fd;

  //  CreateArgs arg = CreateArgs(this, pm, &wIret, parent)
  CreateArgs arg(this, pm, &wIret, 0);

  // Let's go looking for the widget
  // Match any class with the right name
  QObject *obj = 0x0;
  if(parent() && (strcmp(name, parent()->name()) == 0)){
    obj = parent();
  }
  else {
    obj = objFinder::find(name, 0x0);
    if(obj == 0){
      wIret.fd = 0;
      wIret.iWinId = 0;
      throw(errorCommandFailed(PUKE_INVALID,5));
    }

  }

  arg.fetchedObj = obj;

  WidgetS *ws = new WidgetS;
  ws->pwidget = (widgetCF[iType]->wc)(arg);
  if (ws->pwidget->hasError())
  {
    throw(errorCommandFailed(PUKE_INVALID, 0));
  }
  ws->type = iType;

  connect(ws->pwidget, SIGNAL(outputMessage(int, PukeMessage*)),
	  this, SIGNAL(outputMessage(int, PukeMessage*)));

  // insertPBoject(fd, uiBaseWinId, ws);
  // The widget list has to exist since we have ourselves in the list
  //  WidgetList[wIret.fd]->insert(wIret.iWinId, ws);
  insertPObject(wIret.fd, wIret.iWinId, ws);

  PukeMessage pmRet;
  pmRet.iCommand = PUKE_WIDGET_CREATE_ACK;
  pmRet.iWinId = wIret.iWinId;
  pmRet.iArg = 0;
  pmRet.iTextSize = pm->iTextSize;
  pmRet.cArg = pm->cArg;
  emit outputMessage(fd, &pmRet);

}

void PukeController::hdlrPukeDeleteWidget(int fd, PukeMessage *pm) 
{
  widgetId wI;
  wI.fd = fd;
  wI.iWinId = pm->iWinId;

  if(pm->iWinId == ControllerWinId) // Don't try and delete ourselves
    throw(errorCommandFailed(PUKE_INVALID, INVALID_DEL_NO_CONTROL));

  /*
  QIntDict<WidgetS> *qidWS = WidgetList[fd];
  if(qidWS == 0){
    kdDebug(5008) << "WidgetRunner:: no such set of widget descriptors?" << endl;
    throw(errorCommandFailed(PUKE_INVALID, INVALID_DEL_NO_SUCH_CONNECTION));
  }
  if(qidWS->find(wI.iWinId)){
    // Remove the list item then delete the widget.  This will stop
    // the destroyed signal from trying to remove it again.
    PObject *pw = qidWS->find(wI.iWinId)->pwidget;
    qidWS->remove(wI.iWinId);
    delete pw; pw = 0;
    pmRet.iCommand = PUKE_WIDGET_DELETE_ACK;
  }
  else {
    qWarning("WidgetRunner: no such widget: %d", wI.iWinId);
    throw(errorCommandFailed(PUKE_INVALID, INVALID_DEL_NO_SUCH_WIDGET));
    }
  */

  if(checkWidgetId(&wI) == FALSE){
    qWarning("WidgetRunner: no such widget: %d", wI.iWinId);
    throw(errorCommandFailed(PUKE_INVALID, INVALID_DEL_NO_SUCH_WIDGET));
  }

  WidgetList[fd]->find(wI.iWinId)->pwidget->manTerm();
  delete WidgetList[fd]->find(wI.iWinId)->pwidget;

  PukeMessage pmRet = *pm;
  pmRet.iCommand = PUKE_WIDGET_DELETE_ACK;

  emit outputMessage(fd, &pmRet);
}

void PukeController::closefd(int fd) 
{
  if(bClosing == TRUE)
    return;
  bClosing = TRUE;
  if(qidConnectFd[fd] == NULL){
    kdDebug(5008) << "PukeController: Connect table NULL, closed twice?" << endl;
    return;
  }
  // Shutof the listener before closing the socket, just in case.
  qidConnectFd[fd]->sr->setEnabled(FALSE); // Shut them off
  qidConnectFd[fd]->sw->setEnabled(FALSE);
  delete qidConnectFd[fd]->sr;
  delete qidConnectFd[fd]->sw;
  qidConnectFd[fd]->server.truncate(0);
  qidConnectFd.remove(fd);
  close(fd);

  /*
   * Now let's remove all traces of the widgets
   */
  QIntDict<WidgetS> *qidWS = WidgetList[fd];
  if(qidWS == 0){
    kdDebug(5008) << "WidgetRunner:: Close called twice?" << endl;
    bClosing = FALSE;
    return;
  }

  qidWS->remove(PUKE_CONTROLLER);

  do {

    QIntDictIterator<WidgetS> it(*qidWS);
    if(it.count() == 0){
      kdDebug(5008) << "WidgetRunner: nothing left to delete\n" << endl;
      break;
    }

    PObject *po = 0x0;
    while(it.current()){
      /*
       * Delete all the layouts first
       *
       */
      if(it.current()->type == POBJECT_LAYOUT){
        po = it.current()->pwidget;
        break;
      }
      ++it;
    }

    if(po != 0x0){
      po->manTerm();
      delete po;
      continue;
    }

    /*
     * reset
     */
    it.toFirst();
    po = it.current()->pwidget;
    po->manTerm();
    delete po;


  } while (qidWS->count() > 0);

  WidgetList.remove(fd);
  bClosing = FALSE;
}

bool PukeController::checkWidgetId(widgetId *pwi) 
{
  if(WidgetList[pwi->fd] != NULL)
    if(WidgetList[pwi->fd]->find(pwi->iWinId) != NULL)
      return TRUE;

  return FALSE;
}

PObject *PukeController::id2pobject(widgetId *pwi){ 
  if(checkWidgetId(pwi) == TRUE){
    return WidgetList[pwi->fd]->find(pwi->iWinId)->pwidget;
  }
  throw(errorNoSuchWidget(*pwi));
  return 0; // never reached
}

PObject *PukeController::id2pobject(int fd, int iWinId){ 
  widgetId wi;
  wi.fd = fd;
  wi.iWinId = iWinId;

  return id2pobject(&wi);
}

PWidget *PukeController::id2pwidget(widgetId *pwi){ 
  PObject *obj = id2pobject(pwi);
  if(obj->widget()->isWidgetType())
    return (PWidget *) obj;
  else
    throw(errorNoSuchWidget(*pwi));
  return NULL;
}
void PukeController::insertPObject(int fd, int iWinId, WidgetS *obj){ 
  // If no widget list exists for this fd, create one
  if(WidgetList[fd] == NULL){
    QIntDict<WidgetS> *qidWS = new QIntDict<WidgetS>;
    qidWS->setAutoDelete(TRUE);
    WidgetList.insert(fd, qidWS);
  }
  // Set main widget structure list
  WidgetList[fd]->insert(iWinId, obj);

  // Set reverse list used durring delete to remove the widget
  widgetId *pwi = new widgetId;
  pwi->fd = fd;
  pwi->iWinId = iWinId;
  char key[keySize];
  memset(key, 0, keySize);
  sprintf(key, "%p", obj->pwidget);
  revWidgetList.insert(key, pwi);

  // Now connect to the destroyed signal so we can remove the object from the lists
  // Once it is deleted
  connect(obj->pwidget, SIGNAL(destroyed()),
          this, SLOT(pobjectDestroyed()));
}

void PukeController::pobjectDestroyed(){

  char key[keySize];
  memset(key, 0, keySize);
  sprintf(key, "%p", this->sender());

  widgetId *pwi = revWidgetList[key];

  if(pwi == NULL){
    kdDebug(5008) << "Someone broke the rules for pwi: " << pwi->fd << ", " << pwi->iWinId << endl;
    return;
  }

  if(checkWidgetId(pwi) == TRUE){
    WidgetList[pwi->fd]->remove(pwi->iWinId);
  }
  else {
    kdDebug(5008) << "Someone stole pwi: " << pwi->fd << ", " << pwi->iWinId << endl;
  }

  pwi = 0x0; // remove deletes pwi
  revWidgetList.remove(key);

}

void PukeController::messageHandler(int fd, PukeMessage *pm) { 
  widgetId wI, wIret;
  wI.fd = fd;
  wI.iWinId = pm->iWinId;

  commandStruct *cs;

  cs = qidCommandTable[pm->iCommand];

  if(cs != NULL){
    (this->*(cs->cmd))(fd,pm);
  }
  else if(pm->iCommand == PUKE_WIDGET_CREATE){
    wIret = wI;
    wIret.iWinId = createWidget(wI, pm).iWinId; // Create the acutal pw

    PukeMessage pmRet;
    pmRet.iCommand = PUKE_WIDGET_CREATE_ACK;
    pmRet.iWinId = wIret.iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = strdup(pm->cArg);
    pmRet.iTextSize = strlen(pm->cArg);
    emit outputMessage(fd, &pmRet);
    free(pmRet.cArg);
  }
  else if(pm->iCommand == PUKE_WIDGET_LOAD){
    PukeMessage pmRet = *pm;
    KLibrary *library;
    PObject *(*wc)(CreateArgs &ca);
    widgetCreate *wC;

    if(widgetCF[pm->iArg]){
      pmRet.iCommand = -pm->iCommand;
      pmRet.iTextSize = 0;
      emit outputMessage(fd, &pmRet);
      return;
    }

    if(pm->iTextSize == 0){
      emit(errorCommandFailed(-pm->iCommand, 1));
      return;
    }

    QString libName = "ksirc/lib"+QString::fromLatin1(pm->cArg);
    if (libName.right(3) == ".so")
       libName = libName.left(libName.length()-2)+"la";

    library = KLibLoader::self()->library(libName);
    if (!library) {
      emit(errorCommandFailed(-pm->iCommand, 1));
      return;
    }
    wc =  (PObject *(*)(CreateArgs &ca) )
        library->symbol("createWidget");

    wC = new widgetCreate;
    wC->wc = wc;
    wC->library = library;
    widgetCF.insert(pm->iArg, wC);

    pmRet.iCommand = -pm->iCommand;
    pmRet.iTextSize = 0;
    emit outputMessage(fd, &pmRet);
  }
  else if(pm->iCommand == PUKE_WIDGET_UNLOAD){
    if(widgetCF[pm->iArg]){
//      delete widgetCF[pm->iArg]->library;
      widgetCF.remove(pm->iArg);
      pm->iCommand = -pm->iCommand;
      emit outputMessage(fd, pm);
    }
  }
  else{
    if(checkWidgetId(&wI) == TRUE){
      WidgetList[wI.fd]->find(wI.iWinId)->pwidget->messageHandler(fd, pm);
    }
    else{
      PukeMessage pmRet;
      pmRet.iCommand = PUKE_INVALID;
      pmRet.iWinId = wI.iWinId;
      pmRet.iArg = 0;
      pmRet.iTextSize = 0;
      emit outputMessage(fd, &pmRet);
    }
  }
}

widgetId PukeController::createWidget(widgetId wI, PukeMessage *pm) 
{
  widgetId wIret;
  PWidget *parent = 0; // Defaults to no parent
  WidgetS *ws = new WidgetS;

  /*
   * The parent widget ID and type are packed into the iArg
   * the pattern is 2 shorts.
   */

  int iParent, iType;
  int found = sscanf(pm->cArg, "%d\t%d", &iParent, &iType);
  if(found != 2)
      throw(errorCommandFailed(PUKE_INVALID,7));

  wI.iWinId = iParent; // wI is the identifier for the parent widget

  if(widgetCF[iType] == NULL){ // No such widget, bail out.
    wIret.fd = 0;
    wIret.iWinId = 0;
    throw(errorCommandFailed(PUKE_INVALID,1));
  }

  uiBaseWinId++; // Get a new base win id

  // wIret holds the current widget id for the new widget
  wIret.iWinId = uiBaseWinId;
  wIret.fd = wI.fd;

  if(checkWidgetId(&wI) == TRUE){
    PObject *obj = WidgetList[wI.fd]->find(wI.iWinId)->pwidget;
    if(obj->widget()->isWidgetType() == FALSE){
      throw(errorCommandFailed(PUKE_INVALID, 0));
    }
    parent = (PWidget *) obj;
  }

  //  CreateArgs arg = CreateArgs(this, pm, &wIret, parent)
  CreateArgs arg(this, pm, &wIret, parent);
  ws->pwidget = (widgetCF[iType]->wc)(arg);
  if (ws->pwidget->hasError())
  {
    throw(errorCommandFailed(PUKE_INVALID, 0));
  }
  ws->type = iType;

  connect(ws->pwidget, SIGNAL(outputMessage(int, PukeMessage*)),
	  this, SIGNAL(outputMessage(int, PukeMessage*)));

  // insertPBoject(fd, uiBaseWinId, ws);
  // The widget list has to exist since we have ourselves in the list
  insertPObject(wIret.fd, wIret.iWinId, ws);
//  WidgetList[wIret.fd]->insert(wIret.iWinId, ws);
  return wIret;
}


#include "controller.moc"

