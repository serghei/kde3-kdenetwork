/***************************************************************************
                          kwifimanager.cpp  -  description
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

// include files for C++
#include <iostream>
#include <fstream>

// include files for QT
#include <qdatetime.h>
#include <qdir.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qpushbutton.h>

// include files for KDE
#include <kapplication.h>
#include <kconfig.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>
#include <ksystemtray.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kdialog.h>

// application specific includes
#include "asusled.h"
#include "networkscanning.h"
#include "kwifimanager.h"
#include "interface_wireless.h"
#include "interface_wireless_wirelessextensions.h"
#include "status.h"
#include "locator.h"
#include "picture.h"
#include "strength.h"
#include "speed.h"
#include "statistics.h"

#define ID_STATUS_MSG 1

QStringList APs;
bool useAlternateStrengthCalc;

KWiFiManagerApp::KWiFiManagerApp (QWidget *, const char *name):
DCOPObject("dcop_interface"), KMainWindow (0, name), device(0), m_shuttingDown(false), m_iconSize( 22 )
{
  statistik = 0;
  disablePower = 0;
  useAlternateStrengthCalc = false;
  init_whois_db ();

  QStringList *ignoreInterfaces;
  if (kapp->isRestored())
  	ignoreInterfaces = new QStringList();
  else
	ignoreInterfaces = new QStringList( usedInterfacesList() );

  // substitute the following line with a different interface if required
  device = new Interface_wireless_wirelessextensions (ignoreInterfaces);

   //This setting gets here from the kdeglobals config
  KConfig* config = kapp->config();
  if (config->hasGroup("System Tray"))
      config->setGroup("System Tray");
  m_iconSize=config->readNumEntry("systrayIconWidth", 22);

  /* initView updates the main widget, system tray icon and the ASUS led if applicable
     it needs to called before initActions() because that uses some icons that get
     initialized by initView() */

  initView ();

  initActions ();

#if WIRELESS_EXT > 9
  fileDisableRadio->setEnabled (true);
#else
  fileDisableRadio->setEnabled (false);
#endif

  counter = new QTimer (this);
  connect (counter, SIGNAL (timeout ()), device, SLOT (poll_device_info()));
  counter->start (1000);

  tricorder_trigger = new QTimer ();
  connect (tricorder_trigger, SIGNAL (timeout ()), this,
	   SLOT (tricorder_beep ()));
  connect (trayicon, SIGNAL (quitSelected ()), this, SLOT (slotFileQuit ()));
  slotToggleTric(); //set to value saved by KConfig

  setAutoSaveSettings();
  kapp->setMainWidget(this);
}


void
KWiFiManagerApp::slotChangeWindowCaption ()
{
  if (device->get_interface_name().isEmpty())
    {
      this->setCaption (i18n ("No Interface"));
    }
  else
    {
      this->setCaption (i18n ("Interface %1").
			arg (device->get_interface_name ()));
    }
}

KWiFiManagerApp::~KWiFiManagerApp ()
{
  kdDebug () <<
    "Thank you for using KWiFiManager. You have made a simple program very happy." << endl;
  delete statistik;
  delete tricorder_trigger;
}

void
KWiFiManagerApp::init_whois_db ()
{
  KStandardDirs finder;
  QString kdedir =
    finder.findResourceDir ("data", "kwifimanager/locations/NOWHERE.loc");
  kdedir += "kwifimanager/locations/";
  QDir *locations = new QDir (kdedir);
  if (!locations->exists ())
    kdDebug () <<
      "A grave error occurred while searching for files with AccessPoint information." << endl;
  QStringList *entries = new QStringList (locations->entryList ());
  QString content;
  QString content2;
  for (QStringList::Iterator it = entries->begin (); it != entries->end ();
       it++)
    {
      if ((*it) != "." && (*it) != "..")
	{
	  QFile datei (kdedir + (*it));
	  datei.open (IO_ReadOnly);
	  while (datei.readLine (content, 255) != -1)
	    {
	      QString temp (content);
	      content.truncate (content.find (' '));
	      content = content.upper ();
	      content2 = temp.right (temp.length () - temp.find (' ') - 1);
	      content2.truncate (content2.length () - 1);
	      APs << content << content2;
	    }
	  datei.close ();
	}
    }
  delete entries;
  delete locations;
}

