#ifndef PLAYOUT_H
#define PLAYOUT_H

class PLayout;

#include <qobject.h>
#include <qlayout.h>
#include "pmessage.h"
#include "pobject.h"

class PLayout : public PObject
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PLayout(QObject *parent = 0);
  virtual ~PLayout();

  /**
   * Handles messages from dsirc
   * PObject can't get messages so return an error
   */
  virtual void messageHandler(int fd, PukeMessage *pm);

  /**
   * Sets the current opbect
   */
  virtual void setWidget(QObject *qb = 0x0);
  
  /**
   * Returns the current object
   */
  virtual QBoxLayout *widget();

private:

  QBoxLayout *layout;

};

#endif
