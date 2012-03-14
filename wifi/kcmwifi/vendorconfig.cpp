/*
  Copyright (C) 2005 Stefan Winter <swinter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qstringlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>

#include "kcmwifi.h"
#include "vendorconfig.h"
#include "ifconfigpage.h"
#include "configcrypto.h"
#include "wificonfig.h"

VendorConfig::VendorConfig ( KCMWifi * caller )
{
  m_caller = caller;
  m_totalConfigs = 0;
}

void
VendorConfig::initAll (  )
{
  initSuSE_92plus (  );
  initDebian (  );
}

void
VendorConfig::initSuSE_92plus (  )
{

  /* configurations are located in /etc/sysconfig/network/ifcfg-wlan*
     there can be more than one if there are multiple wireless interfaces */

  QDir filelist ( "/etc/sysconfig/network", "ifcfg-wlan*" );
  QStringList files = filelist.entryList (  );

  if ( files.count() == 0 )
  {
    return;
  }

  for ( QStringList::Iterator it = files.begin (  ); it != files.end (  ); ++it )
    {
      if ( ( ( *it ).endsWith ( "~" ) ) == false )
	{			// ignore backup files
	  QFile configfile ( QString ( "/etc/sysconfig/network/%1" ).arg ( *it ) );
	  configfile.open ( IO_ReadOnly );
	  kdDebug (  ) << "Opened SuSE configuration file " << configfile.name (  ) << "\n";

	  IfConfigPage *newtab = m_caller->addConfigTab ( 1, true );
	  /* this config tab gets automagically stored in an array, the first
	     starting at position vendorBase. We need to construct its power
	     and crypto settings and therefore need the array index, which is
	     vendorBase+m_totalConfigs. It is _essential_ that m_totalConfigs
	     is incremented _after_ successful setup of each given config. */

	  // init crypto entries
	  ConfigCrypto *crypto = new ConfigCrypto ( 0, "ConfigCrypto" );
	  WifiConfig *config = WifiConfig::instance (  );
	  IfConfig & ifconfig = config->m_ifConfig[KCMWifi::vendorBase + m_totalConfigs];

	  QString buffer;
	  while ( configfile.readLine ( buffer, 255 ) != -1 )
	    {
	      // lines look like WIRELESS_ESSID='bla', so split it into WIRELESS_ESSID and bla
	      QString var = buffer.section ( '=', 0, 0 );
	      QString value = buffer.section ( '=', 1, 1 );
	      value = value.mid ( 1, value.length (  ) - 3 );
	      kdDebug (  ) << "Variable " << var << " contains " << value << ".\n";
	      // evaluate the meaningful lines
	      if ( var == "WIRELESS_ESSID" )
		{
		  newtab->le_networkName->setText ( value );
		}
	      else if ( var == "WIRELESS_MODE" )
		{
		  newtab->cmb_wifiMode->setCurrentItem ( IfConfig::convertToWifiModeFromString ( value ) );
		}
	      else if ( var == "WIRELESS_BITRATE" )
		{
		  newtab->cmb_speed->setCurrentItem ( IfConfig::convertToSpeedFromString ( value ) );
		}
	      else if ( var == "WIRELESS_DEFAULT_KEY" )
		{
		  crypto->cmb_activeKey->setCurrentItem ( value.toInt (  ) );
		}
	      else if ( var == "WIRELESS_AUTH_MODE" )
		{
		  if ( value == "shared" || value == "sharedkey" || value == "restricted" )
		    {
		      crypto->rb_restrictMode->setChecked ( true );
		      newtab->cb_useCrypto->setChecked ( true );
		    }
		  if ( value == "open" || value == "opensystem" || value == "" )
		    {
		      crypto->rb_openMode->setChecked ( true );
		      newtab->cb_useCrypto->setChecked ( false );
		    }
		}
	      else if ( var == "WIRELESS_KEY_0" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key1->setText ( value );
		}
	      else if ( var == "WIRELESS_KEY_1" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key2->setText ( value );
		}
	      else if ( var == "WIRELESS_KEY_2" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key3->setText ( value );
		}
	      else if ( var == "WIRELESS_KEY_3" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key4->setText ( value );
		}
	    }
	  newtab->save (  );
	  crypto->save ( ifconfig );
	  configfile.close (  );
	  m_totalConfigs++;
	}
    }

}

