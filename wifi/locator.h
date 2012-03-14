/***************************************************************************
                          locator.h  -  description
                             -------------------
    begin                : Mon Aug 19 2002
    copyright            : (C) 2002 by Stefan Winter
    email                : mail@stefan-winter.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOCATOR_H
#define LOCATOR_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qlabel.h>

#include <klocale.h>

class Interface_wireless;

class Locator:public QWidget
{
public:
  Locator (QWidget * parent, Interface_wireless * device);
private:
  void paintEvent (QPaintEvent *);
  void generateLocWidget ();
  QPixmap widgetbild;
  QLabel *sentence;
  Interface_wireless *device;
};

#endif
