/***************************************************************************
                          strength.h  -  description
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

#ifndef STRENGTH_H
#define STRENGTH_H
#include <qwidget.h>

class Interface_wireless;
class QSize;

class Strength:public QWidget
{
Q_OBJECT 
  public:
  Strength (QWidget * parent, Interface_wireless * device);
  QSize mySizeHint();
  ~Strength ();
  QPixmap *NOT_CON_OFFLINE_ADHOC, *OOR_DOWN, *OOR_MINIMUM, *MARGINAL, *GOOD, *EXCELLENT;
  public slots:
  void updateInfo (QPaintEvent *);
  private:
  QString kdedir;
  void paintEvent (QPaintEvent *);
  Interface_wireless *device;
};

#endif
