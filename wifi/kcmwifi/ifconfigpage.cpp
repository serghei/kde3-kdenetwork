/* This file is part of the KDE project
   Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include "configcrypto.h"
#include "configpower.h"
#include "ifconfigpage.h"
#include "wificonfig.h"

#include <unistd.h>
#include <sys/types.h>

IfConfigPage::IfConfigPage( int configNum, QWidget *parent, const char *name )
    : IfConfigPageBase( parent, name ),
      m_configNum( configNum )
{
  connect( cb_pmEnabled, SIGNAL( toggled( bool ) ), SIGNAL( changed() ) );
  connect( cb_useCrypto, SIGNAL( toggled( bool ) ), SIGNAL( changed() ) );
  connect( le_networkName, SIGNAL( textChanged( const QString & ) ),
                           SIGNAL( changed() ) );
  connect( cmb_wifiMode, SIGNAL( activated( int ) ), SIGNAL( changed() ) );
  connect( cmb_speed, SIGNAL( activated( int ) ), SIGNAL( changed() ) );
  connect( cb_runScript, SIGNAL( toggled( bool ) ), SIGNAL( changed() ) );
  connect( url_connectScript, SIGNAL( textChanged( const QString & ) ),
	   		      SIGNAL( changed() ) );
  connect( cb_Autodetect, SIGNAL( toggled( bool ) ), SIGNAL( changed() ) );
  connect( le_interface, SIGNAL( textChanged( const QString & ) ),
		         SIGNAL( changed() ) );

  connect( pb_setupPower, SIGNAL( clicked() ), SLOT( slotSetupPower() ) );
  connect( pb_setupCrypto, SIGNAL( clicked() ), SLOT( slotSetupCrypto() ) );

  connect( cb_useCrypto, SIGNAL( toggled( bool ) ),
           pb_setupCrypto, SLOT( setEnabled( bool ) ) );
  connect( cb_pmEnabled, SIGNAL( toggled( bool ) ),
           pb_setupPower, SLOT( setEnabled( bool ) ) );
  connect( cb_runScript, SIGNAL( toggled( bool ) ),
           lb_connectScript, SLOT( setEnabled( bool ) ) );
  connect( cb_runScript, SIGNAL( toggled( bool ) ),
           lb_connectScript, SLOT( setEnabled( bool ) ) );
  connect( cb_runScript, SIGNAL( toggled( bool ) ),
           lb_connectScript, SLOT( setEnabled( bool ) ) );
  connect( cb_Autodetect, SIGNAL( toggled( bool ) ),
           le_interface, SLOT( setDisabled( bool ) ) );

  connect( cb_Autodetect, SIGNAL( toggled( bool ) ),
           this, SLOT ( slotResetInterface( bool ) ) );
}

void IfConfigPage::slotResetInterface( bool checked )
{
  if ( checked )
  {
    WifiConfig *config = WifiConfig::instance();
    le_interface->setText( config->autoDetectInterface() );
  }
}

void IfConfigPage::load()
{
  WifiConfig *config = WifiConfig::instance();
  IfConfig ifConfig = config->m_ifConfig[ m_configNum ];

  le_networkName->setText( ifConfig.m_networkName );
  if ( ! ifConfig.m_interface.isEmpty() )
  {
    le_interface->setText( ifConfig.m_interface );
    cb_Autodetect->setChecked( false );
  }
  else
  {
    if ( geteuid()==0 )
      le_interface->setText( config->autoDetectInterface() );
    cb_Autodetect->setChecked( true );
  }

  cmb_wifiMode->setCurrentItem( ifConfig.m_wifiMode );
  cmb_speed->setCurrentItem( ifConfig.m_speed );
  cb_runScript->setChecked( ifConfig.m_runScript );
  url_connectScript->setURL( ifConfig.m_connectScript );
  cb_useCrypto->setChecked( ifConfig.m_useCrypto );
  cb_pmEnabled->setChecked( ifConfig.m_pmEnabled );

  pb_setupCrypto->setEnabled( ifConfig.m_useCrypto );
  pb_setupPower->setEnabled( ifConfig.m_pmEnabled );
  lb_connectScript->setEnabled( ifConfig.m_runScript );
  url_connectScript->setEnabled( ifConfig.m_runScript );
}

void IfConfigPage::save()
{
  WifiConfig *config = WifiConfig::instance();
  IfConfig &ifconfig = config->m_ifConfig[ m_configNum ];

  ifconfig.m_networkName = le_networkName->text();
  ifconfig.m_interface = cb_Autodetect->isChecked()? QString::null :
                                                     le_interface->text();
  ifconfig.m_wifiMode = ( IfConfig::WifiMode ) cmb_wifiMode->currentItem();
  ifconfig.m_speed = ( IfConfig::Speed ) cmb_speed->currentItem();
  ifconfig.m_runScript = cb_runScript->isChecked();
  ifconfig.m_connectScript = url_connectScript->url();
  ifconfig.m_useCrypto = cb_useCrypto->isChecked();
  ifconfig.m_pmEnabled = cb_pmEnabled->isChecked();
}

void IfConfigPage::slotSetupPower()
{
  KDialogBase *dlg = new KDialogBase( this, "ConfigPower", true,
      i18n( "Configure Power Mode" ), KDialogBase::Ok|KDialogBase::Cancel,
      KDialogBase::Ok );
  ConfigPower *power = new ConfigPower( dlg, "ConfigPower" );

  WifiConfig *config = WifiConfig::instance();
  IfConfig &ifconfig = config->m_ifConfig[ m_configNum ];
  power->load( ifconfig );

  dlg->setMainWidget( power );

  if ( dlg->exec() == KDialog::Accepted )
  {
    power->save( ifconfig );
    emit changed();
  }
}

void IfConfigPage::slotSetupCrypto()
{
  KDialogBase *dlg = new KDialogBase( this, "ConfigCrypto", true,
      i18n( "Configure Encryption" ), KDialogBase::Ok|KDialogBase::Cancel,
      KDialogBase::Ok );
  ConfigCrypto *crypto = new ConfigCrypto( dlg, "ConfigCrypto" );

  WifiConfig *config = WifiConfig::instance();
  IfConfig &ifconfig = config->m_ifConfig[ m_configNum ];
  crypto->load( ifconfig );

  dlg->setMainWidget( crypto );

  if ( dlg->exec() == KDialog::Accepted )
  {
    crypto->save( ifconfig );
    emit changed();
  }
}

#include "ifconfigpage.moc"
