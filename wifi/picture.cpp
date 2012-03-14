/***************************************************************************
                          picture.cpp  -  description
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

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <kstandarddirs.h>
#include "picture.h"
#include "interface_wireless.h"

#define X_OFFSET 20
#define Y_OFFSET 10

Picture::Picture (QWidget * parent, Interface_wireless * device):QWidget (parent,
	 "picture")
{
  this->device = device;
  KStandardDirs finder;
  kdedir = finder.findResourceDir ("data", "kwifimanager/pics/no_card.png");
  kdedir += "kwifimanager/pics/";
  NO_CARD = new QPixmap (kdedir + "no_card.png");
  AD_HOC = new QPixmap (kdedir + "ad_hoc.png");
  ALL_ALONE = new QPixmap (kdedir + "all_alone.png");
  AP_CONNECT = new QPixmap (kdedir + "ap_connect.png");
}

QSize Picture::mySizeHint() {
  /* make this quite general, just in case someone uses different icons */
  QSize* temp = new QSize(NO_CARD->size());
  if (AD_HOC->size().width()  > temp->width ()) { temp->setWidth (AD_HOC->size().width ()); }
  if (AD_HOC->size().height() > temp->height()) { temp->setHeight(AD_HOC->size().height()); }
  if (ALL_ALONE->size().width()  > temp->width ()) { temp->setWidth (ALL_ALONE->size().width ()); }
  if (ALL_ALONE->size().height() > temp->height()) { temp->setHeight(ALL_ALONE->size().height()); }
  if (AP_CONNECT->size().width()  > temp->width ()) { temp->setWidth (AP_CONNECT->size().width ()); }
  if (AP_CONNECT->size().height() > temp->height()) { temp->setHeight(AP_CONNECT->size().height()); }
  temp->setWidth (temp->width()  + X_OFFSET);
  temp->setHeight(temp->height() + Y_OFFSET);
  return *temp;
}

Picture::~Picture ()
{
  delete NO_CARD;
  delete AD_HOC;
  delete ALL_ALONE;
  delete AP_CONNECT;

}

void
Picture::paintEvent (QPaintEvent *)
{
  QPainter *picturepainter = new QPainter (this);
  double freq;
  int mode;
  device->get_mode (mode);
  int sig, noise, qual;
  device->get_current_quality (sig, noise, qual);
  if (!device->get_device_freq (freq) || device->get_txpower_disabled())
    {
      picturepainter->drawPixmap (X_OFFSET, Y_OFFSET, *NO_CARD);
    }
  else if (mode == 1)
    {
      picturepainter->drawPixmap (X_OFFSET, Y_OFFSET, *AD_HOC);
    }
  else if (qual == 0)
    {
      picturepainter->drawPixmap (X_OFFSET, Y_OFFSET, *ALL_ALONE);
    }
  else if (qual > 0)
    {
      picturepainter->drawPixmap (X_OFFSET, Y_OFFSET, *AP_CONNECT);
    }
  else
    picturepainter->drawPixmap (X_OFFSET, Y_OFFSET, *NO_CARD);
  delete picturepainter;
}
