/***************************************************************************
                          speed.cpp  -  description
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

#include <qpainter.h>
#include <klocale.h>
#include <kdebug.h>
#include "speed.h"
#include "interface_wireless.h"

Speed::Speed (QWidget * parent, Interface_wireless * device):QWidget (parent,
	 "KWiFiManager")
{
  this->device = device;
}

void
Speed::paintEvent (QPaintEvent *)
{
  QPainter speedpainter (this);
  QColor farbe;
  farbe.setRgb (0, 255, 0);
  if (device->get_bitrate () < 6000000)
    farbe.setRgb (0, 200, 0);
  if (device->get_bitrate () < 2500000)
    farbe.setRgb (255, 255, 0);
  if (device->get_bitrate () < 1500000)
    farbe.setRgb (255, 128, 0);
  speedpainter.drawText (20, 20, i18n ("Connection speed [MBit/s]:"));
  speedpainter.drawRect (19, 29, 222, 12);
  if (device->get_bitrate () <= 11000000.)
    {
      speedpainter.fillRect (20, 30, (int) device->get_bitrate () / 50000,
			      10, farbe);
      speedpainter.drawText (16, 52, "0");
      speedpainter.drawText (35, 52, "1");
      speedpainter.drawText (55, 52, "2");
      speedpainter.drawText (120, 52, "5.5");
      speedpainter.drawText (230, 52, "11");
    }
  else
    {
      speedpainter.fillRect (20, 30, (int) device->get_bitrate () / 500000,
			      10, farbe);
      speedpainter.drawText (16, 52, "0");
      speedpainter.drawText (33, 52, "11");
      speedpainter.drawText (56, 52, "22");
      speedpainter.drawText (120, 52, "54");
      speedpainter.drawText (220, 52, "108");
    };
}
