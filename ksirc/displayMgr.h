#ifndef DISPLAYMGR_H
#define DISPLAYMGR_H

#include <qstring.h>

class QWidget;

class DisplayMgr {
public:
    virtual ~DisplayMgr() {};

    virtual void newTopLevel(QWidget *, bool show = false) = 0;

    virtual void removeTopLevel(QWidget *) = 0;

    virtual void show(QWidget *) = 0;

    virtual void hide(QWidget *) = 0;

    virtual void raise(QWidget *, bool takefocus = false) = 0;

    virtual void setCaption(QWidget *, const QString&) = 0;

};


#endif

