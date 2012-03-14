/*
 * kcmkiolan.h
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

#ifndef KCMKIOLAN_H
#define KCMKIOLAN_H

#include "portsettingsbar.h"
#include <kcmodule.h>
#include <kconfig.h>

class QCheckBox;
class QLineEdit;

class IOSlaveSettings:public KCModule
{
   Q_OBJECT
   public:
      IOSlaveSettings(const QString& config, QWidget *parent=0);
      virtual ~IOSlaveSettings() {};
      void load();
      void save();
   signals:
      void changed();
   protected:
      KConfig m_config;
      QCheckBox *m_shortHostnames;
      QCheckBox *m_rlanSidebar;
      PortSettingsBar *m_ftpSettings;
      PortSettingsBar *m_httpSettings;
      PortSettingsBar *m_nfsSettings;
      PortSettingsBar *m_smbSettings;
      PortSettingsBar *m_fishSettings;
      QLineEdit *m_defaultLisaHostLe;
};

#endif
