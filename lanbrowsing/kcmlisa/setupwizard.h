/*
 * main.cpp for lisa,reslisa,kio_lan and kio_rlan kcm module
 *
 *  Copyright (C) 2000 Alexander Neundorf <neundorf@kde.org>
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

#ifndef LISA_SETUPWIZARD_H
#define LISA_SETUPWIZARD_H

#include "kcmlisa.h"
#include "kcmreslisa.h"
#include "kcmkiolan.h"

#include "findnic.h"

#include <qspinbox.h>
#include <qcheckbox.h>
#include <krestrictedline.h>
#include <qwizard.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qvbox.h>

#include <kcmodule.h>


class SetupWizard:public QWizard
{
   Q_OBJECT
   public:
      SetupWizard(QWidget* parent, LisaConfigInfo* configInfo);
      ~SetupWizard();
      virtual void showPage(QWidget* page);
      void clearAll();
   protected slots:
      virtual void next();
      virtual void accept();
      void checkIPAddress(const QString& address);
   protected:
      void applyLisaConfigInfo(LisaConfigInfo& lci);

      void setupPage1();
      void setupNoNicPage();
      void setupMultiNicPage();
      void setupSearchPage();
      void setupAddressesPage();
      void setupAllowedPage();
      void setupBcastPage();
      void setupUpdateIntervalPage();
      void setupAdvancedSettingsPage();
      void setupFinalPage();
      void setupRest();

      QVBox* m_page1;
      QVBox* m_noNicPage;
      QVBox* m_multiNicPage;
      QVBox* m_searchPage;
      QVBox* m_addressesPage;
      QVBox* m_allowedAddressesPage;
      QVBox* m_bcastPage;
      QVBox* m_intervalPage;
      QVBox* m_advancedPage;
      QVBox* m_finalPage;


      QListBox *m_nicListBox;
      QLabel *m_trustedHostsLabel;
      QCheckBox *m_ping;
      QCheckBox *m_nmblookup;
      KRestrictedLine* m_pingAddresses;
      KRestrictedLine* m_allowedAddresses;
      KRestrictedLine* m_bcastAddress;
      KRestrictedLine* m_manualAddress;
      QSpinBox* m_updatePeriod;
      QCheckBox* m_deliverUnnamedHosts;
      QSpinBox* m_firstWait;
      QSpinBox* m_maxPingsAtOnce;
      QCheckBox* m_secondScan;
      QSpinBox* m_secondWait;

      NICList* m_nics;
      LisaConfigInfo* m_configInfo;
};


#endif

