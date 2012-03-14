/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#ifndef KWIRELESS_H
#define KWIRELESS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kpanelapplet.h>
#include <qstring.h>
#include <kconfig.h>

class KWireLessWidget;

class KWireLess : public KPanelApplet
{
    Q_OBJECT

public:
    KWireLess(const QString& configFile, Type t = Normal, int actions = 0,
              QWidget *parent = 0, const char *name = 0);
    ~KWireLess();

    virtual int widthForHeight(int height) const;
    virtual int heightForWidth(int width) const;
    virtual void about();
    virtual void help();
    virtual void preferences();

protected:
    void resizeEvent(QResizeEvent *);

private:
    KConfig *ksConfig;
    KWireLessWidget *widget;
};

#endif
