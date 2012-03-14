#ifndef PTABLEVIEW_H
#define PTABLEVIEW_H

class PTableView;

#include <qgridview.h>

#include "pmessage.h"
#include "pframe.h"
#include "controller.h"

// Init and setup code
class PTableView : public PFrame
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PTableView ( PObject * parent );
  virtual ~PTableView ();
  
  virtual void messageHandler(int fd, PukeMessage *pm);
  
  virtual void setWidget(QObject *_tbl);
  virtual QGridView *widget();

public slots:

private:
  QGridView *tbl;
};

#endif
