/***************************************************************************
                          status.h  -  description
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

#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qpainter.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>

class Interface_wireless;

class Status:public QWidget
{
public:
  Status (QWidget * parent, Interface_wireless * device);
private:
  void paintEvent (QPaintEvent *);
  bool generateStatsWidget ();
  QGroupBox *content;
  QLabel *stat, *net_id, *ap_str, *ap_ad, *fr_str, *fr, *enc_str, *enc,
    *local_ip_content, *local_ip_string;
  QPixmap widgetbild, widgetbildalt;
  Interface_wireless *device;
};

#endif
