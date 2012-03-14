#ifndef PLISTBOX_H
#define PLISTBOX_H

class PListBox;

#include <qlistbox.h>
#include "pmessage.h"
#include "ptablevw.h"
#include "controller.h"

class PListBox : public PFrame
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PListBox ( PObject * parent );
  virtual ~PListBox ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_lb);
  virtual QListBox *widget();

public slots:
  void highlighted(int);
  void selected(int);

protected:
  bool checkWidget();

private:
  QListBox *lb;
   
};

#endif
