/***************************************************************************
                          locator.cpp  -  description
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
#include "locator.h"
#include "interface_wireless.h"

extern QStringList APs;

Locator::Locator (QWidget * parent, Interface_wireless * device):QWidget (parent,
	 "KwiFiManager")
{
  this->device = device;
}

void
Locator::generateLocWidget ()
{
  QString mac, ip;
  device->get_AP_info (mac, ip);
  sentence =
    new QLabel (i18n ("AccessPoint: ") + whois (mac.local8Bit (), APs), this);
  setFixedSize (QSize
		(sentence->sizeHint ().width () + 2,
		 sentence->sizeHint ().height ()));
  sentence->
    setFixedSize (QSize
		  (sentence->sizeHint ().width () + 2,
		   sentence->sizeHint ().height ()));
  sentence->show ();
  widgetbild = QPixmap::grabWidget (sentence);
  delete sentence;
}

void
Locator::paintEvent (QPaintEvent *)
{
  QPainter *locatorpainter = new QPainter (this);
  generateLocWidget ();
  locatorpainter->drawPixmap (0, 0, widgetbild);
  delete locatorpainter;
}
