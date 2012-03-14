/***************************************************************************
                          status.cpp  -  description
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

#include <klocale.h>
#include "status.h"
#include "interface_wireless.h"

#include <unistd.h>
#include <sys/types.h>

Status::Status (QWidget * parent, Interface_wireless * device)
	: QWidget (parent, "KWiFiManager status")
{
  this->device = device;
}

bool Status::generateStatsWidget ()
{
  double
    freq = 0.;
  int
    sig = 0,
    noi = 0,
    qual = 0;
  device->get_current_quality (sig, noi, qual);
  content =
    new QGroupBox (2, Qt::Horizontal, i18n ("Status of Active Connection"),
		   this);
  bool has_freq = device->get_device_freq (freq);
  if ((!has_freq) || (qual == 0))
    {
      stat = new QLabel (i18n ("Searching for network: "), content);
    }
  else
    stat = new QLabel (i18n ("Connected to network: "), content);
  net_id = new QLabel (device->get_essid (), content);
  ap_str = new QLabel (i18n ("Access point: "), content);
  QString
    mac,
    ip;
  device->get_AP_info (mac, ip);
  if ((mac != "44:44:44:44:44:44") && (mac != "00:00:00:00:00:00")) {
	ap_ad = new QLabel (mac, content);
  } else {
	ap_ad = new QLabel (i18n("- no access point -"),content);
  }	
  local_ip_string = new QLabel (i18n ("Local IP: "), content);
  local_ip_content = new QLabel (device->get_IP_info (), content);
  fr_str = new QLabel (i18n ("Frequency [channel]: "), content);
  QString
    temp3 = "%1";
  QString
    temp2 = "%1";
  if (freq >= 1000000000)
    {
      if (freq / 1000000. > 5690.)
        {
          temp3 = temp3.arg(140);
        }
      else if (freq / 1000000. > 5670.)
        {
          temp3 = temp3.arg(136);
        }
      else if (freq / 1000000. > 5650.)
        {
          temp3 = temp3.arg(132);
        }
      else if (freq / 1000000. > 5630.)
        {
          temp3 = temp3.arg(128);
        }
      else if (freq / 1000000. > 5610.)
        {
          temp3 = temp3.arg(124);
        }
      else if (freq / 1000000. > 5590.)
        {
          temp3 = temp3.arg(120);
        }
      else if (freq / 1000000. > 5570.)
        {
          temp3 = temp3.arg(116);
        }
      else if (freq / 1000000. > 5550.)
        {
          temp3 = temp3.arg(112);
        }
      else if (freq / 1000000. > 5530.)
        {
          temp3 = temp3.arg(108);
        }
      else if (freq / 1000000. > 5510.)
        {
          temp3 = temp3.arg(104);
        }
      else if (freq / 1000000. > 5490.)
        {
          temp3 = temp3.arg(100);
        }
      else if (freq / 1000000. > 5310.)
        {
          temp3 = temp3.arg(64);
        }
      else if (freq / 1000000. > 5290.)
        {
          temp3 = temp3.arg(60);
        }
      else if (freq / 1000000. > 5270.)
        {
          temp3 = temp3.arg(56);
        }
      else if (freq / 1000000. > 5250.)
        {
          temp3 = temp3.arg(52);
        }
      else if (freq / 1000000. > 5230.)
        {
          temp3 = temp3.arg(48);
        }
      else if (freq / 1000000. > 5210.)
        {
          temp3 = temp3.arg(44);
        }
      else if (freq / 1000000. > 5190.)
        {
          temp3 = temp3.arg(40);
        }
      else if (freq / 1000000. > 5170.)
        {
          temp3 = temp3.arg(36);
        }
      else if (freq / 1000000. > 2474.)
	{
	  temp3 = temp3.arg (14);
	}
      else if (freq / 1000000. > 2469.)
	{
	  temp3 = temp3.arg (13);
	}
      else if (freq / 1000000. > 2464.)
	{
	  temp3 = temp3.arg (12);
	}
      else if (freq / 1000000. > 2459.)
	{
	  temp3 = temp3.arg (11);
	}
      else if (freq / 1000000. > 2454.)
	{
	  temp3 = temp3.arg (10);
	}
      else if (freq / 1000000. > 2449.)
	{
	  temp3 = temp3.arg (9);
	}
      else if (freq / 1000000. > 2444.)
	{
	  temp3 = temp3.arg (8);
	}
      else if (freq / 1000000. > 2439.)
	{
	  temp3 = temp3.arg (7);
	}
      else if (freq / 1000000. > 2434.)
	{
	  temp3 = temp3.arg (6);
	}
      else if (freq / 1000000. > 2429.)
	{
	  temp3 = temp3.arg (5);
	}
      else if (freq / 1000000. > 2424.)
	{
	  temp3 = temp3.arg (4);
	}
      else if (freq / 1000000. > 2419.)
	{
	  temp3 = temp3.arg (3);
	}
      else if (freq / 1000000. > 2414.)
	{
	  temp3 = temp3.arg (2);
	}
      else if (freq / 1000000. >= 2400.)
	{
	  temp3 = temp3.arg (1);
	}
      else
	temp3 = temp3.arg ("?");
      temp2 = temp2.arg (freq / 1000000000.);
    }

  if (freq < 1000000000)
    {if (freq >= 140.)
	{
	  temp2 = temp2.arg (5.700);
	}
      else if (freq >= 136.)
	{
	  temp2 = temp2.arg (5.680);
	}
      else if (freq >= 132.)
	{
	  temp2 = temp2.arg (5.660);
	}
      else if (freq >= 128.)
	{
	  temp2 = temp2.arg (5.640);
	}
      else if (freq >= 124.)
	{
	  temp2 = temp2.arg (5.620);
	}
      else if (freq >= 120.)
	{
	  temp2 = temp2.arg (5.600);
	}
      else if (freq >= 116.)
	{
	  temp2 = temp2.arg (5.580);
	}
      else if (freq >= 112.)
	{
	  temp2 = temp2.arg (5.560);
	}
      else if (freq >= 108.)
	{
	  temp2 = temp2.arg (5.540);
	}
      else if (freq >= 104.)
	{
	  temp2 = temp2.arg (5.520);
	}
      else if (freq >= 100.)
	{
	  temp2 = temp2.arg (5.500);
	}
      else if (freq >= 64.)
	{
	  temp2 = temp2.arg (5.320);
	}
      else if (freq >= 60.)
	{
	  temp2 = temp2.arg (5.300);
	}
      else if (freq >= 56.)
	{
	  temp2 = temp2.arg (5.280);
	}
      else if (freq >= 52.)
	{
	  temp2 = temp2.arg (5.260);
	}
      else if (freq >= 48.)
	{
	  temp2 = temp2.arg (5.240);
	}
      else if (freq >= 44.)
	{
	  temp2 = temp2.arg (5.220);
	}
      else if (freq >= 40.)
	{
	  temp2 = temp2.arg (5.200);
	}
      else if (freq >= 36.)
	{
	  temp2 = temp2.arg (5.180);
	}
      else if (freq >= 14.)
	{
	  temp2 = temp2.arg (2.484);
	}
      else if (freq >= 13.)
	{
	  temp2 = temp2.arg (2.472);
	}
      else if (freq >= 12.)
	{
	  temp2 = temp2.arg (2.467);
	}
      else if (freq >= 11.)
	{
	  temp2 = temp2.arg (2.462);
	}
      else if (freq >= 10.)
	{
	  temp2 = temp2.arg (2.457);
	}
      else if (freq >= 9.)
	{
	  temp2 = temp2.arg (2.452);
	}
      else if (freq >= 8.)
	{
	  temp2 = temp2.arg (2.447);
	}
      else if (freq >= 7.)
	{
	  temp2 = temp2.arg (2.442);
	}
      else if (freq >= 6.)
	{
	  temp2 = temp2.arg (2.437);
	}
      else if (freq >= 5.)
	{
	  temp2 = temp2.arg (2.432);
	}
      else if (freq >= 4.)
	{
	  temp2 = temp2.arg (2.427);
	}
      else if (freq >= 3.)
	{
	  temp2 = temp2.arg (2.422);
	}
      else if (freq >= 2.)
	{
	  temp2 = temp2.arg (2.417);
	}
      else if (freq >= 1.)
	{
	  temp2 = temp2.arg (2.412);
	}
      else
	temp2 = temp2.arg ("?");
      temp3 = temp3.arg (freq);
    }

  fr = new QLabel ( has_freq ? temp2 + " [" + temp3 + "]" : "" , content);
  if (geteuid () == 0)
    {
      enc_str = new QLabel (i18n ("Encryption: "), content);
      QString
	key;
      int
	size = 0,
	flags = 0;
      if (device->get_key (key, size, flags))
	// if ((device->key_flags & IW_ENCODE_DISABLED) || (key_size == 0))
	// will the following line suffice?
	if (size == 0)
	  {
	    enc = new QLabel (i18n ("off"), content);
	  }
	else
	  enc = new QLabel (i18n ("active"), content);
    }
  // content->setFixedSize (content->sizeHint ());
  content->setMinimumSize(content->sizeHint());
  if ( content->minimumWidth() < 280 ) content->setMinimumWidth( 280 );
  setFixedSize (content->minimumSize());
  content->show ();
  widgetbild = QPixmap::grabWidget (content);
  delete
    content;
  return true;
}

void
Status::paintEvent (QPaintEvent *)
{
  QPainter *paintArea = new QPainter (this);
  generateStatsWidget ();
  paintArea->drawPixmap (0, 0, widgetbild);
  delete paintArea;
}
