#ifndef PPOPMENU_H
#define PPOPMENU_H

class PPopupMenu;

#include <qpopupmenu.h>
#include "pobject.h"
#include "pmessage.h"

#include "ptablevw.h"
#include "pmenudta.h"

class PPopupMenu : public PFrame
{
  Q_OBJECT
public:
  static PObject *createWidget(CreateArgs &ca);

  PPopupMenu(PObject *child);
  virtual ~PPopupMenu();

  virtual void messageHandler(int fd, PukeMessage *pm);

  virtual void setWidget(QObject *_menu);
  virtual QPopupMenu *widget();

protected slots:
  void got_activated ( int itemId );

private:
  QPopupMenu *menu;
  PMenuData *pmd;

};

#endif
