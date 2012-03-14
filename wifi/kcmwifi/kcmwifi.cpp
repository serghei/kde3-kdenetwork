/*
  Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>
            (C) 2001-2004 by Stefan Winter <mail@stefan-winter.de>

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

#include <qlayout.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocio.h>
#include <klineedit.h>

#include "kcmwifi.h"
#include "ifconfigpage.h"
#include "mainconfig.h"
#include "configcrypto.h"
#include "wificonfig.h"
#include "vendorconfig.h"

typedef KGenericFactory < KCMWifi, QWidget > KWiFiFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_wifi, KWiFiFactory("kcmwifi") )

KCMWifi::KCMWifi(QWidget * parent, const char *name, const QStringList &)
    : KCModule (parent, name)
{
  tabs = new QTabWidget (this, "tabs");

  m_mainConfig = new MainConfig(this, "m_mainConfig");

  WifiConfig *config = WifiConfig::instance();

  for ( int i=0; i<config->m_numConfigs; ++i )
  {
    IfConfigPage *ifConfigPage = new IfConfigPage( i, tabs, "m_configPage" );
    tabs->addTab(ifConfigPage, i18n ("Config &%1").arg( i+1 ));
    connect(ifConfigPage, SIGNAL(changed()), SLOT( slotChanged() ) );

    m_ifConfigPage[ i ] = ifConfigPage;
  }

  QVBoxLayout *top = new QVBoxLayout (this, 0, KDialog::spacingHint());
  top->addWidget( tabs );
  top->addWidget( m_mainConfig );
  top->addStretch();

  connect(m_mainConfig, SIGNAL(changed()), SLOT( slotChanged() ) );
  connect(m_mainConfig, SIGNAL(activateClicked()), SLOT(slotActivate()));

  if ( geteuid() != 0 )
  {
    tabs->setEnabled( false );
    m_mainConfig->setEnabled( false );
  }
  else
  {
    KProcess iwconfigtest;
    iwconfigtest << "iwconfig";
    if (!iwconfigtest.start(KProcess::DontCare))
    {
      KMessageBox::sorry(0, i18n("Error executing iwconfig. WLAN "
          "configurations can only be altered if the wireless tools are "
          "properly installed."), i18n ("No Wireless Tools"));

      tabs->setEnabled( false );
      m_mainConfig->setEnabled( false );
    }
  }

  load();

  m_activeVendorCount = 0;
  VendorConfig vendors( this );
  vendors.initAll();
}


KCMWifi::~KCMWifi()
{
}

void KCMWifi::load()
{
  WifiConfig *config = WifiConfig::instance();

  for ( int i=0; i<config->m_numConfigs; ++i )
    m_ifConfigPage[ i ]->load();

  m_mainConfig->load();
}

void KCMWifi::save()
{
  WifiConfig *config = WifiConfig::instance();
  for ( int i=0; i<config->m_numConfigs; ++i )
    m_ifConfigPage[ i ]->save();

  m_mainConfig->save();

  config->save();
}

void KCMWifi::slotActivate()
{
  save();
  activate();
}

void KCMWifi::defaults()
{
}

void KCMWifi::slotChanged()
{
  emit changed( true );
}

IfConfigPage* KCMWifi::addConfigTab( int count, bool vendor )
{

  WifiConfig *config = WifiConfig::instance();

  IfConfigPage* retval = NULL;
 
  if (vendor) 
  {
    IfConfigPage *ifConfigPage = new IfConfigPage( vendorBase + m_activeVendorCount, tabs, "m_configPage" );
    tabs->addTab(ifConfigPage, i18n ("Vendor %1").arg( m_activeVendorCount+1 ) );
    connect(ifConfigPage, SIGNAL(changed()), SLOT( slotChanged() ) );
    m_ifConfigPage[ vendorBase+m_activeVendorCount ] = ifConfigPage;
    m_mainConfig->registerConfig( vendorBase+m_activeVendorCount );
    m_activeVendorCount++;
    retval = ifConfigPage;

    // these configs are read-only, so disable all widgets
    // the content shall only be modified by VendorConfig code
    retval->cb_Autodetect->setEnabled( false );
    retval->le_networkName->setReadOnly( true );
    retval->cmb_wifiMode->setEnabled( false );
    retval->cmb_speed->setEnabled( false );
    retval->cb_runScript->setEnabled( false );
    retval->cb_useCrypto->setEnabled( false );
    retval->cb_pmEnabled->setEnabled( false );

  }
  else
  {
    for ( int i=config->m_numConfigs; i<config->m_numConfigs+count; i++ )
      {
        IfConfigPage *ifConfigPage = new IfConfigPage( i, tabs, "m_configPage" );
        tabs->insertTab(ifConfigPage, i18n ("Config &%1").arg( i+1 ), i );
        connect(ifConfigPage, SIGNAL(changed()), SLOT( slotChanged() ) );
        m_ifConfigPage[ i ] = ifConfigPage; 
        m_mainConfig->registerConfig( i+1 );
        retval = ifConfigPage;
      }
    config->m_numConfigs += count;
  }
 
  // if more than one was constructed, only the last is returned
  return retval;
}

void KCMWifi::delConfigTab( int count )
{
  WifiConfig *config = WifiConfig::instance();

  for ( int i=config->m_numConfigs-1; i>(config->m_numConfigs)-count-1; i-- )
  {
    tabs->setTabEnabled(m_ifConfigPage[ i ], false);
    m_ifConfigPage[ i ]->deleteLater();

    m_mainConfig->unregisterConfig( i+1 );

  }

  config->m_numConfigs -= count;

}

void KCMWifi::activate()
{
  KProcess proc;

  QStringList failedParts;

  WifiConfig *config = WifiConfig::instance();
  int index;
  if ( config->m_presetConfig >= config->m_numConfigs ) 
    {
      index = vendorBase+config->m_presetConfig-config->m_numConfigs; 
    } else
    {
      index = config->m_presetConfig;
    }

  kdDebug() << "I was told to use array slot number " << index << ".\n";

  IfConfig ifconfig = config->m_ifConfig[ index ];

  QString tempInterface;

  if ( ifconfig.m_interface.isEmpty() ) {
      tempInterface = config->autoDetectInterface();
      kdDebug() << "Autodetecting interface...\n";
    } else
    {
      tempInterface = ifconfig.m_interface;
    }

  proc << "ifconfig"
       << tempInterface
       << "down";
  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (!proc.normalExit()) || (proc.exitStatus() != 0) ) 
	failedParts << i18n("Interface could not be shut down.  It is likely that your settings have not been applied.");

  proc.clearArguments();

  proc << "iwconfig"
       << tempInterface;
  proc << "essid"
       << ifconfig.m_networkName;

  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (!proc.normalExit()) || (proc.exitStatus() != 0) )
        failedParts << i18n("SSID could not be set.");

  proc.clearArguments();

  proc << "iwconfig"
       << tempInterface;
  proc << "mode"
       << ifconfig.wifimodeAsString();

  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (!proc.normalExit()) || (proc.exitStatus() != 0) )
        failedParts << i18n("Operation mode could not be set.");

  proc.clearArguments();

  proc << "iwconfig"
       << tempInterface;
  proc << "rate"
       << ifconfig.speedAsString();

  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (!proc.normalExit()) || (proc.exitStatus() != 0) )
        failedParts << i18n("Speed settings could not be modified.");


  proc.clearArguments();

  proc << "iwconfig"
       << tempInterface;

  if (!ifconfig.m_useCrypto )
  {
    proc << "key"
         << "off";
  }
  else
  {
    for ( int i=0; i<4; ++i )
    {
      if ( ifconfig.m_keys[ i ].isValid( ifconfig.m_keys[ i ].key() ) > INVALID )
      {
        proc << "key"
             << QString( "[%1]").arg( i+1 )
	     << QString( "%1").arg( ifconfig.m_keys[ i ].rawKey() );
      }
    }

    // kdDebug() << "All appropriate keys have been fed into iwconfig.\n";

    if ( ifconfig.activeKey().isValid( ifconfig.activeKey().key() ) > INVALID )
    {
      // kdDebug() << "Entering final phase of key setup.\n";
      proc << "key"
           << ifconfig.cryptomodeAsString();
      proc << "key"
           << QString( "[%1]" ).arg( ifconfig.activeKeyId() );
      proc << "key"
           << "on";
    }
  }
  
  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (ifconfig.m_useCrypto) && ((!proc.normalExit()) || (proc.exitStatus() != 0)) )
        failedParts << i18n("Encryption settings could not be set.");

  proc.clearArguments();

  proc << "iwconfig"
       << tempInterface;

  if ( !ifconfig.m_pmEnabled )
  {
    proc << "power"
         << "off";
  }
  else
  {
    proc << "power"
	 << "period"
         << QString( "%1" ).arg( ifconfig.m_wakeupPeriod );
    proc << "power"
	 << "timeout"
         << QString( "%1" ).arg( ifconfig.m_sleepTimeout );
    proc << ifconfig.powermodeAsString();
  }

  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (ifconfig.m_pmEnabled) && ((!proc.normalExit()) || (proc.exitStatus() != 0)) )
        failedParts << i18n("Power management settings could not be set.");

  proc.clearArguments();
  proc << "ifconfig"
       << tempInterface
       << "up";
  
  kdDebug() << "Command: " << proc.args() << endl;
  proc.start (KProcess::Block);

  if ( (!proc.normalExit()) || (proc.exitStatus() != 0) )
        failedParts << i18n("Interface could not be re-enabled.");

  if ( ifconfig.m_runScript )
  {
    proc.clearArguments();
    proc << QStringList::split( " ", ifconfig.m_connectScript );

    proc.start(KProcess::DontCare);
  }

if (!failedParts.empty()) KMessageBox::informationList(0,i18n("The following settings could not be applied:"),failedParts);

}

extern "C"
{
  void init_wifi()
  {
    KCMWifi::activate();
  }
}


#include "kcmwifi.moc"
