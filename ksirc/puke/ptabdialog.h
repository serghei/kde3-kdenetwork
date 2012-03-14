#ifndef PTABDIALOG_H
#define PTABDIALOG_H

class PTabDialog;

#include <qtabdialog.h>
#include "pmessage.h"
#include "pwidget.h"

class PTabDialog : public PWidget
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PTabDialog(QObject *parent = 0);
  virtual ~PTabDialog();

  /**
   * Handles messages from dsirc
   * PObject can't get messages so return an error
   */
  virtual void messageHandler(int fd, PukeMessage *pm);

  /**
   * Sets the current opbect
   */
  virtual void setWidget(QObject *tb);
  
  /**
   * Returns the current object
   */
  virtual QTabDialog *widget();

private:

  QTabDialog *tab;

};

#endif
