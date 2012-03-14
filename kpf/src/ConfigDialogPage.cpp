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

#include <qwhatsthis.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kseparator.h>
#include <kfiledialog.h>

#include "Defines.h"
#include "ErrorMessageConfigDialog.h"
#include "ConfigDialogPage.h"
#include "WebServerManager.h"
#include "WebServer.h"
#include "Help.h"

#include <dnssd/servicebrowser.h>

namespace KPF
{
  ConfigDialogPage::ConfigDialogPage(WebServer * server, QWidget * parent)
    : QWidget                   (parent, "KPF::ConfigDialogPage"),
      server_                   (server),
      errorMessageConfigDialog_ (0L)
  {
    l_listenPort_       = new QLabel(i18n("&Listen port:"),      this);
    l_bandwidthLimit_   = new QLabel(i18n("&Bandwidth limit:"),  this);
//    l_connectionLimit_  = new QLabel(i18n("Connection &limit"), this);

    sb_listenPort_      = new QSpinBox(1,  65535, 1, this);
    sb_bandwidthLimit_  = new QSpinBox(1,     999999, 1, this);
//    sb_connectionLimit_ = new QSpinBox(1,     9999,   1, this);

    l_serverName_       = new QLabel(i18n("&Server name:"),      this);
    le_serverName_      = new QLineEdit(this);

    bool canPublish = DNSSD::ServiceBrowser::isAvailable() == DNSSD::ServiceBrowser::Working;
    l_serverName_->setEnabled(canPublish);
    le_serverName_->setEnabled(canPublish);
    
    cb_followSymlinks_  = new QCheckBox(i18n("&Follow symbolic links"), this);

//    cb_customErrorMessages_ =
//      new QCheckBox(i18n("Use custom error messages"), this);

//    pb_errorMessages_   = new QPushButton(i18n("&Configure..."), this);

//    pb_errorMessages_->setEnabled(false);

    l_listenPort_       ->setBuddy(sb_listenPort_);
    l_bandwidthLimit_   ->setBuddy(sb_bandwidthLimit_);
    l_serverName_       ->setBuddy(le_serverName_);
//    l_connectionLimit_  ->setBuddy(sb_connectionLimit_);

    sb_listenPort_
      ->setValue(WebServerManager::instance()->nextFreePort());

    sb_bandwidthLimit_  ->setValue(Config::DefaultBandwidthLimit);
    sb_bandwidthLimit_  ->setSuffix(i18n(" kB/s"));
//    sb_connectionLimit_ ->setValue(Config::DefaultConnectionLimit);
    cb_followSymlinks_  ->setChecked(Config::DefaultFollowSymlinks);

    QVBoxLayout * l0 = new QVBoxLayout(this, 0, KDialog::spacingHint());

    QGridLayout * l2 = new QGridLayout(l0);

    l2->addWidget(l_listenPort_,       0, 0);
    l2->addWidget(sb_listenPort_,      0, 1);
    l2->addWidget(l_bandwidthLimit_,   1, 0);
    l2->addWidget(sb_bandwidthLimit_,  1, 1);
    l2->addWidget(l_serverName_,       2, 0);
    l2->addWidget(le_serverName_,      2, 1);
//    l2->addWidget(l_connectionLimit_,  2, 0);
//    l2->addWidget(sb_connectionLimit_, 2, 1);

    l0->addWidget(cb_followSymlinks_);

#if 0
    QHBoxLayout * l3 = new QHBoxLayout(l0);

    l3->addWidget(cb_customErrorMessages_);
    l3->addWidget(pb_errorMessages_);
#endif

    l0->addStretch(1);

#if 0
    connect
      (
       cb_customErrorMessages_,
       SIGNAL(toggled(bool)),
       SLOT(slotCustomErrorMessagesToggled(bool))
      );
#endif

#if 0
    connect
      (
       pb_errorMessages_,
       SIGNAL(clicked()),
       SLOT(slotConfigureErrorMessages())
      );
#endif

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
       "<strong>Warning !</strong> This could be a security"
       " risk. Use only if you understand the issues involved."
       "</p>"
      );

    QString errorMessagesHelp =
      i18n
      (
       "<p>"
       "Specify the text that will be sent upon an error,"
       " such as a request for a page that does not exist"
       " on this server."
       "</p>"
      );

