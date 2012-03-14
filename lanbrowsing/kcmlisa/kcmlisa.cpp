/*
 * kcmlisa.cpp
 *
 * Copyright (c) 2000,2001 Alexander Neundorf <neundorf@kde.org>
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

#include "kcmlisa.h"

#include "findnic.h"
#include "setupwizard.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>

#include <qtooltip.h>
#include <qfile.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgrid.h>
#include <qvbuttongroup.h>

#include <kapplication.h>
#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <keditlistbox.h>
#include <krestrictedline.h>
#include <kdialogbase.h>
#include <ktempfile.h>

#include <kdebug.h>

LisaSettings::LisaSettings(const QString& config, QWidget *parent)
: KCModule(parent, "kcmlanbrowser")
,m_config(config,false,true)
,m_wizard(0)
,m_configFilename(config)
,m_changed(false)
{
   QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
   layout->setAutoAdd(true);

   QWidget *dummy(0);

   QVButtonGroup *gb=new QVButtonGroup(i18n("Tell LISa Daemon How to Search for Hosts"),this);
   gb->setInsideSpacing(10);

   m_useNmblookup=new QCheckBox(i18n("Send &NetBIOS broadcasts using nmblookup"), gb);
   QToolTip::add(m_useNmblookup,i18n("Only hosts running SMB servers will answer"));

   m_sendPings=new QCheckBox(i18n("Send &pings (ICMP echo packets)"), gb);
   QToolTip::add(m_sendPings,i18n("All hosts running TCP/IP will answer"));

   QHBox *hbox=new QHBox(gb);
   hbox->setSpacing(10);

   dummy=new QWidget(hbox);
   dummy->setMinimumWidth(10);
   QLabel *label=new QLabel(i18n("To these &IP addresses:"),hbox);
   QString comment = i18n("Enter all ranges to scan, using the format '192.168.0.1/255.255.255.0;10.0.0.1;255.0.0.0'");
   QToolTip::add(label,comment);
   m_pingAddresses=new KRestrictedLine(hbox,"a","0123456789.-/;");
   QToolTip::add(m_pingAddresses, comment);
   label->setBuddy(m_pingAddresses);

   QGrid *addressesGrid = new QGrid(2, Qt::Horizontal, this);
   addressesGrid->setSpacing(10);
   layout->setStretchFactor(addressesGrid,0);

   label=new QLabel(i18n("&Broadcast network address:"),addressesGrid);
   comment=i18n("Your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)");
   QToolTip::add(label, comment);

   m_broadcastNetwork=new KRestrictedLine(addressesGrid,"a","0123456789./;");
   QToolTip::add(m_broadcastNetwork,comment);
   label->setBuddy(m_broadcastNetwork);

   label=new QLabel(i18n("&Trusted IP addresses:"),addressesGrid);
   comment = i18n("Usually your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)");
   QToolTip::add(label, comment);

   m_allowedAddresses=new KRestrictedLine(addressesGrid,"a","0123456789./;");
   QToolTip::add(m_allowedAddresses, comment);
   label->setBuddy(m_allowedAddresses);

   dummy=new QWidget(this);
   layout->setStretchFactor(dummy,10);

   hbox = new QHBox(this);
   hbox->setSpacing(10);
//   m_autoSetup=new QPushButton(i18n("&Guided LISa Setup..."),hbox);
   m_autoSetup=new QPushButton(i18n("Setup Wizard..."),hbox);
   m_autoSetup->setFixedWidth( m_autoSetup->sizeHint().width() );

   m_suggestSettings=new QPushButton(i18n("&Suggest Settings"),hbox);

   new QWidget(hbox);

   m_advancedSettingsButton=new QPushButton(i18n("Ad&vanced Settings..."),hbox);

   m_lisaAdvancedDlg=new KDialogBase(0,0,true,i18n("Advanced Settings for LISa"),KDialogBase::Close, KDialogBase::Close);
   connect(m_advancedSettingsButton,SIGNAL(clicked()),m_lisaAdvancedDlg,SLOT(show()));

   QVBox *vbox=m_lisaAdvancedDlg->makeVBoxMainWidget();

   m_pingNames=new KEditListBox(i18n("&Additionally Check These Hosts"),vbox,"a",false, KEditListBox::Add|KEditListBox::Remove);
   m_pingNames->setMinimumHeight(180);
   QToolTip::add(m_pingNames,i18n("The hosts listed here will be pinged"));

   dummy=new QWidget(vbox);
   dummy->setMinimumHeight(10);
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

   dummy=new QWidget(advGrid);
   dummy->setMinimumHeight(10);

   connect(m_secondScan,SIGNAL(toggled(bool)),m_secondWait,SLOT(setEnabled(bool)));
   connect(m_sendPings,SIGNAL(toggled(bool)),m_pingAddresses,SLOT(setEnabled(bool)));

   connect(m_pingAddresses,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));
   connect(m_allowedAddresses,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));
   connect(m_broadcastNetwork,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));

   connect(m_pingAddresses,SIGNAL(returnPressed()),this,SIGNAL(changed()));
   connect(m_allowedAddresses,SIGNAL(returnPressed()),this,SIGNAL(changed()));
   connect(m_broadcastNetwork,SIGNAL(returnPressed()),this,SIGNAL(changed()));

   connect(m_sendPings,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_firstWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_maxPingsAtOnce,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondScan,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_deliverUnnamedHosts,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_updatePeriod,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_pingNames,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_useNmblookup,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_autoSetup,SIGNAL(clicked()),this,SLOT(autoSetup()));
   connect(m_suggestSettings,SIGNAL(clicked()),this,SLOT(suggestSettings()));
   connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

void LisaSettings::load()
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
   }
   m_deliverUnnamedHosts->setChecked(m_config.readNumEntry("DeliverUnnamedHosts",0));

   m_firstWait->setValue(m_config.readNumEntry("FirstWait",30)*10);
   m_maxPingsAtOnce->setValue(m_config.readNumEntry("MaxPingsAtOnce",256));
   m_updatePeriod->setValue(m_config.readNumEntry("UpdatePeriod",300));
   m_pingAddresses->setText(m_config.readEntry("PingAddresses","192.168.0.0/255.255.255.0;192.168.100.0-192.168.100.254"));
   m_sendPings->setChecked(!m_pingAddresses->text().isEmpty());
   m_allowedAddresses->setText(m_config.readEntry("AllowedAddresses","192.168.0.0/255.255.255.0"));
   m_broadcastNetwork->setText(m_config.readEntry("BroadcastNetwork","192.168.0.0/255.255.255.0"));
   m_pingNames->clear();
   m_pingNames->insertStringList(m_config.readListEntry("PingNames",';'));
   int i=m_config.readNumEntry("SearchUsingNmblookup",1);
   m_useNmblookup->setChecked(i!=0);
   m_changed = false;
}

void LisaSettings::save()
{
   if (!m_changed) return;

   if ( getuid()==0)
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
      m_config.writeEntry("PingAddresses",m_sendPings->isChecked()?m_pingAddresses->text():"");
      m_config.writeEntry("AllowedAddresses",m_allowedAddresses->text());
      m_config.writeEntry("BroadcastNetwork",m_broadcastNetwork->text());
      QStringList writeStuff;
      for (int i=0; i<m_pingNames->count(); i++)
         writeStuff.append(m_pingNames->text(i));
      m_config.writeEntry("PingNames",writeStuff,';');

      m_config.sync();
      chmod(QFile::encodeName(m_configFilename),S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
   }
   else
   {
      //ok, now it gets harder
      //we are not root but we want to write into /etc ....
      //any idea how to do it better ?
      KTempFile tmp;

      if (tmp.status() == 0 && tmp.textStream())
      {
         m_tmpFilename = tmp.name();
         QTextStream &confStream = *(tmp.textStream());
         if (m_secondScan->isChecked())
            confStream<<"SecondWait = "<<(m_secondWait->value()+5)/10<<"\n";
         else
            confStream<<"SecondWait = -1\n";

         if (m_useNmblookup->isChecked())
            confStream<<"SearchUsingNmblookup = 1\n";
         else
            confStream<<"SearchUsingNmblookup = 0\n";

         if (m_deliverUnnamedHosts->isChecked())
            confStream<<"DeliverUnnamedHosts = 1\n";
         else
            confStream<<"DeliverUnnamedHosts = 0\n";

         confStream<<"FirstWait = "<< (m_firstWait->value()+5)/10 <<"\n";
         confStream<<"MaxPingsAtOnce = "<<m_maxPingsAtOnce->value()<<"\n";
         confStream<<"UpdatePeriod = "<<m_updatePeriod->value()<<"\n";
         confStream<<"PingAddresses = "<<m_pingAddresses->text().latin1()<<"\n";
         confStream<<"AllowedAddresses = "<<m_allowedAddresses->text().latin1()<<"\n";
         confStream<<"BroadcastNetwork = "<<m_broadcastNetwork->text().latin1()<<"\n";
         QString writeStuff;
         for (int i=0; i<m_pingNames->count(); i++)
            writeStuff=writeStuff+m_pingNames->text(i).latin1()+";";

         confStream<<"PingNames = "<<writeStuff.latin1()<<"\n";
         tmp.close();
         QString suCommand=QString("cp '%1' '%2'; chmod 644 '%3'; rm -f '%4'").arg(m_tmpFilename).arg(m_configFilename).arg(m_configFilename).arg(m_tmpFilename);
         KProcess *proc = new KProcess();
         connect(proc, SIGNAL(processExited(KProcess *)), this, SLOT(saveDone(KProcess *)));
         *proc<<"kdesu"<<"-c"<<suCommand;
         KApplication::setOverrideCursor(Qt::waitCursor);
         setEnabled(false);
         if ( !proc->start() )
             delete proc;
      }
      else
         KMessageBox::sorry(0,i18n("Saving the results to %1 failed.").arg(m_configFilename));
   }
}

void LisaSettings::suggestSettings()
{
   NICList *nics=findNICs();
   if (nics->count()==0)
   {
      KMessageBox::sorry(0,i18n("No network interface cards found."));
      delete nics;
      return;
   }
   MyNIC *nic=nics->first();
   LisaConfigInfo lci;
   suggestSettingsForNic(nic,lci);
   m_pingAddresses->setText(lci.pingAddresses);
   m_sendPings->setChecked(!m_pingAddresses->text().isEmpty());
   m_broadcastNetwork->setText(lci.broadcastNetwork);
   m_allowedAddresses->setText(lci.allowedAddresses);
   m_secondWait->setValue(lci.secondWait*10);
   m_secondScan->setChecked(lci.secondScan);
   m_secondWait->setEnabled(lci.secondScan);
   m_firstWait->setValue(lci.firstWait*10);
   m_maxPingsAtOnce->setValue(lci.maxPingsAtOnce);
   m_updatePeriod->setValue(lci.updatePeriod);
   m_useNmblookup->setChecked(lci.useNmblookup);
   m_deliverUnnamedHosts->setChecked(lci.unnamedHosts);

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

   emit changed();
   delete nics;
}

void LisaSettings::autoSetup()
{
   LisaConfigInfo lci;
   if (m_wizard==0)
      m_wizard=new SetupWizard(this,&lci);
   else
      m_wizard->clearAll();
   int result=m_wizard->exec();

   if (result!=QDialog::Accepted)
      return;

   m_pingAddresses->setText(lci.pingAddresses);
   m_sendPings->setChecked(!m_pingAddresses->text().isEmpty());
   m_broadcastNetwork->setText(lci.broadcastNetwork);
   m_allowedAddresses->setText(lci.allowedAddresses);
   m_secondWait->setValue(lci.secondWait*10);
   m_secondScan->setChecked(lci.secondScan);
   m_secondWait->setEnabled(lci.secondScan);
   m_firstWait->setValue(lci.firstWait*10);
   m_maxPingsAtOnce->setValue(lci.maxPingsAtOnce);
   m_updatePeriod->setValue(lci.updatePeriod);
   m_useNmblookup->setChecked(lci.useNmblookup);
   m_deliverUnnamedHosts->setChecked(lci.unnamedHosts);

   emit changed();
   return;
}

void LisaSettings::saveDone(KProcess *proc)
{
   unlink(QFile::encodeName(m_tmpFilename));
   KApplication::restoreOverrideCursor();
   setEnabled(true);
   KMessageBox::information(0,i18n("The configuration has been saved to /etc/lisarc.\n"
                                   "Make sure that the LISa daemon is started,\n e.g. using an init script when booting.\n"
                                   "You can find examples and documentation at http://lisa-home.sourceforge.net ."));

   delete(proc);
}

void LisaSettings::slotChanged()
{
   m_changed = true;
}

#include "kcmlisa.moc"

