/***************************************************************************
                          picture.h  -  description
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

#ifndef PICTURE_H
#define PICTURE_H

class Interface_wireless;
class QSize;

class Picture:public QWidget
{
public:
  Picture (QWidget * parent, Interface_wireless * device);
  QSize mySizeHint();
  ~Picture ();
private:
  void paintEvent (QPaintEvent *);
  QPixmap *NO_CARD, *ALL_ALONE, *AD_HOC, *AP_CONNECT;
  QString kdedir;
  Interface_wireless *device;
};

#endif
