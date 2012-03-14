#ifndef PLINEEDIT_H
#define PLINEEDIT_H

class PLineEdit;

#include <qlineedit.h>
#include "pmessage.h"
#include "pwidget.h"
#include "controller.h"

class PLineEdit : public PWidget
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PLineEdit ( PObject * parent );
  virtual ~PLineEdit ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_f);
  virtual QLineEdit *widget();

public slots:
  void updateText(const char *);
  void returnPress();
  
private:
  QLineEdit *lineedit;
};

#endif