void
VendorConfig::initDebian (  )
{
  QFile configfile ( QString ( "/etc/network/interfaces" ) );
  if ( !configfile.open ( IO_ReadOnly ) )
  {
    return;
  }

  kdDebug (  ) << "Opened Debian configuration file " << configfile.name (  ) << "\n";

  IfConfigPage *newtab = m_caller->addConfigTab ( 1, true );
/* this config tab gets automagically stored in an array, the first
starting at position vendorBase. We need to construct its power
and crypto settings and therefore need the array index, which is
vendorBase+m_totalConfigs. It is _essential_ that m_totalConfigs
is incremented _after_ successful setup of each given config. */
// init crypto entries
  ConfigCrypto *crypto = new ConfigCrypto ( 0, "ConfigCrypto" );
  WifiConfig *config = WifiConfig::instance (  );
  IfConfig & ifconfig = config->m_ifConfig[KCMWifi::vendorBase + m_totalConfigs];

  QString buffer;
  while ( configfile.readLine ( buffer, 255 ) != -1 )
    {
// lines look like wireless-defaultkey bla, so split it into wireless-defaultkey and bla
      QString var = buffer.section ( ' ', 0, 0 );
      QString value = buffer.section ( ' ', 1, 1 );
      kdDebug (  ) << "Variable " << var << " contains " << value << ".\n";
// evaluate the meaningful lines
      if ( var.startsWith ( "wireless", false ) )
	{
	  if ( var.endsWith ( "essid" ) )
	    {
	      newtab->le_networkName->setText ( value );
	    }

	  else if ( var.endsWith ( "mode" ) && !var.contains ( "key" ) )

	    {
	      newtab->cmb_wifiMode->setCurrentItem ( IfConfig::convertToWifiModeFromString ( value ) );
	    }

	  else if ( var.endsWith ( "rate" ) )

	    {
	      newtab->cmb_speed->setCurrentItem ( IfConfig::convertToSpeedFromString ( value ) );
	    }

	  else if ( var.contains ( "key" ) )
	    {
// Could be any of key, key1, key2, ..., or defaultkey
	      if ( var.contains ( "default" ) )
		{
		  crypto->cmb_activeKey->setCurrentItem ( value.toInt (  ) );
		}

	      else if ( var.contains ( "mode" ) )
		{
		  if ( value == "shared" || value == "sharedkey" || value == "restricted" )
		    {
		      crypto->rb_restrictMode->setChecked ( true );
		      newtab->cb_useCrypto->setChecked ( true );
		    }
		}

	      else if ( value == "open" || value == "opensystem" || value == "" )
		{
		  crypto->rb_openMode->setChecked ( true );
		  newtab->cb_useCrypto->setChecked ( false );
		}

	      else if ( var.right ( 1 ) == "1" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key1->setText ( value );
		}

	      else if ( var.right ( 1 ) == "2" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key2->setText ( value );
		}

	      else if ( var.right ( 1 ) == "3" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key3->setText ( value );
		}

	      else if ( var.right ( 1 ) == "4" )
		{
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key4->setText ( value );
		}

	      else
		{
// does not contain a number. It's the only one.
		  if ( value.startsWith ( "s:" ) )
		    value = value.right ( value.length (  ) - 2 );
		  crypto->le_key1->setText ( value );
		}
	    }
	}
    }
  newtab->save (  );
  crypto->save ( ifconfig );
  configfile.close (  );
  m_totalConfigs++;
}
