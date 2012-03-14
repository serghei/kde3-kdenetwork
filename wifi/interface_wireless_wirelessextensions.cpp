/***************************************************************************
                          interface_wireless_wirelessextensions.cpp  -  description
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

#include "interface_wireless_wirelessextensions.h"
#include <iwlib.h>
#include <qdir.h>
#include <qfile.h>
#include <qtable.h>

#include <kmessagebox.h>

#ifndef WITHOUT_ARTS
#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/iomanager.h>
#include <arts/referenceclean.h>
#endif

#include <iostream>
#include <string>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kprocio.h>
#include <kdebug.h>
#include <qstring.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


// domi:the wireless-tools people managed to change their interfaces
// around.  Someone with more knowledge of this code should look into
// moving to use the new iw_range interfaces.
#ifdef HAVE_IW_27

#ifndef HAVE_IW_27pre19
#define WIFI_CONFIG(config,field) config.field
#define WIFI_EXTRACT_EVENT_STREAM(stream, event) \
	iw_extract_event_stream(stream, event)
#else
#define WIFI_CONFIG(config,field) config.b.field
#define WIFI_EXTRACT_EVENT_STREAM(stream, event) \
	iw_extract_event_stream(stream, event, WIRELESS_EXT)
#endif

#define WIFI_GET_STATS(skfd, ifname, stats) \
	iw_get_stats (skfd, ifname, stats, 0, 0)
#define WIFI_PRINT_STATS(buff, event) \
        iw_print_stats(buffer, sizeof(buffer), &event->u.qual, 0, 0);

#else // HAVE_IW_25
#define WIFI_CONFIG(config,field) config.field
#define WIFI_EXTRACT_EVENT_STREAM(stream, event) \
	iw_extract_event_stream(stream, event)
#define WIFI_GET_STATS(skfd, ifname, stats) \
	iw_get_stats (skfd, ifname, stats)
#define WIFI_PRINT_STATS(buff, event) \
        iw_print_stats(buffer, &event->u.qual, 0, 0);
#endif

/* ================================== FROM IWCONFIG.C ================================== */
int
Interface_wireless_wirelessextensions::get_info (int skfd, const QString& interface,
						 struct wireless_info& info)
{
  struct iwreq wrq;
  char ifname[20];
  snprintf(ifname, sizeof(ifname), "%s", interface.latin1() );
  memset (&info, 0, sizeof(info));

  /* Get wireless name */
  if (iw_get_ext (skfd, ifname, SIOCGIWNAME, &wrq) < 0)
    {
      /* If no wireless name : no wireless extensions */
      /* But let's check if the interface exists at all */
      struct ifreq ifr;

      strcpy (ifr.ifr_name, ifname);
      if (ioctl (skfd, SIOCGIFFLAGS, &ifr) < 0)
	return (-ENODEV);
      else
	return (-ENOTSUP);
    }
  else
    {
      strncpy (WIFI_CONFIG(info,name), wrq.u.name, IFNAMSIZ);
      WIFI_CONFIG(info,name)[IFNAMSIZ] = '\0';
    }

  /* Get ranges */
  if (iw_get_range_info (skfd, ifname, &(info.range)) >= 0)
    info.has_range = 1;

  /* Get network ID */
  if (iw_get_ext (skfd, ifname, SIOCGIWNWID, &wrq) >= 0)
    {
      WIFI_CONFIG(info,has_nwid) = 1;
      memcpy (&(WIFI_CONFIG(info,nwid)), &(wrq.u.nwid), sizeof (iwparam));
    }

  /* Get frequency / channel */
  if (iw_get_ext (skfd, ifname, SIOCGIWFREQ, &wrq) >= 0)
    {
      WIFI_CONFIG(info,has_freq) = 1;
      WIFI_CONFIG(info,freq) = iw_freq2float (&(wrq.u.freq));
    }

  /* Get sensitivity */
  if (iw_get_ext (skfd, ifname, SIOCGIWSENS, &wrq) >= 0)
    {
      info.has_sens = 1;
      memcpy (&(info.sens), &(wrq.u.sens), sizeof (iwparam));
    }

  /* Get encryption information */
  wrq.u.data.pointer = (caddr_t) WIFI_CONFIG(info,key);
  wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
  wrq.u.data.flags = 0;
  if (iw_get_ext (skfd, ifname, SIOCGIWENCODE, &wrq) >= 0)
    {
      WIFI_CONFIG(info,has_key) = 1;
      WIFI_CONFIG(info,key_size) = wrq.u.data.length;
      WIFI_CONFIG(info,key_flags) = wrq.u.data.flags;
    }

