/*
 * kcmreslisa.cpp
 *
 * Copyright (c) 2000-2002 Alexander Neundorf <neundorf@kde.org>
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
#include "kcmreslisa.h"
#include "findnic.h"

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qvbuttongroup.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krestrictedline.h>
#include <keditlistbox.h>

ResLisaSettings::ResLisaSettings(const QString& config, QWidget *parent)
: KCModule(parent)
,m_config(config,false,true)
,m_kiolanConfig("kio_lanrc",false,true)
,m_advancedSettingsButton(0)
,m_suggestSettings(0)
,m_useNmblookup(0)
,m_pingNames(0)
,m_allowedAddresses(0)
,m_firstWait(0)
,m_secondScan(0)
,m_secondWait(0)
,m_updatePeriod(0)
,m_deliverUnnamedHosts(0)
,m_maxPingsAtOnce(0)
,m_reslisaAdvancedDlg(0)
{
   QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
   layout->setAutoAdd(true);
   
   QVButtonGroup *gb=new QVButtonGroup(i18n("Tell ResLISa Daemon How to Search for Hosts"),this);
   gb->setInsideSpacing(10);

   m_useNmblookup=new QCheckBox(i18n("Send &NetBIOS broadcasts using &nmblookup"),gb);
   QToolTip::add(m_useNmblookup,i18n("Only hosts running SMB servers will answer"));

   m_pingNames=new KEditListBox(i18n("A&dditionally Check These Hosts"),gb,"a",false, KEditListBox::Add|KEditListBox::Remove);
   QToolTip::add(m_pingNames,i18n("The hosts listed here will be pinged."));

   QHBox *hbox=new QHBox(this);
   hbox->setSpacing(10);
   QLabel *label=new QLabel(i18n("&Trusted addresses:"),hbox);
   QString comment = i18n("Usually your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)");
   QToolTip::add(label, comment);
   m_allowedAddresses=new KRestrictedLine(hbox,"a","0123456789./;");
   QToolTip::add(m_allowedAddresses,comment);
   label->setBuddy(m_allowedAddresses);

    m_rlanSidebar = new QCheckBox(i18n("Use &rlan:/ instead of lan:/ in Konqueror's navigation panel"), this);

   hbox = new QHBox(this);
   m_suggestSettings=new QPushButton(i18n("&Suggest Settings"),hbox);

   new QWidget(hbox);

   m_advancedSettingsButton=new QPushButton(i18n("Ad&vanced Settings"),hbox);


   m_reslisaAdvancedDlg=new KDialogBase(0,0,true,i18n("Advanced Settings for ResLISa"),KDialogBase::Close, KDialogBase::Close);
   connect(m_advancedSettingsButton,SIGNAL(clicked()),m_reslisaAdvancedDlg,SLOT(show()));

   QVBox *vbox=m_reslisaAdvancedDlg->makeVBoxMainWidget();

   m_deliverUnnamedHosts=new QCheckBox(i18n("Show &hosts without DNS names"),vbox);

   QGrid *advGrid = new QGrid(2, Qt::Horizontal, vbox);
   advGrid->setSpacing(10);

   label=new QLabel(i18n("Host list update interval:"),advGrid);
   QToolTip::add(label,i18n("Search hosts after this number of seconds"));
   m_updatePeriod=new QSpinBox(30,1800,10,advGrid);
   m_updatePeriod->setSuffix(i18n(" sec"));
   QToolTip::add(m_updatePeriod,i18n("Search hosts after this number of seconds"));

   m_secondScan=new QCheckBox(i18n("Always check twice for hosts when searching"),advGrid);
   new QWidget(advGrid);

   label=new QLabel(i18n("Wait for replies from hosts after first scan:"),advGrid);
   QToolTip::add(label,i18n("How long to wait for replies to the ICMP echo requests from hosts"));
   m_firstWait=new QSpinBox(10,1000,50,advGrid);
   m_firstWait->setSuffix(i18n(" ms"));
   QToolTip::add(m_firstWait,i18n("How long to wait for replies to the ICMP echo requests from hosts"));

   label=new QLabel(i18n("Wait for replies from hosts after second scan:"),advGrid);
   QToolTip::add(label,i18n("How long to wait for replies to the ICMP echo requests from hosts"));
   m_secondWait=new QSpinBox(0,1000,50,advGrid);
   m_secondWait->setSuffix(i18n(" ms"));
   QToolTip::add(m_secondWait,i18n("How long to wait for replies to the ICMP echo requests from hosts"));

   label=new QLabel(i18n("Max. number of ping packets to send at once:"),advGrid);
   m_maxPingsAtOnce=new QSpinBox(8,1024,5,advGrid);

   QWidget *dummy=new QWidget(advGrid);
   dummy->setMinimumHeight(10);

   connect(m_secondScan,SIGNAL(toggled(bool)),m_secondWait,SLOT(setEnabled(bool)));

   connect(m_allowedAddresses,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));

   connect(m_allowedAddresses,SIGNAL(returnPressed()),this,SIGNAL(changed()));

   connect(m_firstWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_maxPingsAtOnce,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondScan,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_deliverUnnamedHosts,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_updatePeriod,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_pingNames,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_useNmblookup,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_suggestSettings,SIGNAL(clicked()),this,SLOT(suggestSettings()));
   connect(m_rlanSidebar,SIGNAL(clicked()),this,SIGNAL(changed()));

   load();
}

void ResLisaSettings::load()
{
   int secondWait=m_config.readNumEntry("SecondWait",-1);
   if (secondWait<0)
   {
      m_secondWait->setValue(300);
      m_secondScan->setChecked(FALSE);
      m_secondWait->setEnabled(FALSE);
   }
   else
   {
      m_secondWait->setValue(secondWait*10);
      m_secondScan->setChecked(TRUE);
      m_secondWait->setEnabled(TRUE);
   };
   m_deliverUnnamedHosts->setChecked(m_config.readNumEntry("DeliverUnnamedHosts",0));

   m_firstWait->setValue(m_config.readNumEntry("FirstWait",30)*10);
   m_maxPingsAtOnce->setValue(m_config.readNumEntry("MaxPingsAtOnce",256));
   m_updatePeriod->setValue(m_config.readNumEntry("UpdatePeriod",300));
   m_allowedAddresses->setText(m_config.readEntry("AllowedAddresses","192.168.0.0/255.255.255.0"));

   int i=m_config.readNumEntry("SearchUsingNmblookup",1);
   m_useNmblookup->setChecked(i!=0);
   m_pingNames->clear();
   m_pingNames->insertStringList(m_config.readListEntry("PingNames",';'));

   m_rlanSidebar->setChecked(m_kiolanConfig.readEntry("sidebarURL", "lan:/") == "rlan:/" ? true : false );
}

void ResLisaSettings::save()
{
   if (m_secondScan->isChecked())
      m_config.writeEntry("SecondWait",(m_secondWait->value()+5)/10);
   else
      m_config.writeEntry("SecondWait",-1);

   if (m_useNmblookup->isChecked())
      m_config.writeEntry("SearchUsingNmblookup",1);
   else
      m_config.writeEntry("SearchUsingNmblookup",0);

   if (m_deliverUnnamedHosts->isChecked())
      m_config.writeEntry("DeliverUnnamedHosts",1);
   else
      m_config.writeEntry("DeliverUnnamedHosts",0);

   m_config.writeEntry("FirstWait",(m_firstWait->value()+5)/10);
   m_config.writeEntry("MaxPingsAtOnce",m_maxPingsAtOnce->value());
   m_config.writeEntry("UpdatePeriod",m_updatePeriod->value());
   m_config.writeEntry("AllowedAddresses",m_allowedAddresses->text());
   //m_config.writeEntry("BroadcastNetwork",m_broadcastNetwork->text());
   QStringList writeStuff;
   for (int i=0; i<m_pingNames->count(); i++)
      writeStuff.append(m_pingNames->text(i));
   m_config.writeEntry("PingNames",writeStuff,';');

   m_config.sync();

   m_kiolanConfig.writeEntry("sidebarURL", m_rlanSidebar->isChecked() ? "rlan:/" : "lan:/");
   m_kiolanConfig.sync();

}

void ResLisaSettings::suggestSettings()
{
   NICList* nics=findNICs();
   if (nics->count()==0)
   {
      //ok, easy one :-)
      KMessageBox::sorry(0,i18n("It appears you do not have any network interfaces installed on your system."));
      delete nics;
      return;
   }

   MyNIC *nic=nics->first();
   QString address = nic->addr;
   QString netmask = nic->netmask;
   m_allowedAddresses->setText(address+"/"+netmask+";");
   m_secondWait->setValue(0);
   m_secondScan->setChecked(FALSE);
   m_secondWait->setEnabled(FALSE);
   m_firstWait->setValue(300);
   m_maxPingsAtOnce->setValue(256);
   m_updatePeriod->setValue(300);
   m_useNmblookup->setChecked(true);
   if (nics->count()>1)
   {
      QString msg(i18n("You have more than one network interface installed.<br>"
                       "Please make sure the suggested settings are correct.<br>"
                       "<br>The following interfaces were found:<br><br>"));
      //not that easy to handle
      for (MyNIC* tmp=nics->first(); tmp!=0; tmp=nics->next())
      {
         msg+="<b>"+tmp->name+": </b>"+tmp->addr+"/"+tmp->netmask+";<br>";
      }
      KMessageBox::information(0,QString("<html>%1</html>").arg(msg));
   }
   KMessageBox::information(0,QString("<html>%1</html>").arg(i18n("The ResLISa daemon is now configured "
                                                                  "correctly, hopefully.<br>Make sure that the reslisa binary is installed <i>suid root</i>.")));

   emit changed();
   delete nics;
}

#include "kcmreslisa.moc"

