/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#ifndef KWIRELESS_PROPERTYTABLE_H
#define KWIRELESS_PROPERTYTABLE_H

#include <kdialogbase.h>
#include <qptrlist.h>
#include "propertytablebase.h"

class DeviceInfo;
class QTimer;

class PropertyTable : public PropertyTableBase
{
    Q_OBJECT
public:
    PropertyTable(QWidget *parent = 0, const char* name = 0);
    ~PropertyTable();
};


class PropertiesDialog : public KDialogBase
{
    Q_OBJECT
public:
    PropertiesDialog(QWidget *parent = 0, const char *name = 0);
protected:
    PropertyTable *table;
    QPtrList<DeviceInfo> *info;
    bool wait;
    QTimer *timer;
public slots:
    void update(QPtrList<DeviceInfo> *);
    void timeout();
    void selected(int);
};

#endif // KWIRELESS_PROPERTYTABLE_H
