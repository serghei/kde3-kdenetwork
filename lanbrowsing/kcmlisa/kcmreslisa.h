/*
 * kcmreslisa.h
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

#ifndef KCMRESLISA_H
#define KCMRESLISA_H

#include <kcmodule.h>
#include <kconfig.h>


class QSpinBox;
class QCheckBox;
class QPushButton;
class KEditListBox;
class KDialogBase;
class KRestrictedLine;

class ResLisaSettings:public KCModule
{
   Q_OBJECT
   public:
      ResLisaSettings(const QString& config, QWidget *parent=0);
      virtual ~ResLisaSettings() {}
      void load();
      void save();
   signals:
      void changed();
   protected slots:
      void suggestSettings();
   protected:
      KConfig m_config;
      KConfig m_kiolanConfig;
      QPushButton* m_advancedSettingsButton;
      QPushButton* m_suggestSettings;
      QCheckBox* m_useNmblookup;
      KEditListBox *m_pingNames;

      KRestrictedLine *m_allowedAddresses;
      QSpinBox *m_firstWait;
      QCheckBox *m_secondScan;
      QSpinBox *m_secondWait;
      QSpinBox *m_updatePeriod;
      QCheckBox *m_deliverUnnamedHosts;
      QSpinBox *m_maxPingsAtOnce;
      QCheckBox* m_rlanSidebar;
      KDialogBase* m_reslisaAdvancedDlg;
};

#endif

