/* This file is part of the KDE project
   Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>
             (C) 2001-2004 by Stefan Winter <mail@stefan-winter.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING. If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "mainconfig.h"
#include "wificonfig.h"
#include "kcmwifi.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocio.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include <qcheckbox.h>
#include <qfile.h>

MainConfig::MainConfig( QWidget *parent, const char *name )
    : MainConfigBase( parent, name )
{
  WifiConfig *config = WifiConfig::instance();

  for (int i=1;i<=config->m_numConfigs;i++) {
    cmb_presetConfig->insertItem( i18n( "Config %1" ).arg( i ) );  
  }


  connect( cb_usePreset, SIGNAL( toggled( bool ) ), SIGNAL( changed() ) );
  connect( cmb_presetConfig, SIGNAL( activated( int ) ), SIGNAL( changed() ) );
  connect( sb_numConfigs, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );

  connect( pb_activate, SIGNAL( clicked() ), SIGNAL( activateClicked() ) );
  connect( sb_numConfigs, SIGNAL( valueChanged( int ) ), SLOT( slotChangeNumConfigs( int ) ) );

}

void MainConfig::load()
{
  WifiConfig *config = WifiConfig::instance();

  cb_usePreset->setChecked( config->m_usePreset );
  cmb_presetConfig->setCurrentItem( config->m_presetConfig );
  sb_numConfigs->setValue( config->m_numConfigs );
}

void MainConfig::save()
{
  WifiConfig *config = WifiConfig::instance();

  config->m_usePreset = cb_usePreset->isChecked();
  config->m_presetConfig = cmb_presetConfig->currentItem();
  config->m_numConfigs = sb_numConfigs->value();
}

void MainConfig::slotChangeNumConfigs( int newnumber )
{

  WifiConfig* config = WifiConfig::instance();

  if ( config->m_numConfigs < newnumber ) // number of configs raised
  {
    int diff = newnumber - config->m_numConfigs;
    ( (KCMWifi*)parentWidget() )->addConfigTab( diff, false );
  } 
  else // number of configs lowered
  {
    int diff = config->m_numConfigs - newnumber;
    ( (KCMWifi*)parentWidget() )->delConfigTab( diff );
  } 
}

void MainConfig::registerConfig( int number )
{
  if ( number >= KCMWifi::vendorBase ) 
  {
    cmb_presetConfig->insertItem( i18n( "Vendor %1" ).arg( number-KCMWifi::vendorBase+1 ) );
  }
  else
  {
    WifiConfig* config = WifiConfig::instance();
    cmb_presetConfig->insertItem( i18n( "Config %1" ).arg( number ), number-1 );
  }
}

void MainConfig::unregisterConfig( int number )
{
  cmb_presetConfig->removeItem( number-1 );
}

#include "mainconfig.moc"
