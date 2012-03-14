/***************************************************************************
                          stuff.h  -  description
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

#ifndef INTERFACE_WIRELESS_H
#define INTERFACE_WIRELESS_H

#include "config.h"

#include <qobject.h>
#include <qstring.h>
#include <qtable.h>

#ifndef WITHOUT_ARTS
#include <arts/iomanager.h>
#include <arts/dispatcher.h>
#endif

const int POLL_DELAY_MS = 250;	// how much time between device polls
const int MAX_HISTORY = 240;	// number of device states to be saved

class Interface_wireless:public QObject
{
Q_OBJECT public:
  Interface_wireless (QStringList * ignoreInterfaces);
  virtual ~ Interface_wireless() { };

  virtual bool get_device_freq (double &freq);
  // true: device returned valid frequency
  virtual bool get_mode (int &mode);
  // true: device returned valid mode info
  virtual bool get_key (QString & key, int &size, int &flags);
  // true: device has set up a valid crypto key
  virtual QString get_essid ();
  virtual bool get_AP_info (QString & mac, QString & ip);
  // true: could retrieve IP, false: returned IP info invalid
  virtual QString get_IP_info ();
  virtual double get_bitrate ();
  virtual QString get_interface_name ();
  // if no device is attached, return empty QString
  virtual bool get_current_quality (int &sig, int &noi, int &qua);
  // quality info is only valid when true
  virtual QTable* get_available_networks () = 0;
  virtual bool get_has_txpower();
  virtual int get_txpower_disabled();
  virtual void setActiveDevice( QString device ) = 0;
  // stats
  int sigLevel[MAX_HISTORY];
  int sigLevelMin, sigLevelMax;
  int noiseLevel[MAX_HISTORY];
  int noiseLevelMin, noiseLevelMax;
  int qual[MAX_HISTORY];
  bool valid[MAX_HISTORY];
  int current;
  
public slots:
  virtual bool poll_device_info () = 0;

signals:
  void interfaceChanged ();
  void strengthChanged ();
  void statusChanged ();
  void modeChanged ();
  void speedChanged ();
  void essidChanged (QString essid);
  void txPowerChanged ();
  void statsUpdated ();
protected:
  bool already_warned;
  // device info
  bool has_frequency;
  float frequency;
  bool has_mode;
  int mode;
  bool has_key;
  QString key;
  int key_size;
  int key_flags;
  QString essid;
  QString access_point_address;
  QString ip_address;
  double bitrate;
  QString interface_name;
  int socket;
  bool has_range;
  int range;
  QStringList * ignoreInterfaces;
  bool has_txpower;
  int txpower_disabled;
};

#ifndef WITHOUT_ARTS

void sinus_wave (double frequency);

class MyTimeNotify:public
  Arts::TimeNotify
{
public:
  Arts::Dispatcher * test;
  MyTimeNotify (Arts::Dispatcher * siff)
  {
    test = siff;
  };
  void notifyTime ();
};

#endif

QString whois (const char *MAC_ADR, QStringList APList);

#endif /*  INTERFACE_WIRELESS_H */
