/*
 * kcmlisa.h
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

#ifndef KCMLISA_H
#define KCMLISA_H

#include <kconfig.h>
#include <kcmodule.h>

class QPushButton;
class QCheckBox;
class QSpinBox;
class QPushButton;
class KProcess;
class KDialogBase;
class KRestrictedLine;
class KEditListBox;
class SetupWizard;

class LisaSettings:public KCModule
{
   Q_OBJECT
   public:
      LisaSettings(const QString& config, QWidget *parent=0);
      virtual ~LisaSettings() {};
      void load();
      void save();
   signals:
      void changed();
   protected slots:
      void slotChanged();
      void autoSetup();
      void saveDone(KProcess *);  // called after the kdesud returns (on save)
      void suggestSettings();
   protected:
      KConfig m_config;
      QPushButton *m_autoSetup;
      QCheckBox *m_useNmblookup;
      QCheckBox *m_sendPings;
      KRestrictedLine *m_pingAddresses;
      KEditListBox *m_pingNames;
      KRestrictedLine *m_allowedAddresses;
      KRestrictedLine *m_broadcastNetwork;
      QSpinBox *m_firstWait;
      QCheckBox *m_secondScan;
      QSpinBox *m_secondWait;
      QSpinBox *m_updatePeriod;
      QCheckBox *m_deliverUnnamedHosts;
      QSpinBox *m_maxPingsAtOnce;
      QPushButton* m_suggestSettings;
      QPushButton *m_advancedSettingsButton;
      KDialogBase *m_lisaAdvancedDlg;

      SetupWizard *m_wizard;
   private:
      QString m_tmpFilename;
      QString m_configFilename;
      bool    m_changed;
};

#endif

