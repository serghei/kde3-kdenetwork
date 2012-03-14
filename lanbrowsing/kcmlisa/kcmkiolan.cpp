/*
 * kcmkiolan.cpp
 *
 * Copyright (c) 2000 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kcmkiolan.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

IOSlaveSettings::IOSlaveSettings(const QString& config, QWidget *parent)
:KCModule(parent)
,m_config(config,false,true)
{
   QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
   layout->setAutoAdd(true);

   QGroupBox* group=new QGroupBox(1, Horizontal, i18n("Show Links for Following Services"), this);

   m_ftpSettings=new PortSettingsBar(i18n("FTP (TCP, port 21): "), group);
   m_httpSettings=new PortSettingsBar(i18n("HTTP (TCP, port 80): "),group);
   m_nfsSettings=new PortSettingsBar(i18n("NFS (TCP, port 2049): "),group);
   m_smbSettings=new PortSettingsBar(i18n("Windows shares (TCP, ports 445 and 139):"),group);
   m_fishSettings=new PortSettingsBar(i18n("Secure Shell/Fish (TCP, port 22): "),group);
   m_shortHostnames = new QCheckBox(i18n("Show &short hostnames (without domain suffix)"),this);

   QHBox *hbox=new QHBox(this);
   QLabel *label=new QLabel(i18n("Default LISa server host: "), hbox);
   m_defaultLisaHostLe=new QLineEdit(hbox);
   label->setBuddy(m_defaultLisaHostLe);

   QWidget *w=new QWidget(this);
   layout->setStretchFactor(m_ftpSettings,0);
   layout->setStretchFactor(m_httpSettings,0);
   layout->setStretchFactor(m_nfsSettings,0);
   layout->setStretchFactor(m_smbSettings,0);
   layout->setStretchFactor(m_shortHostnames,0);
   layout->setStretchFactor(hbox,0);
   layout->setStretchFactor(w,1);

   connect(m_ftpSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_httpSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_nfsSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_smbSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_fishSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_shortHostnames,SIGNAL(clicked()),this,SIGNAL(changed()));
   connect(m_defaultLisaHostLe, SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));
}

void IOSlaveSettings::load()
{
   kdDebug()<<"IOSlaveSettings::load()"<<endl;
   m_ftpSettings->setChecked(m_config.readNumEntry("Support_FTP", PORTSETTINGS_CHECK));
   m_httpSettings->setChecked(m_config.readNumEntry("Support_HTTP", PORTSETTINGS_CHECK));
   m_nfsSettings->setChecked(m_config.readNumEntry("Support_NFS", PORTSETTINGS_CHECK));
   m_smbSettings->setChecked(m_config.readNumEntry("Support_SMB", PORTSETTINGS_CHECK));
   m_fishSettings->setChecked(m_config.readNumEntry("Support_FISH", PORTSETTINGS_CHECK));
   m_shortHostnames->setChecked(m_config.readBoolEntry("ShowShortHostnames",false));
//   m_rlanSidebar->setChecked(m_config.readEntry("sidebarURL", "lan:/") == "rlan:/" ? true : false );
   m_defaultLisaHostLe->setText(m_config.readEntry("DefaultLisaHost", "localhost"));
}

void IOSlaveSettings::save()
{
   kdDebug()<<"IOSlaveSettings::save()"<<endl;
   m_config.writeEntry("AlreadyConfigured",true);
   m_config.writeEntry("Support_FTP", m_ftpSettings->selected());
   m_config.writeEntry("Support_HTTP", m_httpSettings->selected());
   m_config.writeEntry("Support_NFS", m_nfsSettings->selected());
   m_config.writeEntry("Support_SMB", m_smbSettings->selected());
   m_config.writeEntry("Support_FISH", m_fishSettings->selected());
   m_config.writeEntry("ShowShortHostnames",m_shortHostnames->isChecked());
//   m_config.writeEntry("sidebarURL", m_rlanSidebar->isChecked() ? "rlan:/" : "lan:/");
   m_config.writeEntry("DefaultLisaHost", m_defaultLisaHostLe->text());

   m_config.sync();
}

#include "kcmkiolan.moc"

