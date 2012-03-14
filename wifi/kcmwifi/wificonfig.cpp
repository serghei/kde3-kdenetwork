/*
  Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>

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

#include <qfile.h>

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kprocio.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "wificonfig.h"

static QStringList speedList;
static QStringList wifiModeList;
static QStringList cryptoModeList;
static QStringList powerModeList;

Key::Key()
{
}

Key::Key( const QString &key )
{
  setKey( key );
}

QString Key::rawKey() const
{
  QString s = m_key;

  if ( isValid( m_key ) >= STRING_64 )
    s = "s:" + s;

  return s;
}

void Key::setKey( const QString &key )
{
    m_key = key;
}

KeyStates Key::isValid ( QString keyCandidate )
{ 
  if (keyCandidate.isEmpty()) {
	kdDebug() << "Ignoring key: empty.\n";
	return EMPTY; 
  }

  if (keyCandidate.length() == 5)
	return STRING_64;

  if (keyCandidate.length() == 10)
	return HEX_64;

  if (keyCandidate.length() == 13)
	return STRING_128;

  if (keyCandidate.length() == 26)
	return HEX_128;

  if (keyCandidate.length() == 32)
	return STRING_256;

  if (keyCandidate.length() == 64)
	return HEX_256;
  
  return INVALID;
}

IfConfig::IfConfig()
{

  // initialise all config items...

  m_networkName = "";
  m_interface = "";
  m_wifiMode = Managed;
  m_speed = AUTO;
  m_runScript = false;
  m_connectScript = "";

  m_useCrypto = false;
  m_cryptoMode = Open;
  m_activeKey = 1;
  for (int i=0; i<4; i++)
    m_keys[ i ].setKey( "" );

  m_pmEnabled = false;
  m_pmMode = AllPackets;
  m_sleepTimeout = 1;
  m_wakeupPeriod = 1;

  speedList << "Auto" << "1M" << "2M" << "5.5M" << "6M" << "9M" << "11M" << "12M" << "18M" << "24M" << "36M" << "48M" << "54M";
  wifiModeList << "Ad-Hoc" << "Managed" << "Repeater"
               << "Master" << "Secondary";
  cryptoModeList << "Open" << "Restricted";
  powerModeList << "All" << "UnicastOnly" << "MulticastOnly";
}

void IfConfig::load( KConfig *config, int i )
{
  QString entry;

  QString group = QString( "Configuration %1" ).arg( i+1 );
  config->setGroup( group );

  m_networkName = config->readEntry( "NetworkName" );
  m_interface = config->readEntry( "InterfaceName" );

  entry = config->readEntry( "WifiMode", "Managed" );
  wifimodeFromString( entry );
  entry =  config->readEntry( "Speed", "Auto" );
  speedFromString( entry );
  m_runScript = config->readBoolEntry( "RunScript", false );
  m_connectScript = config->readEntry( "ScriptName" );

  m_useCrypto = config->readBoolEntry( "UseCrypto", false );
  entry = config->readEntry( "CryptoMode", "Open" );
  cryptomodeFromString( entry );
  m_activeKey = config->readNumEntry( "ActiveKey", 1 );
  m_keys[ 0 ].setKey( config->readEntry( "Key1" ) );
  m_keys[ 1 ].setKey( config->readEntry( "Key2" ) );
  m_keys[ 2 ].setKey( config->readEntry( "Key3" ) );
  m_keys[ 3 ].setKey( config->readEntry( "Key4" ) );

  m_pmEnabled = config->readBoolEntry( "PMEnabled", false );
  entry = config->readEntry( "PMMode", "All" );
  powermodeFromString( entry );
  m_sleepTimeout = config->readNumEntry( "SleepTimeout", 30 );
  m_wakeupPeriod = config->readNumEntry( "WakeupPeriod", 20 );
}

void IfConfig::save( KConfig *config, int i )
{
  QString group = QString( "Configuration %1" ).arg( i+1 );
  config->setGroup( group );

  config->writeEntry( "NetworkName", m_networkName );
  config->writeEntry( "InterfaceName", m_interface );
  config->writeEntry( "WifiMode", wifimodeAsString() );
  config->writeEntry( "Speed", speedAsString() );
  config->writeEntry( "RunScript", m_runScript );
  config->writeEntry( "ScriptName", m_connectScript );

  config->writeEntry( "UseCrypto", m_useCrypto );
  config->writeEntry( "CryptoMode", cryptomodeAsString() );
  config->writeEntry( "ActiveKey", m_activeKey );
  config->writeEntry( "Key1", m_keys[ 0 ].key() );
  config->writeEntry( "Key2", m_keys[ 1 ].key() );
  config->writeEntry( "Key3", m_keys[ 2 ].key() );
  config->writeEntry( "Key4", m_keys[ 3 ].key() );

  config->writeEntry( "PMEnabled", m_pmEnabled );
  config->writeEntry( "PMMode", powermodeAsString() );
  config->writeEntry( "SleepTimeout", m_sleepTimeout );
  config->writeEntry( "WakeupPeriod", m_wakeupPeriod );
}

WifiConfig *WifiConfig::m_instance = 0;

WifiConfig *WifiConfig::instance()
{
  if ( m_instance == 0 )
    m_instance = new WifiConfig();

  return m_instance;
}

WifiConfig::WifiConfig()
{
  //m_config = KGlobal::config();
  m_config = new KSimpleConfig( "kcmwifirc" );

  load();
}

WifiConfig::~WifiConfig()
{
		delete m_config;
}

void WifiConfig::load()
{
  m_config->setGroup( "General" );

  m_usePreset = m_config->readBoolEntry( "UsePreset", false );
  m_presetConfig = m_config->readNumEntry( "PresetConfig", 1 )-1;
  m_numConfigs = m_config->readNumEntry( "NumberConfigs", 4 );

  for ( int i=0; i<m_numConfigs; ++i )
  {
    m_ifConfig[ i ].load( m_config, i );
  }
}

void WifiConfig::save()
{
  m_config->setGroup( "General" );

  m_config->writeEntry( "UsePreset", m_usePreset );
  m_config->writeEntry( "PresetConfig", m_presetConfig+1 );
  m_config->writeEntry( "NumberConfigs", m_numConfigs );

  for ( int i=0; i<m_numConfigs; ++i )
  {
    m_ifConfig[ i ].save( m_config, i );
  }

  m_config->sync();
}

QString WifiConfig::autoDetectInterface()
{
  m_detectedInterface.truncate( 0 );

  QFile procFile ( "/proc/net/dev" );

  if ( !procFile.open( IO_ReadOnly ) )
  {
    KMessageBox::sorry( 0,
        i18n( "Unable to autodetect wireless interface." ) );
    return m_detectedInterface;
  }

  QStringList list;
  QString line;

  while ( !procFile.atEnd() )
  {
    procFile.readLine( line, 9999 );
    if ( line.find( ":" ) > 0 )
    {
      line.truncate ( line.find( ":" ) );
      list.append( line.stripWhiteSpace() );
    }
  }

  procFile.close();

  if ( list.empty() )
  {
    KMessageBox::sorry( 0,
        i18n( "Unable to autodetect wireless interface." ) );
    return m_detectedInterface;
  }

  for ( QStringList::Iterator it = list.begin (); it != list.end (); ++it )
  {
    if ( ((*it).contains("wifi"))==0 ) { // if the name is wifiX, ignore
      KProcIO test;
      test << "iwconfig";
      test << *it;
      connect( &test, SIGNAL( readReady( KProcIO * ) ),
               this, SLOT( slotTestInterface( KProcIO * ) ) );
      test.start ( KProcess::Block );
    }
  }

  if ( m_detectedInterface.isEmpty() )
  {
    KMessageBox::sorry( 0,
        i18n( "Unable to autodetect wireless interface." ) );
    return m_detectedInterface;
  }

  return m_detectedInterface;
}

void WifiConfig::slotTestInterface( KProcIO *proc )
{
  QString output;
  proc->readln( output );
  if ( output.find ( "no wireless extensions" ) == -1 )
  {
    output.truncate( output.find ( " " ) );
    m_detectedInterface = output.stripWhiteSpace();
  }
}

QString IfConfig::speedAsString()
{
  return speedList[ m_speed ];
}

QString IfConfig::wifimodeAsString()
{
  return wifiModeList[ m_wifiMode ];
}

QString IfConfig::cryptomodeAsString()
{
  return cryptoModeList[ m_cryptoMode ];
}

QString IfConfig::powermodeAsString()
{
  return powerModeList[ m_pmMode ];
}

int IfConfig::activeKeyId()
{
  return m_activeKey;
}

Key IfConfig::activeKey()
{
  return m_keys[ m_activeKey-1 ];
}

IfConfig::Speed IfConfig::convertToSpeedFromString( const QString &s )
{
  return ( IfConfig::Speed )speedList.findIndex( s );
}

void IfConfig::speedFromString( const QString &s )
{
  m_speed = convertToSpeedFromString( s );
}

IfConfig::WifiMode IfConfig::convertToWifiModeFromString( const QString &s )
{
  return ( IfConfig::WifiMode )wifiModeList.findIndex( s );
}

void IfConfig::wifimodeFromString( const QString &s )
{
  m_wifiMode = convertToWifiModeFromString( s );
}

IfConfig::CryptoMode IfConfig::convertToCryptoModeFromString( const QString &s )
{
  return ( IfConfig::CryptoMode )cryptoModeList.findIndex( s );
}

void IfConfig::cryptomodeFromString( const QString &s )
{
  m_cryptoMode = convertToCryptoModeFromString( s );
}

IfConfig::PowerMode IfConfig::convertToPowerModeFromString( const QString &s )
{
  return ( IfConfig::PowerMode )powerModeList.findIndex( s );
}

void IfConfig::powermodeFromString( const QString &s )
{
  m_pmMode = convertToPowerModeFromString( s );
}

#include "wificonfig.moc"

