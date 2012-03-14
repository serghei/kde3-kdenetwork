/*
  Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>
            (C) 2001 by Stefan Winter <mail@stefan-winter.de>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef KCMWIFI_H
#define KCMWIFI_H

#include <kcmodule.h>

#include <qstring.h>

class IfConfigPage;
class MainConfig;
class QTabWidget;
class KProcIO;

class KCMWifi : public KCModule
{
    Q_OBJECT
public:
  KCMWifi (QWidget * parent, const char *name, const QStringList &);
  ~KCMWifi ();

  void load();
  void save();
  void defaults();
  int buttons();

  static void activate();
  IfConfigPage* addConfigTab( int count, bool vendor );
  void delConfigTab( int count );

  static const int vendorBase = 10; 

protected slots:
  void slotActivate();
  void slotChanged();

private:
  MainConfig *m_mainConfig;
  IfConfigPage *m_ifConfigPage[ 15 ]; // that makes 5 vendor configs
  QTabWidget* tabs;
  int m_activeVendorCount;
};

#endif // KCMWIFI_H
