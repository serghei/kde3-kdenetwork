#ifndef PKFILEDIALOG_H
#define PKFILEDIALOG_H

class PKFileDialog;

#include <kfiledialog.h>
#include "pmessage.h"
#include "pwidget.h"
#include "controller.h"

class PKFileDialog : public PWidget
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PKFileDialog ( PObject * parent );
  virtual ~PKFileDialog ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_f = 0x0);
  virtual KFileDialog *widget();

public slots:

private:
  KFileDialog *kfbd;
};

#endif