    QString serverNameHelp = KPF::HelpText::getServerNameHelp();
    QWhatsThis::add(l_listenPort_,        listenPortHelp);
    QWhatsThis::add(sb_listenPort_,       listenPortHelp);
    QWhatsThis::add(l_bandwidthLimit_,    bandwidthLimitHelp);
    QWhatsThis::add(sb_bandwidthLimit_,   bandwidthLimitHelp);
//    QWhatsThis::add(l_connectionLimit_,   connectionLimitHelp);
//    QWhatsThis::add(sb_connectionLimit_,  connectionLimitHelp);
    QWhatsThis::add(cb_followSymlinks_,   followSymlinksHelp);
    QWhatsThis::add(l_serverName_,        serverNameHelp);
    QWhatsThis::add(le_serverName_,       serverNameHelp);
//    QWhatsThis::add(pb_errorMessages_,    errorMessagesHelp);

    connect
      (
        sb_listenPort_,
        SIGNAL(valueChanged(int)),
        SLOT(slotListenPortChanged(int))
      );

    connect
      (
        sb_bandwidthLimit_,
        SIGNAL(valueChanged(int)),
        SLOT(slotBandwidthLimitChanged(int))
      );

    connect
      (
        cb_followSymlinks_,
        SIGNAL(toggled(bool)),
        SLOT(slotFollowSymlinksToggled(bool))
      );


    load();
  }

  ConfigDialogPage::~ConfigDialogPage()
  {
    // Empty.
  }

    void
  ConfigDialogPage::load()
  {
    sb_listenPort_          ->setValue(server_->listenPort());
    sb_bandwidthLimit_      ->setValue(server_->bandwidthLimit());
//    sb_connectionLimit_     ->setValue(server_->connectionLimit());
    cb_followSymlinks_      ->setChecked(server_->followSymlinks());
    le_serverName_          ->setText(server_->serverName());
//    cb_customErrorMessages_ ->setChecked(server_->customErrorMessages());
  }

    void
  ConfigDialogPage::save()
  {
    server_->setListenPort          (sb_listenPort_->value());
    server_->setBandwidthLimit      (sb_bandwidthLimit_->value());
//    server_->setConnectionLimit     (sb_connectionLimit_->value());
    server_->setFollowSymlinks      (cb_followSymlinks_->isChecked());
    server_->setCustomErrorMessages (cb_followSymlinks_->isChecked());
    server_->setServerName          (le_serverName_->text());
  }

    void
  ConfigDialogPage::slotCustomErrorMessagesToggled(bool)
  {
//    pb_errorMessages_->setEnabled(b);
  }

    void
  ConfigDialogPage::slotConfigureErrorMessages()
  {
    if (0 == errorMessageConfigDialog_)
      errorMessageConfigDialog_ = new ErrorMessageConfigDialog(server_, this);

    errorMessageConfigDialog_->show();
  }

    void
  ConfigDialogPage::slotListenPortChanged(int)
  {
    kpfDebug << "slotBandwidthLimitChanged" << endl;
    checkOkAndEmit();
  }

  void ConfigDialogPage::checkOk()
  {
    kpfDebug << "slotBandwidthLimitChanged" << endl;
    checkOkAndEmit();
  }

  void ConfigDialogPage::slotBandwidthLimitChanged(int)
  {
    kpfDebug << "slotBandwidthLimitChanged" << endl;
    checkOkAndEmit();
  }

  void ConfigDialogPage::slotFollowSymlinksToggled(bool)
  {
    kpfDebug << "slotBandwidthLimitChanged" << endl;
    checkOkAndEmit();
  }

  void ConfigDialogPage::checkOkAndEmit()
  {
    int newPort = sb_listenPort_->value();

    if (newPort <= 1024)
    {
      emit(ok(false));
      return;
    }

    QPtrList<WebServer>
      serverList(WebServerManager::instance()->serverListLocal());

    for (QPtrListIterator<WebServer> it(serverList); it.current(); ++it)
    {
      if (it.current() == server_)
        continue;

      if (it.current()->listenPort() == uint(newPort))
      {
        emit(ok(false));
        return;
      }
    }

    emit(ok(true));
  }
}
#include "ConfigDialogPage.moc"
// vim:ts=2:sw=2:tw=78:et