void
KWiFiManagerApp::slotLogESSID (QString essid)
{
  QFile log_file( locateLocal( "data", "kwifimanager/kwifimanager.log", true ) );
  kdDebug() << "Log file is " << log_file.name() << endl;
  if (!log_file.open (IO_WriteOnly | IO_Append))
    kdDebug () << "Logging failed!" << endl;
  QTextStream stream (&log_file);
  stream << QDateTime::currentDateTime().toString ().latin1 ();
  stream << " Entering network: " << essid.latin1() << endl;
  log_file.close ();
}

void
KWiFiManagerApp::slotTXPowerChanged ()
{
  fileDisableRadio->setChecked( device->get_txpower_disabled() );
}

void
KWiFiManagerApp::slotChangeTrayIcon ()
{
  QPixmap* ICON;
  int sig, noi, str;
  double freq = 0.;
  bool validdata = device->get_current_quality (sig, noi, str);
  if ( settingsUseAlternateCalc->isChecked() ) str = sig - noi;
  int mode;
  QLabel sstrength2( i18n("N/A"), 0, 0 );

  if ( ( device->get_txpower_disabled() ) || ( !device->get_device_freq ( freq ) ) )
    {
      ICON = strength->NOT_CON_OFFLINE_ADHOC;
      if (led->state == true)
	led->Off ();
    }
  else if (device->get_mode(mode) && mode == 1)
    {
      ICON = strength->NOT_CON_OFFLINE_ADHOC;
      if (led->state == true)
	led->Off ();
    }
  else if (validdata && (str > 45))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->EXCELLENT;
      if (led->state == false)
	led->On ();
    }
  else if (validdata && (str > 35))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->EXCELLENT;
      if (led->state == false)
	led->On ();
    }
  else if (validdata && (str > 25))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->EXCELLENT;
      if (led->state == false)
	led->On ();
    }
  else if (validdata && (str > 15))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->GOOD;
      if (led->state == false)
	led->On ();
    }
  else if (validdata && (str > 5))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->MARGINAL;
      if (led->state == false)
	led->On ();
    }
  else if (validdata && (str > 0))
    {
      sstrength2.setText( QString::number(str) );
      ICON = strength->OOR_MINIMUM;
      if (led->state == true)
	led->Off ();
    }
  else if (((str == 0) && (mode == 2)) || (!validdata))
    {
      sstrength2.setText( QString::fromLatin1("0") );
      ICON = strength->OOR_DOWN;
      if (led->state == true)
	led->Off ();
    }
  else
    {
      ICON = strength->NOT_CON_OFFLINE_ADHOC;
      if (led->state == true)
	led->Off ();
    }

      QPixmap temp (m_iconSize+8, m_iconSize+8);
      QPainter bla (&temp);
      bla.fillRect (0, 0, m_iconSize+8, m_iconSize+8, backgroundBrush ());
      bla.drawPixmap ( QRect(1, 4, m_iconSize+2,m_iconSize+2), *ICON );

      if (showStrength) {
  static QFont labelfont ( "Helvetica", 10 );

    labelfont.setStyleHint( QFont::SansSerif );
    labelfont.setStretch( QFont::Condensed );
    labelfont.setBold( true );
    sstrength2.setFont( labelfont );
    sstrength2.setFixedSize( sstrength2.sizeHint() );
      }
    QPixmap labeltemp = QPixmap::grabWidget( &sstrength2 );
    labeltemp.setMask( labeltemp.createHeuristicMask() );
    bla.drawPixmap (0, 0, labeltemp );
  temp.setMask (temp.createHeuristicMask() );
  trayicon->setPixmap ( temp );
    this->setIcon( temp );
  
    QToolTip::add( trayicon, "SSID: "+device->get_essid() );
}

