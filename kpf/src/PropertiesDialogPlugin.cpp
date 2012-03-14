/*
  KPF - Public fileserver for KDE

  Copyright 2001 Rik Hemsley (rikkus) <rik@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qdir.h>
#include <qlineedit.h>

#include <kapplication.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>
#include <kgenericfactory.h>

#include "Defines.h"
#include "Defaults.h"
#include "PropertiesDialogPlugin.h"
#include "StartingKPFDialog.h"
#include "WebServerManager_stub.h"
#include "WebServer_stub.h"
#include "Help.h"

#include <dnssd/servicebrowser.h>

namespace KPF
{
  class ServerState
  {
    public:

      ServerState()
        : shared            (false),
          listenPort        (Config::DefaultListenPort),
          bandwidthLimit    (Config::DefaultBandwidthLimit),
//          connectionLimit   (Config::DefaultConnectionLimit),
          followSymlinks    (Config::DefaultFollowSymlinks)
      {
      }

      bool operator == (const ServerState & other) const
      {
        return
          (
            other.shared          == shared
            &&
            other.listenPort      == listenPort
            &&
            other.bandwidthLimit  == bandwidthLimit
            &&
//            other.connectionLimit == connectionLimit
//            &&
            other.followSymlinks  == followSymlinks
          );
      }

      bool operator != (const ServerState & other) const
      {
        return
          (
            other.shared          != shared
            ||
            other.listenPort      != listenPort
            ||
            other.bandwidthLimit  != bandwidthLimit
            ||
//            other.connectionLimit != connectionLimit
//            ||
            other.followSymlinks  != followSymlinks
          );
      }


      bool  shared;
      uint  listenPort;
      uint  bandwidthLimit;
//      uint  connectionLimit;
      QString serverName;
      bool  followSymlinks;
  };

  class PropertiesDialogPlugin::Private
  {
    public:

      Private()
        : l_listenPort              (0L),
          l_bandwidthLimit          (0L),
//          l_connectionLimit         (0L),
          sb_listenPort             (0L),
          sb_bandwidthLimit         (0L),
//          sb_connectionLimit        (0L),
	  le_serverName             (0L),
          cb_followSymlinks         (0L),
          cb_share                  (0L),
          stack                     (0L),
          initWidget                (0L),
          configWidget              (0L),
          webServerManagerInterface (0L),
          kpfRunning                (false)
      {
      }

      QLabel                * l_listenPort;
      QLabel                * l_bandwidthLimit;
//      QLabel                * l_connectionLimit;
      QLabel                * l_serverName;
      QLabel                * l_kpfStatus;

      QSpinBox              * sb_listenPort;
      QSpinBox              * sb_bandwidthLimit;
//      QSpinBox              * sb_connectionLimit;
      QLineEdit             * le_serverName;

      QCheckBox             * cb_followSymlinks;
      QCheckBox             * cb_share;

      QPushButton           * pb_startKPF;

      QWidgetStack          * stack;
      QWidget               * initWidget;
      QWidget               * configWidget;

      WebServerManager_stub * webServerManagerInterface;

      bool                    kpfRunning;
      DCOPRef                 webServerRef;
      KURL                    url;

      ServerState             currentState;
      ServerState             wantedState;
  };

  PropertiesDialogPlugin::PropertiesDialogPlugin(KPropertiesDialog * dialog,
                                                 const char *,
                                                 const QStringList &)
    : KPropsDlgPlugin(dialog)
  {
    d = new Private;

    d->webServerManagerInterface =
      new WebServerManager_stub("kpf", "WebServerManager");

    d->url = dialog->kurl();

    if (
      d->url == QDir::homeDirPath()
      ||  d->url == "file:" + QDir::homeDirPath()
    )
    {
      // Don't even show ourselves if it's the home dir
      return;
    }

    QWidget * widget = dialog->addPage(i18n("&Sharing"));

    d->stack = new QWidgetStack(widget);

    QVBoxLayout * stackLayout = new QVBoxLayout(widget);
    stackLayout->addWidget(d->stack);

    d->initWidget   = createInitWidget(d->stack);
    d->configWidget = createConfigWidget(d->stack);

    d->stack->addWidget(d->initWidget,    0);
    d->stack->addWidget(d->configWidget,  1);

    kapp->dcopClient()->setNotifications(true);

    connect
      (
       kapp->dcopClient(),
       SIGNAL(applicationRegistered(const QCString &)),
       SLOT(slotApplicationRegistered(const QCString &))
      );

    connect
      (
       kapp->dcopClient(),
       SIGNAL(applicationRemoved(const QCString &)),
       SLOT(slotApplicationUnregistered(const QCString &))
      );

    d->kpfRunning = kapp->dcopClient()->isApplicationRegistered("kpf");

    if (!d->kpfRunning)
    {
      d->stack->raiseWidget(d->initWidget);
    }
    else
    {
      getServerRef();
      updateGUIFromCurrentState();
      d->stack->raiseWidget(d->configWidget);
    }
  }

  PropertiesDialogPlugin::~PropertiesDialogPlugin()
  {
    delete d->webServerManagerInterface;
    d->webServerManagerInterface = 0;

    delete d;
    d = 0;
  }

    void
  PropertiesDialogPlugin::slotSharingToggled(bool b)
  {
    if (b)
    {
      if (!userAcceptsWarning())
      {
        // Avoid loop.
        d->cb_share->blockSignals(true);
        d->cb_share->setChecked(false);
        d->cb_share->blockSignals(false);
        b = false;
      }
    }

    setControlsEnabled(b);
  }

    void
  PropertiesDialogPlugin::setControlsEnabled(bool b)
  {

    bool canPublish = b && DNSSD::ServiceBrowser::isAvailable() == DNSSD::ServiceBrowser::Working;
    
    d->l_serverName->setEnabled(canPublish);
    d->l_listenPort       ->setEnabled(b);
    d->l_bandwidthLimit   ->setEnabled(b);
//    d->l_connectionLimit  ->setEnabled(b);
    d->l_serverName       ->setEnabled(canPublish);
    
    d->sb_listenPort      ->setEnabled(b);
    d->sb_bandwidthLimit  ->setEnabled(b);
//    d->sb_connectionLimit ->setEnabled(b);
    d->le_serverName      ->setEnabled(canPublish);
    d->cb_followSymlinks  ->setEnabled(b);
  }

    QWidget *
  PropertiesDialogPlugin::createInitWidget(QWidget * parent)
  {
    QWidget * w = new QWidget(parent);

    QLabel * about =
      new QLabel
      (
       i18n
       (
        "<p>To share files via the web, you need to be"
        " running an 'applet' in your KDE panel. This"
        " 'applet' is a small program which provides"
        " file sharing capabilities."
        "</p>"
       ),
       w
      );

    d->pb_startKPF
      = new QPushButton(i18n("Start Applet"), w);

    QVBoxLayout * l = new QVBoxLayout(w);

    l->addWidget(about);

    d->l_kpfStatus =
      new QLabel(i18n("Applet status: <strong>not running</strong>"), w);

    l->addWidget(d->l_kpfStatus);

    QHBoxLayout * l2 = new QHBoxLayout(l);

    l2->addStretch(1);
    l2->addWidget(d->pb_startKPF);

    l->addStretch(1);

    connect(d->pb_startKPF, SIGNAL(clicked()), SLOT(slotStartKPF()));

    return w;
  }

    QWidget *
  PropertiesDialogPlugin::createConfigWidget(QWidget * parent)
  {
    QWidget * w = new QWidget(parent);

    d->cb_share =
      new QCheckBox(i18n("Share this directory on the &Web"), w);

    d->l_listenPort       = new QLabel(i18n("&Listen port:"),      w);
    d->l_bandwidthLimit   = new QLabel(i18n("&Bandwidth limit:"),  w);
//    d->l_connectionLimit  = new QLabel(i18n("Connection &limit"), w);
    d->l_serverName       = new QLabel(i18n("&Server name:"), w);
    bool canPublish = DNSSD::ServiceBrowser::isAvailable() == DNSSD::ServiceBrowser::Working;
    d->l_serverName->setEnabled(canPublish);

    d->sb_listenPort      = new QSpinBox(1000,  999999, 1, w);
    d->sb_bandwidthLimit  = new QSpinBox(1,     999999, 1, w);
//    d->sb_connectionLimit = new QSpinBox(1,     9999,   1, w);
    d->le_serverName      = new QLineEdit( w);
    d->le_serverName->setEnabled(canPublish);
    
    d->cb_followSymlinks  =
      new QCheckBox(i18n("&Follow symbolic links"), w);

    d->l_listenPort       ->setBuddy(d->sb_listenPort);
    d->l_serverName       ->setBuddy(d->le_serverName);
    d->l_bandwidthLimit   ->setBuddy(d->sb_bandwidthLimit);
//    d->l_connectionLimit  ->setBuddy(d->sb_connectionLimit);

    d->sb_listenPort      ->setValue(Config::DefaultListenPort);
    d->sb_bandwidthLimit  ->setValue(Config::DefaultBandwidthLimit);
    d->sb_bandwidthLimit  ->setSuffix(i18n("kB/s"));
//    d->sb_connectionLimit ->setValue(Config::DefaultConnectionLimit);
    d->cb_followSymlinks  ->setChecked(Config::DefaultFollowSymlinks);

    QVBoxLayout * l0 =
      new QVBoxLayout(w, KDialog::marginHint(), KDialog::spacingHint());

    l0->addWidget(d->cb_share);

    l0->addWidget(new KSeparator(QFrame::HLine, w));

    QGridLayout * l2 = new QGridLayout(l0);

    l2->addWidget(d->l_listenPort,       0, 0);
    l2->addWidget(d->sb_listenPort,      0, 1);
    l2->addWidget(d->l_bandwidthLimit,   1, 0);
    l2->addWidget(d->sb_bandwidthLimit,  1, 1);
//    l2->addWidget(d->l_connectionLimit,  2, 0);
//    l2->addWidget(d->sb_connectionLimit, 2, 1);
    l2->addWidget(d->l_serverName,       2, 0);
    l2->addWidget(d->le_serverName,      2, 1);

    l0->addWidget(d->cb_followSymlinks);

    l0->addStretch(1);

    QString shareHelp =
      i18n
      (
       "<p>"
       "Setting this option makes all files in this directory and"
       " any subdirectories available for reading to anyone"
       " who wishes to view them."
       "</p>"
       "<p>"
       "To view your files, a web browser or similar program"
       " may be used."
       "</p>"
       "<p>"
       "<strong>Warning!</strong> Before sharing a directory,"
       " you should be sure that it does not contain sensitive"
       " information, such as passwords, company secrets, your"
       " addressbook, etc."
       "</p>"
       "<p>"
       "Note that you cannot share your home directory"
       " (%1)"
       "</p>"
      )
      .arg(QDir::homeDirPath());

    QString listenPortHelp =
      i18n
      (
       "<p>"
       "Specify the network `port' on which the server should"
       " listen for connections."
       "</p>"
      );

    QString bandwidthLimitHelp =
      i18n
      (
       "<p>"
       "Specify the maximum amount of data (in kilobytes) that will be"
       " sent out per second."
       "</p>"
       "<p>"
       "This allows you to keep some bandwidth for yourself instead"
       " of allowing connections with kpf to hog your connection."
       "</p>"
      );

    QString connectionLimitHelp =
      i18n
      (
       "<p>"
       "Specify the maximum number of connections allowed at"
       " any one time."
       "</p>"
      );

    QString followSymlinksHelp =
      i18n
      (
       "<p>"
       "Allow serving of files which have a symbolic link in"
       " the path from / to the file, or are a symbolic link"
       " themselves."
       "</p>"
       "<p>"
       "<strong>Warning!</strong> This could be a security"
       " risk. Use only if you understand the issues involved."
       "</p>"
      );
    QString serverNameHelp = KPF::HelpText::getServerNameHelp();

    QWhatsThis::add(d->cb_share,            shareHelp);
    QWhatsThis::add(d->l_listenPort,        listenPortHelp);
    QWhatsThis::add(d->sb_listenPort,       listenPortHelp);
    QWhatsThis::add(d->l_bandwidthLimit,    bandwidthLimitHelp);
    QWhatsThis::add(d->sb_bandwidthLimit,   bandwidthLimitHelp);
//    QWhatsThis::add(d->l_connectionLimit,   connectionLimitHelp);
//    QWhatsThis::add(d->sb_connectionLimit,  connectionLimitHelp);
    QWhatsThis::add(d->l_serverName,        serverNameHelp);
    QWhatsThis::add(d->le_serverName,       serverNameHelp);
    QWhatsThis::add(d->cb_followSymlinks,   followSymlinksHelp);

    connect(d->cb_share, SIGNAL(toggled(bool)), SLOT(slotSharingToggled(bool)));

    slotSharingToggled(false);

    connect
      (
        d->cb_share,
        SIGNAL(toggled(bool)),
        SLOT(slotChanged())
      );

    connect
      (
        d->sb_listenPort,
        SIGNAL(valueChanged(int)),
        SLOT(slotChanged())
      );

    connect
      (
        d->sb_bandwidthLimit,
        SIGNAL(valueChanged(int)),
        SLOT(slotChanged())
      );

#if 0
    connect
      (
        d->sb_connectionLimit,
        SIGNAL(valueChanged(int)),
        SLOT(slotChanged())
      );
#endif
    connect
      (
        d->le_serverName,
	SIGNAL(textChanged(const QString&)),
	SLOT(slotChanged())
      );

    connect
      (
        d->cb_followSymlinks,
        SIGNAL(toggled(bool)),
        SLOT(slotChanged())
      );

    return w;
  }

    void
  PropertiesDialogPlugin::slotStartKPF()
  {
    d->l_kpfStatus
      ->setText(i18n("Applet status: <strong>starting...</strong>"));

    kapp->dcopClient()
      ->send("kicker", "default", "addApplet(QString)", "kpfapplet.desktop");

    QTimer::singleShot(4 * 1000, this, SLOT(slotStartKPFFailed()));
  }

    void
  PropertiesDialogPlugin::slotStartKPFFailed()
  {
    d->l_kpfStatus
      ->setText(i18n("Applet status: <strong>failed to start</strong>"));

    d->pb_startKPF->setEnabled(true);
  }

    void
  PropertiesDialogPlugin::slotApplicationRegistered(const QCString & s)
  {
    if ("kpf" == s)
    {
      d->kpfRunning = true;

      d->l_kpfStatus
        ->setText(i18n("Applet status: <strong>running</strong>"));

      d->pb_startKPF->setEnabled(false);

      getServerRef();
      updateGUIFromCurrentState();
      d->stack->raiseWidget(d->configWidget);
    }
  }

    void
  PropertiesDialogPlugin::slotApplicationUnregistered(const QCString & s)
  {
    if ("kpf" == s)
    {
      d->kpfRunning = false;

      d->webServerRef.clear();

      d->pb_startKPF->setEnabled(true);

      d->l_kpfStatus
        ->setText(i18n("Applet status: <strong>not running</strong>"));

      d->stack->raiseWidget(d->initWidget);
    }
  }

    void
  PropertiesDialogPlugin::readSettings()
  {
    d->currentState = ServerState();

    if (!d->kpfRunning || d->webServerRef.isNull())
      return;

    d->currentState.shared = true;

    WebServer_stub webServer(d->webServerRef.app(), d->webServerRef.object());

    d->currentState.listenPort = webServer.listenPort();

    if (DCOPStub::CallFailed == webServer.status())
    {
      // TODO: warn user ?
      kpfDebug << "WebServer_stub call failed" << endl;
      d->currentState.listenPort = Config::DefaultListenPort;
      return;
    }

    d->currentState.bandwidthLimit = webServer.bandwidthLimit();

    if (DCOPStub::CallFailed == webServer.status())
    {
      // TODO: warn user ?
      kpfDebug << "WebServer_stub call failed" << endl;
      d->currentState.bandwidthLimit = Config::DefaultBandwidthLimit;
      return;
    }

#if 0
    d->currentState.connectionLimit = webServer.connectionLimit();

    if (DCOPStub::CallFailed == webServer.status())
    {
      // TODO: warn user ?
      kpfDebug << "WebServer_stub call failed" << endl;
      d->currentState.connectionLimit = Config::DefaultConnectionLimit;
      return;
    }
#endif

    d->currentState.serverName = webServer.serverName();
    if (DCOPStub::CallFailed == webServer.status())
    {
      // TODO: warn user ?
      kpfDebug << "WebServer_stub call failed" << endl;
      d->currentState.serverName = "";
      return;
    }
  

    d->currentState.followSymlinks = webServer.followSymlinks();

    if (DCOPStub::CallFailed == webServer.status())
    {
      // TODO: warn user ?
      kpfDebug << "WebServer_stub call failed" << endl;
      d->currentState.followSymlinks = Config::DefaultFollowSymlinks;
      return;
    }
  }

    void
  PropertiesDialogPlugin::getServerRef()
  {
    QValueList<DCOPRef> serverRefList =
      d->webServerManagerInterface->serverList();

    if (DCOPStub::CallFailed == d->webServerManagerInterface->status())
    {
      // TODO: warn
      kpfDebug << "webServerManagerInterface.serverList call failed" << endl;
      return;
    }

    d->webServerRef.clear();

    QValueList<DCOPRef>::ConstIterator it(serverRefList.begin());

    for (; it != serverRefList.end(); ++it)
    {
      DCOPRef serverRef(*it);

      WebServer_stub webServer(serverRef.app(), serverRef.object());

      if (webServer.root() == d->url.path())
      {
        d->webServerRef = serverRef;
        break;
      }
    }
  }

    bool
  PropertiesDialogPlugin::userAcceptsWarning() const
  {
    QString noWarningKey("DoNotWarnAboutSharingDirectoriesViaHTTP");

    KConfig * config(KGlobal::config());

    if (config->readBoolEntry(noWarningKey, false))
      return true;

    return
      (
        KMessageBox::Continue
        ==
        KMessageBox::warningContinueCancel
        (
          d->configWidget,
          i18n(
            "<p>"
            "Before you share a directory, be <strong>absolutely"
            " certain</strong> that it does not contain sensitive"
            " information."
            "</p>"
            "<p>"
            "Sharing a directory makes all information"
            " in that directory <strong>and all subdirectories</strong>"
            " available to <strong>anyone</strong> who wishes to read it."
            "</p>"
            "<p>"
            "If you have a system administrator, please ask for permission"
            " before sharing a directory in this way."
            "</p>"
          ),
          i18n("Warning - Sharing Sensitive Information?"),
          i18n("&Share Directory"),
          noWarningKey,
          true
        )
        );
  }

    void
  PropertiesDialogPlugin::slotChanged()
  {
    kpfDebug << "PropertiesDialogPlugin::slotChanged" << endl;
    readSettings();
    updateWantedStateFromGUI();

    setDirty(d->currentState != d->wantedState);
    kpfDebug << "Dirty: " << isDirty() << endl;
    emit(changed());
  }

    void
  PropertiesDialogPlugin::applyChanges()
  {
    readSettings();
    updateWantedStateFromGUI();

    enum Action
    {
      None,
      Enable,
      Disable,
      Reconfigure
    };

    bool needRestart = false;

    Action action = None;

    if (!d->currentState.shared && d->wantedState.shared)
    {
//    kpfDebug << "Not shared, but want to be. Action is Enable" << endl;
      action = Enable;
    }
    else if (d->currentState.shared && !d->wantedState.shared)
    {
//    kpfDebug << "Shared, but don't want to be. Action is Disable" << endl;
      action = Disable;
    }
    else if
      (
       d->currentState.listenPort       != d->wantedState.listenPort
       ||
       d->currentState.bandwidthLimit   != d->wantedState.bandwidthLimit
       ||
//       d->currentState.connectionLimit  != d->wantedState.connectionLimit
//       ||
       d->currentState.serverName       != d->wantedState.serverName
       ||
       d->currentState.followSymlinks   != d->wantedState.followSymlinks
      )
    {
//    kpfDebug << "Config changed. Action is Reconfigure" << endl;
      action = Reconfigure;

      if (d->currentState.listenPort != d->wantedState.listenPort)
        needRestart = true;
    }

    if (None == action)
    {
//    kpfDebug << "Nothing changed. Action = None" << endl;
      return;
    }

    switch (action)
    {
      case Enable:
        {
          DCOPRef ref =
            d->webServerManagerInterface->createServer
            (
             d->url.path(),
             d->wantedState.listenPort,
             d->wantedState.bandwidthLimit,
             Config::DefaultConnectionLimit,//d->wantedState.connectionLimit,
             d->wantedState.followSymlinks,
	     d->wantedState.serverName
            );

          if (ref.isNull())
          {
            // TODO: Warn user.
            kpfDebug
              << "kpf refused to create server - warn user here !" << endl;
            break;
          }
          else
          {
            d->webServerRef = ref;
          }
        }
        break;

      case Disable:
        if (d->webServerRef.isNull())
        {
          // TODO: Warn user.
          kpfDebug << "Disable, but d->webServerRef is null" << endl;
        }
        else
        {
          d->webServerManagerInterface->disableServer(d->webServerRef);
        }
        break;

      case Reconfigure:

        if (d->webServerRef.isNull())
        {
          kpfDebug << "Need restart, but d->webServerRef is null" << endl;
        }
        else
        {
          WebServer_stub webServer
            (d->webServerRef.app(), d->webServerRef.object());

          webServer.set
            (
             d->wantedState.listenPort,
             d->wantedState.bandwidthLimit,
             Config::DefaultConnectionLimit,//d->wantedState.connectionLimit,
             d->wantedState.followSymlinks,
	     d->wantedState.serverName
            );

          if (DCOPStub::CallFailed == webServer.status())
          {
            // TODO: Warn user.
            kpfDebug << "Reconfigure failed" << endl;
          }

          if (needRestart)
          {
            webServer.restart();

            if (DCOPStub::CallFailed == webServer.status())
            {
              // TODO: Warn user.
              kpfDebug << "Restart failed" << endl;
            }
          }
        }
        break;

      default:
        kpfDebug << "Code error in KPF::PropertiesDialogPlugin." << endl;
        break;
    }
  }

    void
  PropertiesDialogPlugin::updateGUIFromCurrentState()
  {
    readSettings();

    // We don't want slotSharingToggled to be called.
    d->cb_share->blockSignals(true);
    d->cb_share->setChecked(d->currentState.shared);
    d->cb_share->blockSignals(false);

    d->sb_listenPort      ->setValue    (d->currentState.listenPort);
    d->sb_bandwidthLimit  ->setValue    (d->currentState.bandwidthLimit);
//    d->sb_connectionLimit ->setValue    (d->currentState.connectionLimit);
    d->le_serverName      ->setText     (d->currentState.serverName);
    d->cb_followSymlinks  ->setChecked  (d->currentState.followSymlinks);

    setControlsEnabled(d->currentState.shared);
  }

    void
  PropertiesDialogPlugin::updateWantedStateFromGUI()
  {
    d->wantedState.shared           = d->cb_share->isChecked();
    d->wantedState.listenPort       = d->sb_listenPort->value();
    d->wantedState.bandwidthLimit   = d->sb_bandwidthLimit->value();
//    d->wantedState.connectionLimit  = d->sb_connectionLimit->value();
    d->wantedState.serverName       = d->le_serverName->text();
    d->wantedState.followSymlinks   = d->cb_followSymlinks->isChecked();
  }

  typedef KGenericFactory<PropertiesDialogPlugin, KPropertiesDialog> PropertiesDialogPluginFactory;
}

K_EXPORT_COMPONENT_FACTORY( kpfpropertiesdialog, 
                            KPF::PropertiesDialogPluginFactory( "kpf" ) )

#include "PropertiesDialogPlugin.moc"
// vim:ts=2:sw=2:tw=78:et
