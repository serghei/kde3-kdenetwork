#ifndef PBUTTON_H
#define PBUTTON_H

class PButton;

#include <qbutton.h>
#include "pmessage.h"
#include "pwidget.h"
#include "pobject.h"
#include "controller.h"


class PButton : public PWidget
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PButton ( PObject * parent );
  virtual ~PButton ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_qb = 0x0);
  virtual QButton *widget();

public slots:
  void buttonPressed();
  void buttonReleased();
  void buttonClicked();
  void buttonToggled(bool);
  
private:
  QButton *button;

  void buttonMessage(int iCommand);
  bool checkWidget();
};

#endif