void
KWiFiManagerApp::initActions ()
{
  KConfig* config = kapp->config();
  if (config->hasGroup("General"))
	config->setGroup("General");
  fileDisableRadio = new KToggleAction (i18n ("&Disable Radio"), 0, this,
		     SLOT (slotDisableRadio ()), actionCollection (), "disable_radio");
  fileDisableRadio->setChecked( false );

  settingsUseAlternateCalc =
    new KToggleAction (i18n ("&Use Alternate Strength Calculation"), 0, this,
		       SLOT (slotToggleStrengthCalc ()), actionCollection (), "use_alt_calculation");

  settingsUseAlternateCalc->setChecked( config->readBoolEntry("useAlternateStrengthCalculation") );
  slotToggleStrengthCalc();  //set to value saved by KConfig

  settingsShowStatsNoise =
    new KToggleAction (i18n ("Show &Noise Graph in Statistics Window"), 0, this,
		       SLOT (slotShowStatsNoise ()), actionCollection (), "show_stats_noise");

  settingsShowStatsNoise->setChecked( config->readBoolEntry("showStatsNoise") );
  slotShowStatsNoise();  //set to value saved by KConfig

  settingsShowStrengthNumber = new KToggleAction (i18n ("&Show Strength Number in System Tray"), 0, this,
                       SLOT (slotToggleShowStrengthNumber ()), actionCollection (), "show_strength_number_in_tray");
  settingsShowStrengthNumber->setChecked( config->readBoolEntry("showStrengthNumberInTray") );
  slotToggleShowStrengthNumber (); //set to value saved by KConfig

  KStdAction::quit (this, SLOT (slotFileQuit ()), actionCollection ());

  new KAction (i18n ("Configuration &Editor..."), 0, this,
	       SLOT (slotStartConfigEditor ()), actionCollection (), "configuration_editor");
  new KAction (i18n ("Connection &Statistics"), 0, this,
			       SLOT (slotStartStatViewer ()), actionCollection (), "connection_statistics");
  settingsAcousticScanning = new KToggleAction (i18n ("&Acoustic Scanning"), 0, this,
			    SLOT (slotToggleTric ()),
			    actionCollection (), "acoustic_scanning");
  settingsAcousticScanning->setChecked( config->readBoolEntry("acousticScanning") );
#ifdef WITHOUT_ARTS
  settingsAcousticScanning->setEnabled( false);
#endif
  settingsStayInSystrayOnClose = new KToggleAction (i18n ("Stay in System &Tray on Close"), 0, this,
						    SLOT (slotToggleStayInSystray()),
						    actionCollection (), "stay_in_systray_on_close");
  settingsStayInSystrayOnClose->setChecked( config->readBoolEntry("stayInSystrayOnClose") );
  createGUI();
}


