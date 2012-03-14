/***************************************************************************
                          statistics.cpp  -  description
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
#include <klocale.h>
#include "statistics.h"
#include "interface_wireless.h"

Statistics::Statistics (Interface_wireless * device, bool showStatsNoise):QWidget (0,
	 "KWiFiManager")
{
  this->setCaption (i18n ("Statistics - KWiFiManager"));
  this->device = device;
  this->showStatsNoise = showStatsNoise;
}

void
Statistics::paintEvent (QPaintEvent *)
{
  statpainter = new QPainter (this);
  QColor farbe (255, 255, 255);
  statpainter->drawText (40, 30, i18n ("Noise/Signal Level Statistics"));
  statpainter->drawText (150, 260,
			 i18n ("BLUE = signal level, RED = noise level"));
  statpainter->drawLine (41, 244, 41, 260);
  statpainter->drawLine (520, 244, 520, 260);
  statpainter->drawText (21, 274, i18n ("-240 s"));
  statpainter->drawText (510, 274, i18n ("now"));
  statpainter->fillRect (41, 41, 480, 201, farbe);
  QPointArray datensatz (240);
  QPointArray datensatz2 (240);
  // we may need to scale the output to fit into the window, so here we 
  // determine the range of values
  int bottom =
    (device->sigLevelMin >
     device->noiseLevelMin) ? device->noiseLevelMin : device->sigLevelMin;
  int top =
    (device->sigLevelMax >
     device->noiseLevelMax) ? device->sigLevelMax : device->noiseLevelMax;
  int datarange = top - bottom;
  statpainter->drawText (10, 50, QString ("%1").arg (top));
  statpainter->drawText (10, 240, QString ("%1").arg (bottom));

  // if values are all below 0, this indicates proper dBm values

  if (top < 0) {
	statpainter->drawText (2,65, "dBm");
	statpainter->drawText (2,255, "dBm");
  }

  double scaleRatio;
  if (datarange != 0)
    {
      scaleRatio = 200. / datarange;
    }
  else
    {
      scaleRatio = 1.;
    }
  int i = 0;
  bool atLeastOneValid = false;
  for (int j = device->current; j < device->current + MAX_HISTORY; j++)
    {
      if (device->valid[j % MAX_HISTORY])
	{
	  atLeastOneValid = true;
	  datensatz.setPoint (i, 40 + (i * 2),
			      241 -
			      (int) (((device->sigLevel[j % MAX_HISTORY] + device->sigLevel[(j-1) % MAX_HISTORY]) / 2 -
				      bottom) * scaleRatio));
	  datensatz2.setPoint (i, 40 + (i * 2),
			       241 -
			       (int) (((device->noiseLevel[j % MAX_HISTORY] + device->noiseLevel[(j-1) % MAX_HISTORY]) / 2 -
				       bottom) * scaleRatio));
	}
      else
	{
	  datensatz.setPoint (i, 40 + (i * 2), 241);
	  datensatz2.setPoint (i, 40 + (i * 2), 241);
	}
      i++;
    };
  // the above point array is only useful if at least one entry has its
  // valid flag set. This fact is determined in the iteration before.
  // Only paint the point array if there is one valid entry
  if (atLeastOneValid)
    {
      if (showStatsNoise) {
        statpainter->setPen (red);
        statpainter->drawPolyline (datensatz2, 1);
      }
      statpainter->setPen (blue);
      statpainter->drawPolyline (datensatz, 1);
    }
  delete statpainter;
}
