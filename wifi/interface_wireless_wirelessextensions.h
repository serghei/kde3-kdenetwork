/***************************************************************************
                          interface_wireless_wirelessextensions.h  -  description
                             -------------------
    begin                : Mon Aug 19 2002
    copyright            : (C) 2002 by Stefan Winter
    email                : mail@stefan-winter.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERFACE_WIRELESS_WIRELESSEXTENSIONS_H
#define INTERFACE_WIRELESS_WIRELESSEXTENSIONS_H

#include "config.h"

#include <qstring.h>
#include <qtable.h>

#ifndef WITHOUT_ARTS
#include <arts/iomanager.h>
#include <arts/dispatcher.h>
#endif

#include "interface_wireless.h"

class QTable;
class KProcIO;

extern "C"
{
#include <iwlib.h>
}

class Interface_wireless_wirelessextensions:public Interface_wireless
{
Q_OBJECT public:
  Interface_wireless_wirelessextensions (QStringList * ignoreInterfaces);
  QTable* get_available_networks ();

public:
  void setActiveDevice( QString device );

public slots:  
  bool poll_device_info ();

private:
  bool isSocketOpen();
  void setNoActiveDevice( );
  QStringList available_wifi_devices();
  bool autodetect_device();

  QString print_scanning_token (struct iw_event *event);
  int get_info (int skfd, const QString& interface, struct wireless_info& info);
  QTable* networks;
private slots:
  void parseScanData ( KProcIO * iwlist );
};

#endif /*  INTERFACE_WIRELESS_WIRELESSEXTENSIONS_H */