void
KWiFiManagerApp::initView ()
{

  led = new Led ();

  trayicon = new KSystemTray (this);
  trayicon->show ();

  view = new QWidget (this, "mainwidget");
  view->setBackgroundMode (PaletteBackground);
  setCentralWidget (view);
  QGridLayout *zentrallayout = new QGridLayout (view);
  zentrallayout->setMargin( KDialog::marginHint() );
  speedmeter = new Speed (view, device);
  status = new Status (view, device);
  location = new Locator (view, device);
  strength = new Strength (view, device);
  pictogram = new Picture (view, device);
  scan = new QPushButton (i18n("Scan for &Networks..."),view);
  slotChangeWindowCaption ();
  QToolTip::add (strength, i18n ("The current signal strength"));
  QToolTip::add (speedmeter, i18n ("The speed at which the wireless LAN card is operating"));
  QToolTip::add (status, i18n ("Detailed connection status"));
  QToolTip::add (scan, i18n ("Performs a scan to discover the networks you can log into"));
  pictogram->setMinimumSize (pictogram->mySizeHint());
  strength->setMinimumSize (strength->mySizeHint());
  speedmeter->setMinimumHeight (60);
  status->setMinimumHeight (60);
  zentrallayout->addWidget (pictogram, 0, 0);
  zentrallayout->addWidget (strength, 1, 0);
  zentrallayout->addWidget (speedmeter, 0, 1);
  zentrallayout->addMultiCellWidget (status, 1, 2, 1, 1);
  zentrallayout->addWidget (scan, 2, 0);
  zentrallayout->addMultiCellWidget (location, 3, 3, 0, 1, Qt::AlignLeft);
  connect (device, SIGNAL (interfaceChanged ()), this, SLOT (slotChangeWindowCaption ()));
  connect (device, SIGNAL (strengthChanged ()), this, SLOT (slotChangeTrayIcon ()));
  connect (device, SIGNAL (strengthChanged ()), strength, SLOT (repaint ()));
  connect (device, SIGNAL (statusChanged ()), status, SLOT (repaint ()));
  connect (device, SIGNAL (speedChanged ()), speedmeter, SLOT (repaint ()));
  connect (device, SIGNAL (modeChanged ()), pictogram, SLOT (repaint ()));
  connect (device, SIGNAL (essidChanged (QString)), this, SLOT (slotLogESSID (QString)));
  connect (device, SIGNAL (essidChanged (QString)), location, SLOT (repaint ()));
  connect (device, SIGNAL (statusChanged ()), location, SLOT (repaint ()));
  connect (device, SIGNAL (txPowerChanged ()), this, SLOT (slotTXPowerChanged ()));
  connect (device, SIGNAL (txPowerChanged ()), this, SLOT (slotChangeTrayIcon ()));
  connect (device, SIGNAL (txPowerChanged ()), pictogram, SLOT (repaint ()));
  connect (device, SIGNAL (txPowerChanged ()), strength, SLOT (repaint ()));
  connect (scan,  SIGNAL (clicked()), this, SLOT (slotNetworkScan()));
}

void
KWiFiManagerApp::slotToggleShowStrengthNumber ()
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry( "showStrengthNumberInTray", settingsShowStrengthNumber->isChecked() );
  if (settingsShowStrengthNumber->isChecked()) { showStrength = true; }
  else { showStrength = false; }
  slotChangeTrayIcon ();
}

void
KWiFiManagerApp::slotNetworkScan ()
{
  scan->setText( i18n( "Scan in progress..." ) );
  scan->setEnabled ( false );
  scanwidget = new NetworkScanning(device);
  scan->setText( i18n( "Scan for &Networks..." ) );
  scan->setEnabled ( true );
}


void
KWiFiManagerApp::slotStartStatViewer ()
{
  delete statistik;
  statistik = new Statistics (device, showStatsNoise);
  statistik->setFixedSize (590, 300);
  connect(device,SIGNAL(statsUpdated()),statistik,SLOT(repaint()));
  statistik->show ();
}

void
KWiFiManagerApp::slotDisableRadio ()
{
  fileDisableRadio->setChecked( device->get_txpower_disabled() );

  if ( disablePower != 0 && disablePower->isRunning() ) {
	  return;
  }
  QString interface = device->get_interface_name();

  if (interface.isEmpty()) {
	  return;
  }

  QString onOrOff;
  if ( fileDisableRadio->isChecked() ) {
    onOrOff = "on";
  } else {
    onOrOff = "off";
  }

  disablePower = new KProcess;

  // FIXME my Cisco Aironet 350 has two interfaces eth1 and wifi1,
  // kwifimanager works on wifi 1 where txpower does not work -- jriddell

  //*disablePower << "kdesu" << "iwconfig" << "eth1" << "txpower" << onOrOff;

  *disablePower << "kdesu" << "iwconfig" << interface << "txpower" << onOrOff;
  connect( disablePower, SIGNAL(processExited(KProcess*)), this, SLOT(slotDisablePowerProcessExited()) );
  disablePower->start(KProcess::NotifyOnExit);
}

void
KWiFiManagerApp::slotDisablePowerProcessExited()
{
  if (! disablePower->normalExit() || disablePower->exitStatus() != 0) {
    delete disablePower;
    disablePower = 0;
    return;
  }

  if ( fileDisableRadio->isChecked() ) {
	  fileDisableRadio->setChecked(false);
  } else {
	  fileDisableRadio->setChecked(true);
  }

  delete disablePower;
  disablePower = 0;

  fileDisableRadio->setChecked( device->get_txpower_disabled() );
}

