/***************************************************************************
                          statistics.h  -  description
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
#ifndef STATISTICS_H
#define STATISTICS_H

class Interface_wireless;

class Statistics:public QWidget
{
public:
  Statistics (Interface_wireless * device, bool showStatsNoise);
private:
  QPainter * statpainter;
  void paintEvent (QPaintEvent *);
  Interface_wireless *device;
  bool showStatsNoise;
};

#endif
