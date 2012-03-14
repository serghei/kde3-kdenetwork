/*
 * setupwizard.cpp
 *
 *  Copyright (C) 2001 Alexander Neundorf <neundorf@kde.org>
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

#include "setupwizard.h"

#include <qregexp.h>

#include <klocale.h>
#include <kdialog.h>
#include <ksockaddr.h>
#include <kdebug.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

SetupWizard::SetupWizard(QWidget* parent, LisaConfigInfo* configInfo)
   :QWizard(parent,"hallo",true)
   ,m_page1(0)
   ,m_noNicPage(0)
   ,m_multiNicPage(0)
   ,m_searchPage(0)
   ,m_addressesPage(0)
   ,m_allowedAddressesPage(0)
   ,m_bcastPage(0)
   ,m_intervalPage(0)
   ,m_advancedPage(0)
   ,m_finalPage(0)
   ,m_nicListBox(0)
   ,m_trustedHostsLabel(0)
   ,m_ping(0)
   ,m_nmblookup(0)
   ,m_pingAddresses(0)
   ,m_allowedAddresses(0)
   ,m_bcastAddress(0)
   ,m_manualAddress(0)
   ,m_updatePeriod(0)
   ,m_deliverUnnamedHosts(0)
   ,m_firstWait(0)
   ,m_maxPingsAtOnce(0)
   ,m_secondScan(0)
   ,m_secondWait(0)
   ,m_nics(0)
   ,m_configInfo(configInfo)
{
   QString title( i18n("LISa Network Neighborhood Setup") );
   setCaption(title);

   m_configInfo->clear();
   setupPage1();
   addPage( m_page1, title);

   setupAdvancedSettingsPage();
   addPage( m_advancedPage, i18n("Advanced Settings"));

}

SetupWizard::~SetupWizard()
{}

void SetupWizard::clearAll()
{
   showPage(m_page1);
   if (m_nicListBox)
      m_nicListBox->clear();

   if (m_manualAddress)
      m_manualAddress->setText("");

   if (m_ping)
      m_ping->setChecked(false);
   if (m_nmblookup)
      m_nmblookup->setChecked(false);

   if (m_pingAddresses)
      m_pingAddresses->setText("");
   if (m_allowedAddresses)
      m_allowedAddresses->setText("");
   if (m_bcastAddress)
      m_bcastAddress->setText("");
   if (m_updatePeriod)
      m_updatePeriod->setValue(300);

   if (m_deliverUnnamedHosts)
      m_deliverUnnamedHosts->setChecked(false);
   if (m_firstWait)
      m_firstWait->setValue(10);
   if (m_maxPingsAtOnce)
      m_maxPingsAtOnce->setValue(256);
   if (m_secondScan)
      m_secondScan->setChecked(false);
   if (m_secondWait)
   {
      m_secondWait->setEnabled(false);
      m_secondWait->setValue(0);
   }

   if (m_nics)
   {
      delete m_nics;
      m_nics=0;
   }
}

void SetupWizard::setupPage1()
{
   m_page1 = new QVBox( this );

   new QLabel(i18n("<qt><p>This wizard will ask you a few questions about your network.</p> "
                   "<p>Usually you can simply keep the suggested settings.</p> "
                   "<p>After you have finished the wizard, you will be able to browse and use "
                   "shared resources on your LAN, not only Samba/Windows shares, but also "
                   "FTP, HTTP and NFS resources exactly the same way.</p> "
                   "<p>Therefore you need to setup the <i>LAN Information Server</i> (LISa) on your machine. "
                   "Think of the LISa server as an FTP or HTTP server; "
                   "it has to be run by root, it should be started during the boot process and "
                   "only one LISa server can run on one machine.</qt>"),
              m_page1);

   QWidget *dummy=new QWidget(m_page1);
   m_page1->setStretchFactor(dummy,10);
   m_page1->setSpacing(KDialog::spacingHint());
   m_page1->setMargin(KDialog::marginHint());

   setNextEnabled( m_page1, true );
   setHelpEnabled( m_page1, false );
}

void SetupWizard::setupMultiNicPage()
{
   m_multiNicPage=new QVBox(this);
   new QLabel(i18n("<qt><p>More than one network interface card was found on your system.</p>"
                   "<p>Please choose the one to which your LAN is connected.</p></qt>"),
              m_multiNicPage);

   m_multiNicPage->setMargin(KDialog::marginHint());
   m_multiNicPage->setSpacing(KDialog::spacingHint());

   m_nicListBox=new QListBox(m_multiNicPage);
   m_nicListBox->setSelectionMode(QListBox::Single);
   setHelpEnabled( m_multiNicPage, false );
}

void SetupWizard::setupNoNicPage()
{
   m_noNicPage=new QVBox(this);
   new QLabel(i18n("<qt><p><b>No network interface card was found on your system.</b></p>"
                   "<p>Possible reason: no network card is installed. You probably want to cancel now "
                   "or enter your IP address and network manually</p>"
                   "Example: <code>192.168.0.1/255.255.255.0</code>.</qt>"),
              m_noNicPage);

   m_noNicPage->setMargin(KDialog::marginHint());
   m_noNicPage->setSpacing(KDialog::spacingHint());
   m_manualAddress=new KRestrictedLine(m_noNicPage,"0123456789./");
   connect(m_manualAddress,SIGNAL(textChanged(const QString&)),this,SLOT(checkIPAddress(const QString&)));
   QWidget *dummy=new QWidget(m_noNicPage);
   m_noNicPage->setStretchFactor(dummy,10);

   setNextEnabled(m_noNicPage, false);
   setHelpEnabled(m_noNicPage, false);
}


void SetupWizard::setupSearchPage()
{
   m_searchPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("There are two ways LISa can search hosts on your network."),m_searchPage);
   info->setTextFormat(Qt::RichText);
   m_ping=new QCheckBox(i18n("Send pings"), m_searchPage);
   info=new QLabel(i18n("All hosts with TCP/IP will respond,<br>"
                        "whether or not they are samba servers.<br>"
                        "Don\'t use it if your network is very large, i.e. more than 1000 hosts.<br>"),m_searchPage);
   m_nmblookup=new QCheckBox(i18n("Send NetBIOS broadcasts"),m_searchPage);
   info=new QLabel(i18n("You need to have the samba package (nmblookup) installed.<br>"
                        "Only samba/windows servers will respond.<br>"
                        "This method is not very reliable.<br>"
                        "You should enable it if you are part of a large network."),m_searchPage);
   info->setTextFormat(Qt::RichText);
   QWidget *dummy=new QWidget(m_searchPage);
   m_searchPage->setStretchFactor(dummy,10);
   m_searchPage->setSpacing(KDialog::spacingHint());
   m_searchPage->setMargin(KDialog::marginHint());
   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"),m_searchPage);
   info->setAlignment(AlignRight|AlignVCenter);

   setHelpEnabled( m_searchPage, false );
}

void SetupWizard::setupAddressesPage()
{
   m_addressesPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("All IP addresses included in the specified range will be pinged.<br>"
                                "If you are part of a small network, e.g. with network mask 255.255.255.0<br>"
                                "use your IP address/network mask.<br>"),m_addressesPage);
   info->setTextFormat(Qt::RichText);
   m_pingAddresses=new KRestrictedLine(m_addressesPage,"0123456789./;-");
   info=new QLabel(i18n("<br>There are four ways to specify address ranges:<br>"
                        "1. IP address/network mask, like <code>192.168.0.0/255.255.255.0;</code><br>"
                        "2. single IP addresses, like <code>10.0.0.23;</code><br>"
                        "3. continuous ranges, like <code>10.0.1.0-10.0.1.200;</code><br>"
                        "4. ranges for each part of the address, like <code>10-10.1-5.1-25.1-3;</code><br>"
                        "You can also enter combinations of 1 to 4, separated by \";\", like<br>"
                        "<code>192.168.0.0/255.255.255.0;10.0.0.0;10.0.1.1-10.0.1.100;</code><br>"),m_addressesPage);
   info->setAlignment(AlignLeft|AlignVCenter|WordBreak);
   QWidget *dummy=new QWidget(m_addressesPage);
   m_addressesPage->setStretchFactor(dummy,10);
   m_addressesPage->setSpacing(KDialog::spacingHint());
   m_addressesPage->setMargin(KDialog::marginHint());
   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"), m_addressesPage);
   info->setAlignment(AlignRight|AlignVCenter);

   setHelpEnabled( m_addressesPage, false );
}

void SetupWizard::setupAllowedPage()
{
   m_allowedAddressesPage=new QVBox(this);
   QLabel* info=new QLabel(i18n("This is a security related setting.<br>"
                                "It provides a simple IP address based way to specify \"trusted\" hosts.<br>"
                                "Only hosts which fit into the addresses given here are accepted by LISa as clients. "
                                "The list of hosts published by LISa will also only contain hosts which fit into this scheme.<br>"
                                "Usually you enter your IP address/network mask here."),m_allowedAddressesPage);
   info->setAlignment(AlignLeft|AlignVCenter|WordBreak);
   m_allowedAddresses=new KRestrictedLine(m_allowedAddressesPage,"0123456789./-;");
   m_trustedHostsLabel=new QLabel(m_allowedAddressesPage);

   QWidget *dummy=new QWidget(m_allowedAddressesPage);
   m_allowedAddressesPage->setStretchFactor(dummy,10);
   m_allowedAddressesPage->setSpacing(KDialog::spacingHint());
   m_allowedAddressesPage->setMargin(KDialog::marginHint());
   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"), m_allowedAddressesPage);
   info->setAlignment(AlignRight|AlignVCenter);

   setHelpEnabled( m_allowedAddressesPage, false );
}

void SetupWizard::setupBcastPage()
{
   m_bcastPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("<br>Enter your IP address and network mask here, like <code>192.168.0.1/255.255.255.0</code>"),m_bcastPage);
   info->setAlignment(AlignLeft|AlignVCenter|WordBreak);
   m_bcastAddress=new KRestrictedLine(m_bcastPage,"0123456789./");
   info=new QLabel(i18n("<br>To reduce the network load, the LISa servers in one network<br>"
                        "cooperate with each other. Therefore you have to enter the broadcast<br>"
                        "address here. If you are connected to more than one network, choose <br>"
                        "one of the broadcast addresses."),m_bcastPage);
   info->setAlignment(AlignLeft|AlignVCenter|WordBreak);
   QWidget *dummy=new QWidget(m_bcastPage);
   m_bcastPage->setStretchFactor(dummy,10);
   m_bcastPage->setSpacing(KDialog::spacingHint());
   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"), m_bcastPage);
   info->setAlignment(AlignRight|AlignVCenter);

   m_bcastPage->setSpacing(KDialog::spacingHint());
   m_bcastPage->setMargin(KDialog::marginHint());
   setHelpEnabled( m_bcastPage, false );
}

void SetupWizard::setupUpdateIntervalPage()
{
   m_intervalPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("<br>Enter the interval after which LISa, if busy, will update its host list."),m_intervalPage);
   info->setTextFormat(Qt::RichText);

   m_updatePeriod=new QSpinBox(300,1800,10,m_intervalPage);
   m_updatePeriod->setSuffix(i18n(" sec"));

   info=new QLabel(i18n("<br>Please note that the update interval will grow automatically by "
                        "up to 16 times the value you enter here, if nobody accesses the LISa server. "
                        "So if you enter 300 sec = 5 min here, this does not mean that LISa will ping "
                        "your whole network every 5 minutes. The interval will increase up to 16 x 5 min = 80 min."),m_intervalPage);
   info->setAlignment(AlignLeft|AlignVCenter|WordBreak);
   QWidget *dummy=new QWidget(m_intervalPage);
   m_intervalPage->setStretchFactor(dummy,10);
   m_intervalPage->setSpacing(KDialog::spacingHint());
   m_intervalPage->setMargin(KDialog::marginHint());
   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"), m_intervalPage);
   info->setAlignment(AlignRight|AlignVCenter);
   info->setTextFormat(Qt::RichText);

   setHelpEnabled( m_intervalPage, false );
}

void SetupWizard::setupAdvancedSettingsPage()
{
   m_advancedPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("This page contains several settings you usually only<br>"
                                "need if LISa doesn't find all hosts in your network."),m_advancedPage);
   info->setTextFormat(Qt::RichText);
   m_deliverUnnamedHosts=new QCheckBox(i18n("Re&port unnamed hosts"),m_advancedPage);
   info=new QLabel(i18n("Should hosts for which LISa can\'t resolve the name be included in the host list?<br>"),m_advancedPage);

   QHBox* hbox=new QHBox(m_advancedPage);

   info=new QLabel(i18n("Wait for replies after first scan"),hbox);
   m_firstWait=new QSpinBox(10,1000,50,hbox);
   m_firstWait->setSuffix(i18n(" ms"));
   info=new QLabel(i18n("How long should LISa wait for answers to pings?<br>"
                        "If LISa doesn\'t find all hosts, try to increase this value.<br>"),m_advancedPage);

   hbox=new QHBox(m_advancedPage);
   info=new QLabel(i18n("Max. number of pings to send at once"),hbox);
   info->setTextFormat(Qt::RichText);
   m_maxPingsAtOnce=new QSpinBox(8,1024,5,hbox);
   info=new QLabel(i18n("How many ping packets should LISa send at once?<br>"
                        "If LISa doesn't find all hosts you could try to decrease this value.<br>"),m_advancedPage);

   m_secondScan=new QCheckBox(i18n("Al&ways scan twice"),m_advancedPage);

   hbox=new QHBox(m_advancedPage);
//   hbox->setSpacing(10); // WTF?
   info=new QLabel(i18n("Wait for replies after second scan"),hbox);
   info->setTextFormat(Qt::RichText);
   m_secondWait=new QSpinBox(0,1000,50,hbox);
   m_secondWait->setSuffix(i18n(" ms"));
   info=new QLabel(i18n("If LISa doesn't find all hosts, enable this option."),m_advancedPage);

   //this would make the dialog to large
   //m_advancedPage->setSpacing(KDialog::spacingHint());
   //m_advancedPage->setMargin(KDialog::marginHint());

   info=new QLabel(i18n("<b>If unsure, keep it as is.</b>"), m_advancedPage);
   info->setAlignment(AlignRight|AlignVCenter);

   connect(m_secondScan,SIGNAL(toggled(bool)),m_secondWait,SLOT(setEnabled(bool)));
   setHelpEnabled( m_advancedPage, false );
}

void SetupWizard::setupFinalPage()
{
   m_finalPage=new QVBox(this);
   QLabel *info=new QLabel(i18n("<br>Your LAN browsing has been successfully set up.<br><br>"
                                "Make sure that the LISa server is started during the "
                                "boot process. How this is done depends on your "
                                "distribution and OS. Usually you have to insert it somewhere "
                                "in a boot script under <code>/etc</code>.<br>"
                                "Start the LISa server as root and without any command line options.<br>"
                                "The config file will now be saved to <code>/etc/lisarc</code>.<br>"
                                "To test the server, try <code>lan:/</code> in Konqueror.<br><br>"
                                "If you have problems or suggestions, visit http://lisa-home.sourceforge.net."),m_finalPage);
   info->setTextFormat(Qt::RichText);
   QWidget *dummy=new QWidget(m_finalPage);
   m_finalPage->setStretchFactor(dummy,10);
   m_finalPage->setSpacing(KDialog::spacingHint());
   m_finalPage->setMargin(KDialog::marginHint());

   setHelpEnabled( m_finalPage, false );
}

void SetupWizard::next()
{
   if (currentPage()==m_page1)
   {
      if (m_noNicPage==0)
         setupRest();

      setAppropriate(m_noNicPage,false);
      setAppropriate(m_multiNicPage,false);
      if (m_nics!=0)
         delete m_nics;
      m_nics=findNICs();

      if (m_nics->count()==0)
      {
         setAppropriate(m_noNicPage,true);
      }
      else if (m_nics->count()==1)
      {
         //still easy
         //if the host part is less than 20 bits simply take it
         MyNIC *nic=m_nics->first();
         LisaConfigInfo lci;
         suggestSettingsForNic(nic,lci);
         applyLisaConfigInfo(lci);
      }
      else
      {
         //more than one nic
         setAppropriate(m_multiNicPage,true);
         m_nicListBox->clear();
         for (MyNIC *nic=m_nics->first(); nic!=0; nic=m_nics->next())
         {
            QString tmp=nic->name+": "+nic->addr+"/"+nic->netmask+";";
            m_nicListBox->insertItem(tmp);
         }
         m_nicListBox->setSelected(0,true);
      }
   }
   else if (currentPage()==m_multiNicPage)
   {
      QString nic=m_nicListBox->currentText();
      unsigned int i=0;
      for (i=0; i<m_nicListBox->count(); i++)
      {
         if (m_nicListBox->isSelected(i))
         {
            nic=m_nicListBox->text(i);
            break;
         }
      }
      MyNIC* thisNic=m_nics->at(i);
      LisaConfigInfo lci;
      suggestSettingsForNic(thisNic,lci);
      applyLisaConfigInfo(lci);
   }
   else if (currentPage()==m_noNicPage)
   {
      LisaConfigInfo lci;
      suggestSettingsForAddress(m_manualAddress->text(),lci);
      applyLisaConfigInfo(lci);
   }
   else if (currentPage()==m_searchPage)
      setAppropriate(m_addressesPage, m_ping->isChecked());
   else if (currentPage()==m_intervalPage)
   {
      if (m_finalPage==0)
      {
         setupFinalPage();
         addPage( m_finalPage, i18n("Congratulations!"));
      }
      setAppropriate(m_advancedPage, m_ping->isChecked());
   }
   QWizard::next();
}

void SetupWizard::showPage(QWidget* page)
{
   if (page==m_noNicPage)
   {
      m_manualAddress->setFocus();
      setNextEnabled(m_noNicPage, false);
      setHelpEnabled(m_noNicPage, false);
   }
   else if (page==m_multiNicPage)
      m_nicListBox->setFocus();
   else if ( page == m_searchPage)
      m_ping->setFocus();
   else if (page==m_addressesPage)
      m_pingAddresses->setFocus();
   else if (page==m_allowedAddressesPage)
   {
      QString text;
      if (m_ping->isChecked())
         text+=i18n("You can use the same syntax as on the previous page.<br>");
      else
         text+=i18n("There are three ways to specify IP addresses:<br>"
                    "1. IP address/network mask, like<code> 192.168.0.0/255.255.255.0;</code><br>"
                    "2. continuous ranges, like<code> 10.0.1.0-10.0.1.200;</code><br>"
                    "3. single IP addresses, like<code> 10.0.0.23;</code><br>"
                    "You can also enter combinations of 1 to 3, separated by \";\", <br>"
                    "like<code> 192.168.0.0/255.255.255.0;10.0.0.0;10.0.1.1-10.0.1.100;</code><br>");
      m_trustedHostsLabel->setText(text);
      m_allowedAddresses->setFocus();
   }
   else if (page==m_bcastPage)
      m_bcastAddress->setFocus();
   else if (page==m_intervalPage)
      m_updatePeriod->setFocus();
   else if (page==m_advancedPage)
      m_deliverUnnamedHosts->setFocus();
   else if (page==m_finalPage)
      setFinishEnabled(m_finalPage,true);

   QWizard::showPage(page);
}

void SetupWizard::setupRest()
{
   removePage(m_advancedPage);

   setupMultiNicPage();
   addPage( m_multiNicPage, i18n("Multiple Network Interfaces Found"));

   setupNoNicPage();
   addPage( m_noNicPage, i18n("No Network Interface Found"));
   setNextEnabled(m_noNicPage, false);
   setHelpEnabled(m_noNicPage, false);

   setupSearchPage();
   addPage( m_searchPage, i18n("Specify Search Method"));

   setupAddressesPage();
   addPage( m_addressesPage, i18n("Specify Address Range LISa Will Ping"));

   setupAllowedPage();
   addPage( m_allowedAddressesPage, i18n("\"Trusted\" Hosts"));

   setupBcastPage();
   addPage( m_bcastPage, i18n("Your Broadcast Address"));

   setupUpdateIntervalPage();
   addPage( m_intervalPage, i18n("LISa Update Interval"));

   addPage( m_advancedPage, i18n("Advanced Settings"));
}

void SetupWizard::accept()
{
   if (m_ping->isChecked())
   {
      m_configInfo->pingAddresses=m_pingAddresses->text();
      m_configInfo->secondScan=m_secondScan->isChecked();
      if (m_configInfo->secondScan)
         m_configInfo->secondWait=(m_secondWait->value()+5)/10;
      else
         m_configInfo->secondWait=0;
      m_configInfo->firstWait=(m_firstWait->value()+5)/10;
   }
   else
   {
      m_configInfo->pingAddresses="";
      m_configInfo->secondScan=false;
      m_configInfo->secondWait=0;
      m_configInfo->firstWait=30;
   }

   m_configInfo->broadcastNetwork=m_bcastAddress->text();
   m_configInfo->allowedAddresses=m_allowedAddresses->text();
   m_configInfo->maxPingsAtOnce=m_maxPingsAtOnce->value();
   m_configInfo->updatePeriod=m_updatePeriod->value();
   m_configInfo->useNmblookup=m_nmblookup->isChecked();
   m_configInfo->unnamedHosts=m_deliverUnnamedHosts->isChecked();
   QWizard::accept();
}

void SetupWizard::checkIPAddress(const QString& addr)
{
   QString address=addr.simplifyWhiteSpace();
   QRegExp regex("^\\d+\\.\\d+\\.\\d+\\.\\d+\\s*/\\s*\\d+\\.\\d+\\.\\d+\\.\\d+$");
   setNextEnabled(m_noNicPage, (regex.search(address,0)!=-1));
//   setNextEnabled(m_noNicPage, (regex.find(address,0)!=-1));
}

void SetupWizard::applyLisaConfigInfo(LisaConfigInfo& lci)
{
   m_ping->setChecked(!lci.pingAddresses.isEmpty());
   m_pingAddresses->setText(lci.pingAddresses);
   m_nmblookup->setChecked(lci.useNmblookup);
   m_allowedAddresses->setText(lci.allowedAddresses);
   m_bcastAddress->setText(lci.broadcastNetwork);
   m_updatePeriod->setValue(lci.updatePeriod);
   m_deliverUnnamedHosts->setChecked(lci.unnamedHosts);
   m_firstWait->setValue(lci.firstWait*10);
   m_maxPingsAtOnce->setValue(lci.maxPingsAtOnce);
   m_secondWait->setValue(lci.secondWait*10);
   m_secondScan->setChecked(lci.secondScan);
   m_secondWait->setEnabled(lci.secondScan);
}

#include "setupwizard.moc"

