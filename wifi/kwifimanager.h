/***************************************************************************
          kwifimanager.h  -  a graphical interface for wireless LAN cards
                             -------------------
    begin                : Sam Apr  7 11:44:20 CEST 2001
    copyright            : (C) 2001 by Stefan Winter
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

#ifndef KWIFIMANAGER_H
#define KWIFIMANAGER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt
class QLabel;
class QTimer;
class QPixmap;
class QPushButton;
class QStringList;

// include files for KDE
#include <kmainwindow.h>
class KSystemTray;
class KToggleAction;
class KProcess;

// application specific includes
#include "interface_wireless.h"
#include "interface_dcop.h"
class Strength;
class Status;
class Picture;
class Locator;
class Speed;
class Statistics;
class NetworkScanning;
class Led;

extern QStringList usedInterfacesList();

enum speedlevels
{ AUTO, M1, M2, M55, M11 };
enum packetmode
{ UNI, MULTI, BOTH };
enum cryptomodes
{ OPEN, RESTRICTED };

struct configuration_data
{
  QString net_name[5];
  QString disp_name[5];
  bool managed[5];
  bool crypto_enabled[5];
  int active_crypto_key[5];
  QString active_key_string[5];
  QString crypto1[5], crypto2[5], crypto3[5], crypto4[5];
  bool crypto_is_string1[5], crypto_is_string2[5], crypto_is_string3[5],
    crypto_is_string4[5];
  cryptomodes crypto_mode[5];
  bool pm_enabled[5];
  int sleep_time[5];
  QString sleep_time_string[5];
  int wake_time[5];
  QString wake_time_string[5];
  speedlevels bitrate[5];
  packetmode packet[5];
};

class KWiFiManagerApp:public KMainWindow, virtual public dcop_interface
{
Q_OBJECT public:
  KWiFiManagerApp (QWidget * parent = 0, const char * name = 0);
   ~KWiFiManagerApp ();
  QString interface () const { return device ? device->get_interface_name() : QString::null; };
  public:
  bool startDocked() { return m_startDocked; }
  public slots:
  void slotToggleShowStrengthNumber ();
  void slotDisableRadio ();
  void slotFileQuit ();
  void slotStartConfigEditor ();
  void slotStartStatViewer ();
  void slotToggleTric ();
  void slotToggleStrengthCalc ();
  void slotToggleStayInSystray ();
  void slotChangeTrayIcon ();
  void slotChangeWindowCaption ();
  void slotLogESSID (QString essid);
  void slotTXPowerChanged ();
  void slotShowStatsNoise ();
  void slotNetworkScan();
  void tricorder_beep ();
  void slotDisablePowerProcessExited();
  virtual bool queryClose();
  virtual bool queryExit();
  virtual void readProperties( KConfig* );
  virtual void saveProperties( KConfig* );
private:
  void initActions ();
  void initView ();
  Speed *speedmeter;
  QBoxLayout *bla;
  QLabel *profil;
  Status *status;
  Strength *strength;
  Picture *pictogram;
  Locator *location;
  NetworkScanning* scanwidget;
  QWidget *view;
  Statistics *statistik;
  KToggleAction *fileDisableRadio;
  KToggleAction *settingsUseAlternateCalc;
  KToggleAction *settingsStayInSystrayOnClose;
  KToggleAction *settingsAcousticScanning;
  KToggleAction *settingsShowStatsNoise;
  KToggleAction *settingsShowStrengthNumber;
  KSystemTray *trayicon;
  void init_whois_db ();
  QTimer *tricorder_trigger, *counter;
  QPixmap *pixmap;
  Interface_wireless *device;
  QPushButton * scan;
  Led *led;
  KProcess* disablePower;
  bool showStrength;
  bool showStatsNoise;
  bool m_startDocked;
  bool m_shuttingDown;
  int m_iconSize;
};

#endif /* KWIFIMANAGER_H */
