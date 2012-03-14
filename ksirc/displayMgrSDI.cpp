#include "displayMgrSDI.h"

#include <qwidget.h>

DisplayMgrSDI::DisplayMgrSDI()
{
}

DisplayMgrSDI::~DisplayMgrSDI(){
}

void DisplayMgrSDI::newTopLevel(QWidget *w, bool show){
  if(show == TRUE)
    w->show();
}

void DisplayMgrSDI::removeTopLevel(QWidget *){
}

void DisplayMgrSDI::show(QWidget *w){
  w->show();
}

void DisplayMgrSDI::hide(QWidget *w){
  w->hide();
}

void DisplayMgrSDI::raise(QWidget *w, bool takefocus){
  w->show();
  w->raise();
  if(takefocus)
      w->setActiveWindow();

}


void DisplayMgrSDI::setCaption(QWidget *w, const QString& cap){
  w->setCaption(cap);
}

