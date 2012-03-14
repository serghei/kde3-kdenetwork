/***************************************************************************
                          strength.cpp  -  description
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
#include <qlabel.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include "strength.h"
#include "interface_wireless.h"

extern bool useAlternateStrengthCalc;

Strength::Strength (QWidget * parent, Interface_wireless * device):QWidget (parent,
	 "strength")
{
  this->device = device;
  KStandardDirs finder;
  kdedir = finder.findResourceDir ("data", "kwifimanager/pics/no_card.png");
  kdedir += "kwifimanager/pics/";
  NOT_CON_OFFLINE_ADHOC = new QPixmap (kdedir + "offline.png");
  EXCELLENT = new QPixmap (kdedir + "excellent.png");
  GOOD = new QPixmap (kdedir + "good.png");
  MARGINAL = new QPixmap (kdedir + "marginal.png");
  OOR_MINIMUM = new QPixmap (kdedir + "oor_minimum.png");
  OOR_DOWN = new QPixmap (kdedir + "oor_down.png");
}

QSize Strength::mySizeHint () 
{
   /* make this quite general, just in case someone uses different icons */
  QSize* temp = new QSize(NOT_CON_OFFLINE_ADHOC->size());
  if (EXCELLENT->size().width()  > temp->width ()) { temp->setWidth (EXCELLENT->size().width ()); }
  if (EXCELLENT->size().height() > temp->height()) { temp->setHeight(EXCELLENT->size().height()); }
  if (GOOD->size().width()  > temp->width ()) { temp->setWidth (GOOD->size().width ()); }
  if (GOOD->size().height() > temp->height()) { temp->setHeight(GOOD->size().height()); }
  if (MARGINAL->size().width()  > temp->width ()) { temp->setWidth (MARGINAL->size().width ()); }
  if (MARGINAL->size().height() > temp->height()) { temp->setHeight(MARGINAL->size().height()); }
  if (OOR_MINIMUM->size().width()  > temp->width ()) { temp->setWidth (OOR_MINIMUM->size().width ()); }
  if (OOR_MINIMUM->size().height() > temp->height()) { temp->setHeight(OOR_MINIMUM->size().height()); }
  if (OOR_DOWN->size().width()  > temp->width ()) { temp->setWidth (OOR_DOWN->size().width ()); }
  if (OOR_DOWN->size().height() > temp->height()) { temp->setHeight(OOR_DOWN->size().height()); }
  // add a little more x-space: we need 50 pixels left and 50 pixels right of the icon
  temp->setWidth (temp->width()  + 100);
  // add a little more y-space: we need 10 pixels above and 20 pixels below the icon
  temp->setHeight(temp->height() + 30);
  // up to now: this is the height required to get a decent logo display
  // now determine height of QLabels to calculate the extra needed height
  QLabel heightTest( "ABC", 0 );
  temp->setHeight( temp->height() + 2 * heightTest.sizeHint().height() + 10 );
  return *temp;
}

Strength::~Strength ()
{
  delete NOT_CON_OFFLINE_ADHOC;
  delete GOOD;
  delete MARGINAL;
  delete OOR_MINIMUM;
  delete OOR_DOWN;
}

void
Strength::paintEvent (QPaintEvent * event)
{
  updateInfo (event);
}

void
Strength::updateInfo (QPaintEvent *)
{
  QPainter *strengthpainter = new QPainter (this);
  QString sstrength1, sstrength2( i18n("N/A") );
  int mode;
  double freq;

  int sig, noi, strength;
  bool validdata = device->get_current_quality (sig, noi, strength);
  if (useAlternateStrengthCalc) 
	strength = sig - noi;

  if ( device->get_txpower_disabled() )
    {
      sstrength1 = i18n("DISABLED");
      strengthpainter->drawPixmap (50, 10, *NOT_CON_OFFLINE_ADHOC);
    }
  else if (!device->get_device_freq (freq))
    {
      sstrength1 = i18n("NOT CONNECTED");
      strengthpainter->drawPixmap (50, 10, *NOT_CON_OFFLINE_ADHOC);
    }
  else if (device->get_mode(mode) && mode == 1)
    {
      sstrength1 = i18n("AD-HOC MODE");
      strengthpainter->drawPixmap (50, 10, *NOT_CON_OFFLINE_ADHOC);
    }
  else if (validdata && (strength > 45))
    {
      sstrength1 = i18n("ULTIMATE");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *EXCELLENT);
    }
  else if (validdata && (strength > 35))
    {
      sstrength1 = i18n("TOP");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *EXCELLENT);
    }
  else if (validdata && (strength > 25))
    {
      sstrength1 = i18n("EXCELLENT");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *EXCELLENT);
    }
  else if (validdata && (strength > 15))
    {
      sstrength1 = i18n("GOOD");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *GOOD);
    }
  else if (validdata && (strength > 5))
    {
      sstrength1 = i18n("WEAK");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *MARGINAL);
    }
  else if (validdata && (strength > 0))
    {
      sstrength1 = i18n("MINIMUM");
      sstrength2 = QString::number(strength);
      strengthpainter->drawPixmap (50, 10, *OOR_MINIMUM);
    }
  else if (((strength == 0) && (mode == 2)) || (!validdata))
    {
      sstrength1 = i18n("OUT OF RANGE");
      sstrength2 = QString::fromLatin1("0");
      strengthpainter->drawPixmap (50, 10, *OOR_DOWN);
    }
  else
    {
      sstrength1 = i18n("N/A");
      strengthpainter->drawPixmap (50, 10, *NOT_CON_OFFLINE_ADHOC);
    };
  sstrength2.prepend( i18n("Signal strength: ") );
  strengthpainter->drawText (5, 55, sstrength1);
  strengthpainter->drawText (5, 80, sstrength2);
  delete strengthpainter;
}

#include "strength.moc"
