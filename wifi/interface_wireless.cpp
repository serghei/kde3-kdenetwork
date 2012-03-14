/***************************************************************************
                          stuff.cpp  -  description
                             -------------------
    begin                : Sun May 6 2001
    copyright            : (C) 2001 by Stefan Winter
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

#include "interface_wireless.h"
#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>

#ifndef WITHOUT_ARTS
#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/iomanager.h>
#include <arts/referenceclean.h>
#endif // WITHOUT_ARTS

#include <iostream>
#include <string>
#include <klocale.h>
#include <kprocio.h>
#include <kdebug.h>
#include <qstring.h>
#include <arpa/inet.h>

Interface_wireless::Interface_wireless (QStringList * ignoreInterfaces)
{
  this->ignoreInterfaces = ignoreInterfaces;
  has_frequency = false;
  frequency = 0.;
  has_mode = false;
  mode = 0;
  has_key = 0;
  key = "";
  key_size = 0;
  key_flags = 0;
  essid = "";
  access_point_address = "";
  ip_address = "";
  bitrate = 0.;
  socket = -1;
  has_range = false;
  for (int i = 0; i < MAX_HISTORY; i++)
    {
      sigLevel[i] = -255;
      noiseLevel[i] = -255;
      qual[i] = -255;
      valid[i] = false;
    }
  current = 0;
  sigLevelMin = 32000;
  noiseLevelMin = 32000;
  sigLevelMax = -32000;
  noiseLevelMax = -32000;

  already_warned = false;
}

bool Interface_wireless::get_device_freq (double &freq)
{
  if (has_frequency)
    {
      freq = frequency;
      return true;
    }
  else
    {
      return false;
    }
}

bool Interface_wireless::get_mode (int &ext_mode)
{
  if (has_mode)
    {
      ext_mode = mode;
      return true;
    }
  else
    {
      return false;
    }
}

bool Interface_wireless::get_key (QString & ext_key,
						  int &ext_size,
						  int &ext_flags)
{
  if (has_key)
    {
      ext_key = key;
      ext_size = key_size;
      ext_flags = key_flags;
      return true;
    }
  else
    {
      return false;
    }
}

QString Interface_wireless::get_essid ()
{
  return essid;
}

bool Interface_wireless::get_has_txpower()
{
  return has_txpower;
}

int Interface_wireless::get_txpower_disabled()
{
  return txpower_disabled;
}

bool Interface_wireless::get_AP_info (QString & mac, QString &/*ip*/)
{
  mac = access_point_address;
  return false;
}

QString Interface_wireless::get_IP_info ()
{
  return ip_address;
}

double Interface_wireless::get_bitrate ()
{
  return bitrate;
}

QString Interface_wireless::get_interface_name ()
{
  return interface_name;
}

bool Interface_wireless::get_current_quality (int &sig,
							      int &noi,
							      int &qua)
{
  if (valid[current])
    {
      sig = sigLevel[current];
      noi = noiseLevel[current];
      qua = qual[current];
      return true;
    }
  else
    return false;
}

#ifndef WITHOUT_ARTS

void
sinus_wave (double frequency)
{
  using namespace Arts;
  StdIOManager *limiter = new StdIOManager;
  Dispatcher dispatcher (limiter);
  MyTimeNotify *zeit = new MyTimeNotify (&dispatcher);
  Synth_FREQUENCY freq;
  Synth_WAVE_SIN sin;
  Synth_PLAY play;
  setValue (freq, frequency);
  connect (freq, sin);
  connect (sin, play, "invalue_left");
  freq.start ();
  sin.start ();
  play.start ();
  limiter->addTimer (240, zeit);
  dispatcher.run ();
  play.stop ();
  sin.stop ();
  freq.stop ();
}

void
MyTimeNotify::notifyTime ()
{
  test->terminate ();
}

#endif

QString
whois (const char *MAC_ADR, QStringList APList)
{
  for (QStringList::Iterator it = APList.begin (); it != APList.end ();
       (it++)++)
    {
      if ((*it) == (QString) MAC_ADR)
	return *(++it);
    }
  return i18n ("UNKNOWN");
}

#include "interface_wireless.moc"