void
KWiFiManagerApp::slotToggleTric ()
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry( "acousticScanning", settingsAcousticScanning->isChecked() );
  if (settingsAcousticScanning->isChecked())
    {
      tricorder_trigger->start (250);
    }
  else
    {
      tricorder_trigger->stop ();
    }
}

void KWiFiManagerApp::slotToggleStrengthCalc ()
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry( "useAlternateStrengthCalculation", settingsUseAlternateCalc->isChecked() );
  useAlternateStrengthCalc = settingsUseAlternateCalc->isChecked();
}

void KWiFiManagerApp::slotToggleStayInSystray ()
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry( "stayInSystrayOnClose", settingsStayInSystrayOnClose->isChecked() );
}

void
KWiFiManagerApp::tricorder_beep ()
{
  int sig, noi, qual;
  device->get_current_quality (sig, noi, qual);
#ifndef WITHOUT_ARTS
  sinus_wave (150.0 + qual * 20);
#endif
  if (qual == 0)
    {
      tricorder_trigger->changeInterval (2000);
    }
  else if (qual < 10)
    {
      tricorder_trigger->changeInterval (1000);
    }
  else
    {
      tricorder_trigger->changeInterval (500);
    }
}

void
KWiFiManagerApp::slotShowStatsNoise ()
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry( "showStatsNoise", settingsShowStatsNoise->isChecked() );
  showStatsNoise = settingsShowStatsNoise->isChecked();
}

void
KWiFiManagerApp::slotFileQuit ()
{
  m_shuttingDown = true;
  kapp->quit();
}

void
KWiFiManagerApp::slotStartConfigEditor ()
{
  KProcess startConf;
  startConf << "kdesu" << locate("exe", "kcmshell") << "kcmwifi";
  startConf.start (KProcess::DontCare);
}

void KWiFiManagerApp::saveProperties( KConfig* conf)
{
  m_startDocked = !isVisible();
  conf->writeEntry( "startDocked", m_startDocked );
  conf->writeEntry( "wifidevice", device->get_interface_name() );
  conf->sync();
}

void KWiFiManagerApp::readProperties( KConfig* conf)
{
  m_startDocked = conf->readBoolEntry( "startDocked", false );
  if (m_startDocked)
	show();
  else
	hide();

  QString dev = conf->readEntry( "wifidevice" );
  if (!dev.isEmpty())
	device->setActiveDevice(dev);
}

bool
KWiFiManagerApp::queryClose()
{
  if(!m_shuttingDown &&
       !kapp->sessionSaving() &&
       settingsStayInSystrayOnClose->isChecked() ) {
    hide();
    return false;
  }
  return true;
}

bool
KWiFiManagerApp::queryExit()
{
  // Save settings if auto-save is enabled, and settings have changed
  if ( settingsDirty() && autoSaveSettings() )
	saveAutoSaveSettings();
  return true;
}

// List of network interfaces used by all running kwifimanager applications
QStringList usedInterfacesList()
{
  // Register with DCOP
  DCOPClient *client = kapp->dcopClient();
  client->registerAs( "kwifimanager" );
  client->setDefaultObject( "dcop_interface" );

  // shamelessly stolen from kdelibs/kio/booksmarks/kbookmarkimporter_crash.cc
  QStringList ignoreInterfaces;
  QCStringList apps = client->registeredApplications();
  for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
    {
        QCString &clientId = *it;
        if ( qstrncmp(clientId, "kwifimanager", 12) != 0 )
            continue;

	DCOPRef ask( clientId, "dcop_interface" );
	DCOPReply reply = ask.call( "interface()" );
	QString interface;
	if ( reply.isValid() ) {
		QString i = reply;
		interface = i;
	}
	if (!interface.isEmpty())
		ignoreInterfaces.append(interface);
    }
  return ignoreInterfaces;
}

#include "kwifimanager.moc"
