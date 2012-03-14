
/***************************************************************************
                          led.cpp  -  description
                             -------------------
    begin                : wo okt 8 2003
    copyright            : (C) 2003 by Stefan Winter
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

#include <fstream>
#include "asusled.h"

#include <kdebug.h>

Led::Led ()
{
  led = new std::ofstream ("/proc/acpi/asus/wled");

  // If there is no LED, just give a one time warning on cerr
  if (!(*led))
    {
      static bool error_given = false;
      if (!error_given)
	{
	  kdDebug () <<
	    "No Asus WLAN LED found, disabling LED updates" << endl;
	  error_given = true;
	}
    }
  Off ();
  // if Asusled not present, state never gets initialised, so we do it once here
  // so that valgrind doesn't complain.
  state = false;
}

Led::~Led ()
{

  Off ();
  delete led;

}


void
Led::On (void)
{

  if (*led)
    {
      *led << 1;
      led->flush ();
      state = true;
    }

}

void
Led::Off (void)
{

  if (*led)
    {
      *led << 0;
      led->flush ();
      state = false;
    }

}