  /* Get ESSID */
  /* prepare NULL-terminated buffer in case the string returned by SIOCGIWESSID is NOT NULL-terminated */
  memset(wrq.u.essid.pointer, '\0', IW_ESSID_MAX_SIZE + 1);
  wrq.u.essid.pointer = (caddr_t) WIFI_CONFIG(info,essid);
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if (iw_get_ext (skfd, ifname, SIOCGIWESSID, &wrq) >= 0)
    {
      WIFI_CONFIG(info,has_essid) = 1;
      WIFI_CONFIG(info,essid_on) = wrq.u.data.flags;
    }

  /* Get AP address */
  if (iw_get_ext (skfd, ifname, SIOCGIWAP, &wrq) >= 0)
    {
      info.has_ap_addr = 1;
      memcpy (&(info.ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
    }

  /* Get NickName */
  wrq.u.essid.pointer = (caddr_t) info.nickname;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if (iw_get_ext (skfd, ifname, SIOCGIWNICKN, &wrq) >= 0)
    if (wrq.u.data.length > 1)
      info.has_nickname = 1;

  /* Get bit rate */
  if (iw_get_ext (skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
    {
      info.has_bitrate = 1;
      memcpy (&(info.bitrate), &(wrq.u.bitrate), sizeof (iwparam));
    }

  /* Get RTS threshold */
  if (iw_get_ext (skfd, ifname, SIOCGIWRTS, &wrq) >= 0)
    {
      info.has_rts = 1;
      memcpy (&(info.rts), &(wrq.u.rts), sizeof (iwparam));
    }

  /* Get fragmentation threshold */
  if (iw_get_ext (skfd, ifname, SIOCGIWFRAG, &wrq) >= 0)
    {
      info.has_frag = 1;
      memcpy (&(info.frag), &(wrq.u.frag), sizeof (iwparam));
    }

  /* Get operation mode */
  if (iw_get_ext (skfd, ifname, SIOCGIWMODE, &wrq) >= 0)
    {
      WIFI_CONFIG(info,mode) = wrq.u.mode;
      if ((WIFI_CONFIG(info,mode) < IW_NUM_OPER_MODE) && (WIFI_CONFIG(info,mode) >= 0))
	WIFI_CONFIG(info,has_mode) = 1;
    }

  /* Get Power Management settings */
  wrq.u.power.flags = 0;
  if (iw_get_ext (skfd, ifname, SIOCGIWPOWER, &wrq) >= 0)
    {
      info.has_power = 1;
      memcpy (&(info.power), &(wrq.u.power), sizeof (iwparam));
    }

#if WIRELESS_EXT > 9
  /* Get Transmit Power and check if it is disabled */
  bool emitTXPowerChanged = false;
  if (iw_get_ext (skfd, ifname, SIOCGIWTXPOW, &wrq) >= 0)
    {
      if (txpower_disabled != info.txpower.disabled)
        {
	  emitTXPowerChanged = true;
        }

      info.has_txpower = 1;
      memcpy (&(info.txpower), &(wrq.u.txpower), sizeof (iwparam));
    }
  has_txpower = info.has_txpower;
  txpower_disabled = ( int )info.txpower.disabled;
  if (emitTXPowerChanged)
    {
      emit txPowerChanged();
    }
#endif

#if WIRELESS_EXT > 10
  /* Get retry limit/lifetime */
  if (iw_get_ext (skfd, ifname, SIOCGIWRETRY, &wrq) >= 0)
    {
      info.has_retry = 1;
      memcpy (&(info.retry), &(wrq.u.retry), sizeof (iwparam));
    }
#endif /* WIRELESS_EXT > 10 */

  /* Get stats */
  if (WIFI_GET_STATS (skfd, (char *) ifname, &(info.stats)) >= 0)
    {
      info.has_stats = 1;
    }

  return (0);
}

/* ================================ END IWCONFIG.C ================================ */


Interface_wireless_wirelessextensions::Interface_wireless_wirelessextensions (QStringList * ignoreInterfaces)
		: Interface_wireless(ignoreInterfaces)
{
}

bool Interface_wireless_wirelessextensions::isSocketOpen()
{
  if (socket <= 0)
    socket = iw_sockets_open ();

  return (socket > 0);
}

void Interface_wireless_wirelessextensions::setActiveDevice( QString device )
{
  kdDebug () << "activating wireless device " << device << endl;
  interface_name = device;

  if (!isSocketOpen())
	return;

  emit interfaceChanged ();
  emit strengthChanged ();
  emit statusChanged ();
  emit modeChanged ();
  emit speedChanged ();
  valid[current] = false;
  emit statsUpdated ();
}

void Interface_wireless_wirelessextensions::setNoActiveDevice( )
{
  // interface has disappeared - unplugged?
  // reset all info except stats
  has_frequency = false;
  frequency = 0.0;
  has_mode = false;
  mode = 0;
  has_key = 0;
  key = QString::null;
  key_size = 0;
  key_flags = 0;
  essid = QString::null;
  access_point_address = QString::null;
  ip_address = QString::null;
  bitrate = 0;
  has_range = false;
  has_txpower = false;
  txpower_disabled = 0;

  // propagate the changes
  setActiveDevice( QString::null );
}

QStringList Interface_wireless_wirelessextensions::available_wifi_devices()
{
  QFile procnetdev(PROC_NET_DEV);
  procnetdev.open (IO_ReadOnly);

  kdDebug () << "Autodetecting..." << endl;

  QStringList liste;
  QString device;
  while (!procnetdev.atEnd()) {
	  procnetdev.readLine (device, 9999);
	  int pos = device.find (':');
	  if (pos == -1)
		continue;
	  device = device.left(pos).stripWhiteSpace();
	  if (device.isEmpty())
		continue;
	// Some drivers create two interfaces, ethX and wifiX.  The
	// wifiX device are not useful to users because they only
	// control the physical wlan settings, not the logical ones.  If
	// we find a wifiX device then we move it to the start of the
	// list which means the first instance of KWifiManger will pick
	// up the ethX and only the second instance will pick up the
	// wifiX.
	  if (device.startsWith("wifi"))
		liste.prepend(device);
	  else
		liste.append(device);	
  }


  if (liste.isEmpty())
	kdDebug () << "No wireless interface found." << endl;

  return liste;
}

bool Interface_wireless_wirelessextensions::autodetect_device()
{
  if (!isSocketOpen())
	return false;

  QStringList liste = available_wifi_devices();

  for (QStringList::Iterator it = liste.begin (); it != liste.end (); ++it)
  {
	QString device = *it;
	kdDebug () << "[ " << device << " ] " << endl;
  	wireless_info info;
	int result = get_info (socket, device, info);
	if ((result != -ENODEV) && (result != -ENOTSUP) &&
	    (!ignoreInterfaces || ignoreInterfaces->findIndex(device)==-1))
	{
		setActiveDevice(device);
	}
   }

   if (interface_name.isEmpty())
   {
	close(socket);
	socket = -1;
	return false;
   }

   return true;
}

bool Interface_wireless_wirelessextensions::poll_device_info ()
{
  if (current < MAX_HISTORY-1)
	++current;
    else
	current = 0;

  if (interface_name.isEmpty())
	if (!autodetect_device())
		return false;

  wireless_info	info;
  int result = get_info (socket, interface_name, info);
  if ((result == -ENODEV) || (result == -ENOTSUP))
	{
	  // interface has disappeared - unplugged?
	  // reset all info except stats
	  setNoActiveDevice();

	  close(socket);
	  socket = -1;

	  return false;
	}

  bool
    emitStatusChanged = false, emitModeChanged = false, emitEssidChanged =
    false, emitSpeedChanged = false, emitStrengthChanged = false;

  iwstats	tempic2;
  WIFI_GET_STATS (socket, (char *) interface_name.latin1(), &tempic2);
  has_frequency = WIFI_CONFIG(info,has_freq);
  if (has_frequency)
    {
      if (frequency != WIFI_CONFIG(info,freq))
	emitStatusChanged = true;
      frequency = WIFI_CONFIG(info,freq);
    }
  has_mode = WIFI_CONFIG(info,has_mode);
  if (has_mode)
    {
      if (mode != WIFI_CONFIG(info,mode))
	emitModeChanged = true;
      mode = WIFI_CONFIG(info,mode);
    }
  has_key = WIFI_CONFIG(info,has_key);
  if (has_key)
    {
      if ((key != (char *) WIFI_CONFIG(info,key)) || (key_size != WIFI_CONFIG(info,key_size))
	  || (key_flags != WIFI_CONFIG(info,key_flags)))
	emitStatusChanged = true;
      key = (char *) WIFI_CONFIG(info,key);
      key_size = WIFI_CONFIG(info,key_size);
      key_flags = WIFI_CONFIG(info,key_flags);
    }
  if (essid != WIFI_CONFIG(info,essid))
    {
      emitStatusChanged = true;
      emitEssidChanged = true;
    }
  essid = (WIFI_CONFIG(info,essid_on) ? WIFI_CONFIG(info,essid) : "any");
  char
    ap_addr[256];
  iw_ether_ntop ( (const ether_addr *) info.ap_addr.sa_data, ap_addr);
  if (access_point_address != ap_addr)
    emitStatusChanged = true;
  access_point_address = ap_addr;
  if (bitrate != info.bitrate.value)
    emitSpeedChanged = true;
  bitrate = info.bitrate.value;
  has_range = info.has_range;

  sigLevel[current] = tempic2.qual.level;
  noiseLevel[current] = tempic2.qual.noise;

  if (has_range) {

    range = info.range.max_qual.qual;
  
    // the following checks adjust values for sig levels that are in dBm
    // according to iwlib.c. If offset is applied, the resulting
    // value is negative and the GUI knows it can add the scale
    // unit dBm. 

    if (tempic2.qual.level > info.range.max_qual.level) {
  	  sigLevel[current] = tempic2.qual.level - 0x100;
          noiseLevel[current] = tempic2.qual.noise - 0x100;
    }
  }

  if (sigLevel[current] < sigLevelMin)
    sigLevelMin = sigLevel[current];
  if (sigLevel[current] > sigLevelMax)
    sigLevelMax = sigLevel[current];

  if (noiseLevel[current] < noiseLevelMin)
    noiseLevelMin = noiseLevel[current];
  if (noiseLevel[current] > noiseLevelMax)
    noiseLevelMax = noiseLevel[current];
  int
    tempqual = tempic2.qual.qual;
  if (has_range && (range != 0))
    tempqual = tempqual * 100 / range;
  if (      qual[current != 0 ? current - 1 : MAX_HISTORY-1] != tempqual)
    emitStrengthChanged = true;

  qual[current] = ( tempqual <= 100 ? tempqual : 100 );
  valid[current] = true;

  // try to get our local IP address
  struct sockaddr *
    sa;
  struct sockaddr_in *
    sin;
  struct ifreq
    ifr;
  /* Copy the interface name into the buffer */
  strncpy (ifr.ifr_name, interface_name.latin1 (), IFNAMSIZ);

  if (ioctl (socket, SIOCGIFADDR, &ifr) == -1)
    {
      if (ip_address != "unavailable")
	emitStatusChanged = true;
      ip_address = "unavailable";
    }
  /* Now the buffer will contain the information we requested */
  sa = (struct sockaddr *) &(ifr.ifr_addr);
  if (sa->sa_family == AF_INET)
    {
      sin = (struct sockaddr_in *) sa;
      if (ip_address != (QString) inet_ntoa (sin->sin_addr))
	emitStatusChanged = true;
      ip_address = (QString) inet_ntoa (sin->sin_addr);
    }
  else
    {
      ip_address = "unavailable";
    }
  if (emitStatusChanged)
    emit statusChanged ();
  if (emitStrengthChanged)
    emit strengthChanged ();
  if (emitModeChanged)
    emit modeChanged ();
  if (emitSpeedChanged)
    emit speedChanged ();
  if (emitEssidChanged)
    emit essidChanged (essid);
  emit statsUpdated ();
  return true;
}

QTable* Interface_wireless_wirelessextensions::get_available_networks ()
{

    networks = new QTable(0,4,0);
    networks->horizontalHeader()->setLabel( 0, i18n("Network Name") );
    networks->horizontalHeader()->setLabel( 1, i18n("Mode") );
    networks->horizontalHeader()->setLabel( 2, i18n("Quality") );
    networks->horizontalHeader()->setLabel( 3, i18n("WEP") );

    KProcIO *iwlist = new KProcIO;
    QString iwlist_bin = KStandardDirs::findExe("iwlist");
    if(iwlist_bin.isEmpty())
      iwlist_bin = KStandardDirs::findExe("iwlist", "/usr/local/sbin:/usr/sbin:/sbin");
    if(iwlist_bin.isEmpty())
      iwlist_bin = "iwlist"; // try our best ;/

    *iwlist << iwlist_bin << interface_name << "scanning";

//    connect ( iwlist, SIGNAL ( readReady ( KProcIO * ) ), this, SLOT ( parseScanData ( KProcIO * ) ) );
	    
   if ( !iwlist->start ( KProcess::Block ) )
        KMessageBox::sorry ( 0, i18n ( "Unable to perform the scan. Please make sure the executable \"iwlist\" is in your $PATH." ),
				i18n ( "Scanning not possible" ) );

    // this should never happen. But there was a report about Block not being as blocking as it should, so let's be safe about it
    while (iwlist->isRunning()) sleep ( 1 );

    parseScanData ( iwlist );
   
    for (int i = 0; i<4; i++ ) networks->adjustColumn(i);

    return networks;
}

void
Interface_wireless_wirelessextensions::parseScanData ( KProcIO * iwlist )
{
  QString data;
  int cellcount = 0, iteratecount = 0;
  bool ignoreRemainingBits = false;
  while ( iwlist->readln ( data, true ) >= 0 )
    {
      kdDebug (  ) << "iwlist: " << data << "\n";
      if ( data.contains ( "does not support scanning" ) )
        KMessageBox::sorry ( 0,
                             i18n
                             ( "Your card does not support scanning. The results window will not contain any results." ),
                             i18n ( "Scanning not possible" ) );
      if ( data.contains ( "Scan completed" ) )
        cellcount = 0;          // at the very beginning of a scan
      if ( data.contains ( "Cell" ) ) {
        cellcount++;            // new cell discovered
	networks->setNumRows ( networks->numRows() +1 );
	ignoreRemainingBits = false;
      }
      if ( data.contains ( "ESSID:" ) )
        {
          QString ssid = data.mid ( data.find ( "\"" ) + 1, data.length (  ) - data.find ( "\"" ) - 2 );
	  if ((ssid=="") || (ssid==" ")) ssid = "(hidden cell)";
          networks->setText ( cellcount - 1, 0, ssid );
        }
      if ( data.contains ( "Mode:" ) )
        {
          if ( data.contains ( "Master" ) )
            networks->setText ( cellcount - 1, 1, QString ( i18n ( "Managed" ) ) );
          if ( data.contains ( "Ad-Hoc" ) )
            networks->setText ( cellcount - 1, 1, QString ( i18n ( "Ad-Hoc" ) ) );
	  // if could be that this cell belongs to an SSID already discovered, or that there are more than one
	  // hidden cells, which doesn't give any new information. So, we first search for duplicates and delete 
	  // this row if it's a duplicate. If the same SSID is there once as Managed and once as Ad-Hoc it is no
	  // duplicate of course, but "something completely different"
	  for (iteratecount = 0; iteratecount < cellcount - 1; iteratecount++)
	  {
	      kdDebug() << "Comparing <" << networks->text(cellcount - 1, 0) << "," << networks->text(iteratecount,0) << " and <"
		        << networks->text(cellcount - 1, 1) << "," << networks->text(iteratecount,1) << ">.\n";
	      if ((networks->text(cellcount - 1 , 0) == networks->text(iteratecount,0)) &&
	          (networks->text(cellcount - 1 , 1) == networks->text(iteratecount,1)) )
	      {
	           networks->setNumRows ( networks->numRows() - 1 );
	           cellcount--;
		   ignoreRemainingBits = true;
	      }
	  }
        }
      if ( !ignoreRemainingBits && data.contains ( "Encryption key:" ) )
        {
          if ( data.contains ( "off" ) )
            networks->setText ( cellcount - 1, 3, QString ( "off" ) );
          else
            networks->setText ( cellcount - 1, 3, QString ( "on" ) );
        }
      if ( !ignoreRemainingBits && data.contains ( "Quality:" ) )
        {
          QString quality = data.mid ( data.find ( ":" ) + 1, data.find ( "/" ) - data.find ( ":" ) - 1 );
          networks->setText ( cellcount - 1, 2, quality );
        }
      
      if ( !ignoreRemainingBits && data.contains ( "Quality=" ) )
        {
          QString quality = data.mid ( data.find ( "=" ) + 1, data.find ( "/" ) - data.find ( "=" ) - 1 );
          networks->setText ( cellcount - 1, 2, quality );
        }
      
      if ( !ignoreRemainingBits && data.contains ( "wpa_ie" ) )
	{
	  networks->setText ( cellcount - 1, 3, QString ( "WPA" ) );
	}
      
      if ( !ignoreRemainingBits && data.contains ( "rsn_ie" ) )
        {
          networks->setText ( cellcount - 1, 3, QString ( "WPA2" ) );
        }
      
    }
}

#include "interface_wireless_wirelessextensions.moc"
