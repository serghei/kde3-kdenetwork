/*
 * Main controller for:
 *
 * PUKE = Perl gUi Kontrol Environment
 *
 */

#ifndef PUKE_CONTROLLER_H
#define PUKE_CONTROLLER_H

#include <qobject.h>
#include <qsocketnotifier.h>
#include <qstring.h>
#include <qdict.h>
#include <qintdict.h>
#include <qstrlist.h>

#include "pmessage.h"
#include "pobject.h"
#include "pwidget.h"
#include "commands.h"

class PukeController;
class KLibrary;

typedef struct {
  QString server;
  bool writeable;
  QSocketNotifier *sr,*sw;
} fdStatus;


struct commandStruct {
  void (PukeController::*cmd)(int, PukeMessage*);
  KLibrary *library;
};

typedef struct {
  PObject *pwidget; // The widget
  int type;         // The type so casting is "safer"
} WidgetS;          // WidgetStruct

typedef struct {
  PObject *(*wc)(CreateArgs &ca);
  KLibrary *library;
} widgetCreate;

class errorNoSuchWidget {
public:
  errorNoSuchWidget(widgetId &_wi)
  {
    wi = _wi;
  }

  widgetId &widgetIden() {
    return wi;
  }
private:
  widgetId wi;
};

class errorCommandFailed {
public:
    errorCommandFailed(int _command, int _iarg){
        __command = _command;
        __iarg = _iarg;
    }

    int command() { return __command; }
    int iarg() { return __iarg; }
    
private:
    int __command, __iarg;
};

#define INVALID_DEL_NO_CONTROL 100
#define INVALID_DEL_NO_SUCH_CONNECTION 101
#define INVALID_DEL_NO_SUCH_WIDGET 102

class PukeController : public PObject
{
  Q_OBJECT
public:
  PukeController(QString socket = "", QObject *parent=0, const char *name=0);
  virtual ~PukeController();
  bool running;

  /**
   * Verifies the widgetId exists and is a valid widget.
   * True is valid, false if invalid.
   */
  bool checkWidgetId(widgetId *pwI);

  /**
   * id2pobject takes a window id and returns the reuired object
   * it throw an errorNoSuchWidget on failures
   */
  PObject *id2pobject(int fd, int iWinId);
  PObject *id2pobject(widgetId *pwi);
  /**
   * Return a PWidget if it's a widget, throws an exception if not found
   */
  PWidget *id2pwidget(widgetId *pwi);

  QStrList allObjects();

signals:
  void PukeMessages(QString server, int command, QString args);
  void inserted(QObject *);

public slots:
  void ServMessage(QString, int, QString);

protected slots:
  void Traffic(int);
  void Writeable(int);
  void NewConnect(int);
  void slotInserted(QObject *obj);

  /**
   * When we delete a widget, this removes it from our internal
   * list of widgets.  We never remove a widget ourselfs, we call delete
   * and this function removes it.
   */
  void pobjectDestroyed();

  /**
   * Fd to write to
   * PukeMessage message  to be written, if null buffer is flushed.
   */
  void writeBuffer(int fd, PukeMessage *message);


private:
  QString qsPukeSocket;
  int iListenFd;
  bool bClosing; // Set true if we are closing, we don't try and close twice at the same time.
  QSocketNotifier *qsnListen;
  QIntDict<fdStatus> qidConnectFd;

  /**
   * Controller ID is defined as 1
   */
  enum { ControllerWinId = PUKE_CONTROLLER };
      
  
  // List of widgets and the fle descriptors they belong too
  QIntDict<QIntDict<WidgetS> > WidgetList;
  // I use a char * key that's the %p (hex address) of the pwidget
  QDict<widgetId> revWidgetList;
  enum { keySize = 10 };

  // Funtions used to create new widget
  QIntDict<widgetCreate> widgetCF; // widgetCreatingFuntion List

  QIntDict<commandStruct> qidCommandTable;

  void initHdlr();

  void closefd(int fd);

  void MessageDispatch(int fd, PukeMessage *pm);

  /**
   * WinId comes from a static unsigned int we increment for each new window
   */
  static uint uiBaseWinId;
  
  /**
   * Create new Widget, returns new iWinId for it.
   * Takes server fd and parent winid, and type as arguments
   */
  widgetId createWidget(widgetId wI, PukeMessage *pm);

  /**
   * Used to process messages going to controller, winId #1
   *
   */
  void messageHandler(int fd, PukeMessage *pm);

  /**
   * NOT APPLICAABLE
   */
  void setWidget(QObject *) { }
  /**
   * NOT APPLICAABLE
   */
  virtual QObject *widget() { return 0x0; }

  /**
   * Inserts a PObject into our internal list
   */
  void insertPObject(int fd, int iWinId, WidgetS *obj);

  /**
   * Closes a widget, checking for sanity
   */
//  void closeWidget(widgetId wI);
   
  // Message handlers
  void hdlrPukeSetup(int fd, PukeMessage *pm);
  void hdlrPukeInvalid(int fd, PukeMessage *pm);
  void hdlrPukeEcho(int fd, PukeMessage *pm);
  void hdlrPukeDumpTree(int fd, PukeMessage *pm);
  void hdlrPukeFetchWidget(int fd, PukeMessage *pm);
  void hdlrPukeDeleteWidget(int fd, PukeMessage *pm);

};

#endif

