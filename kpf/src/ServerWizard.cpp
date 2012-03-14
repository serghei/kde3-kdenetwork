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

#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qptrlist.h>
#include <qlineedit.h>

#include <kapplication.h>
#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "Defines.h"
#include "ServerWizard.h"
#include "WebServerManager.h"
#include "WebServer.h"
#include "Help.h"
#include <dnssd/servicebrowser.h>

#include <unistd.h>

namespace KPF
{
  ServerWizard::ServerWizard(QWidget * parent)
    : KWizard(parent, "KPF::ServerWizard", true)
  {
    setCaption(i18n("New Server - %1").arg("kpf"));

    page1_ = new QWidget(this);
    page2_ = new QWidget(this);
    page3_ = new QWidget(this);
//    page4_ = new QWidget(this);
    page5_ = new QWidget(this);
    
    QLabel * l_rootDirectoryHelp =
      new QLabel
      (
       i18n
       (
        "<p>"
        "Specify the directory which contains the files"
        " you wish to share."
        "</p>"
        "<p>"
        "<em>Warning</em>: Do not share any directories that contain"
        " sensitive information!"
        "</p>"
       ),
       page1_
      );

    QLabel * l_listenPortHelp =
      new QLabel
      (
       i18n
       (
        "<p>"
        "Specify the network `port' on which the server should"
        " listen for connections."
        "</p>"
       ),
       page2_
      );

    QLabel * l_bandwidthLimitHelp =
      new QLabel
      (
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
       ),
       page3_
      );
/*
    QLabel * l_connectionLimitHelp =
      new QLabel
      (
       i18n
       (
        "<p>"
        "Specify the maximum number of connections allowed at"
        " any one time."
        "</p>"
       ),
       page4_
      );
*/
    bool canPublish = DNSSD::ServiceBrowser::isAvailable() == DNSSD::ServiceBrowser::Working;
    QLabel * l_serverNameHelp =
      new QLabel
      (
       KPF::HelpText::getServerNameHelp(),
       page5_
      );

    QLabel * l_root_ =
      new QLabel(i18n("&Root directory:"), page1_);

    QLabel * l_listenPort_ =
      new QLabel(i18n("&Listen port:"), page2_);

    QLabel * l_bandwidthLimit_ =
      new QLabel(i18n("&Bandwidth limit:"), page3_);

//    QLabel * l_connectionLimit_ =
//      new QLabel(i18n("Connection &limit"), page4_);

    QLabel * l_serverName_ = 
      new QLabel(i18n("&Server name:"), page5_);

    if(!canPublish)
      l_serverName_->setEnabled(false);

    kur_root_           = new KURLRequester(page1_);

    sb_listenPort_      = new QSpinBox(1,  65535, 1, page2_);

    sb_bandwidthLimit_  = new QSpinBox(1,     999999, 1, page3_);
//    sb_connectionLimit_ = new QSpinBox(1,     9999,   1, page4_);

    char hostname[255];
    gethostname(hostname, 255-2);
    hostname[255-1]=0;
    le_serverName_       = new QLineEdit(hostname, page5_);

    if(!canPublish)
      le_serverName_->setEnabled(false);
    
    l_root_             ->setBuddy(kur_root_);
    l_listenPort_       ->setBuddy(sb_listenPort_);
    l_bandwidthLimit_   ->setBuddy(sb_bandwidthLimit_);
    l_serverName_       ->setBuddy(le_serverName_);
//    l_connectionLimit_  ->setBuddy(sb_connectionLimit_);

    sb_listenPort_
      ->setValue(WebServerManager::instance()->nextFreePort());

    sb_bandwidthLimit_  ->setValue(Config::DefaultBandwidthLimit);
    sb_bandwidthLimit_  ->setSuffix(i18n(" kB/s"));
//    sb_connectionLimit_ ->setValue(Config::DefaultConnectionLimit);

    QVBoxLayout * layout1 =
      new QVBoxLayout(page1_, KDialog::marginHint(), KDialog::spacingHint());

    QVBoxLayout * layout2 =
      new QVBoxLayout(page2_, KDialog::marginHint(), KDialog::spacingHint());

    QVBoxLayout * layout3 =
      new QVBoxLayout(page3_, KDialog::marginHint(), KDialog::spacingHint());

//    QVBoxLayout * layout4 =
//      new QVBoxLayout(page4_, KDialog::marginHint(), KDialog::spacingHint());

    QVBoxLayout * layout5 = 
      new QVBoxLayout(page5_, KDialog::marginHint(), KDialog::spacingHint());

    layout1->addWidget(l_rootDirectoryHelp);
    layout2->addWidget(l_listenPortHelp);
    layout3->addWidget(l_bandwidthLimitHelp);
//    layout4->addWidget(l_connectionLimitHelp);
    layout5->addWidget(l_serverNameHelp);
    
    QHBoxLayout * layout10 = new QHBoxLayout(layout1);

    layout10->addWidget(l_root_);
    layout10->addWidget(kur_root_);

    layout1->addStretch(1);

    QHBoxLayout * layout20 = new QHBoxLayout(layout2);

    layout20->addWidget(l_listenPort_);
    layout20->addWidget(sb_listenPort_);

    layout2->addStretch(1);

    QHBoxLayout * layout30 = new QHBoxLayout(layout3);

    layout30->addWidget(l_bandwidthLimit_);
    layout30->addWidget(sb_bandwidthLimit_);

    layout3->addStretch(1);

//    QHBoxLayout * layout40 = new QHBoxLayout(layout4);

//    layout40->addWidget(l_connectionLimit_);
//    layout40->addWidget(sb_connectionLimit_);

//    layout4->addStretch(1);

    QHBoxLayout * layout50 = new QHBoxLayout(layout5);
    
    layout50->addWidget(l_serverName_);
    layout50->addWidget(le_serverName_);

    addPage(page1_, i18n("Root Directory"));
    addPage(page2_, i18n("Listen Port"));
    addPage(page3_, i18n("Bandwidth Limit"));
//    addPage(page4_, i18n("Connection Limit"));
    addPage(page5_, i18n("Server Name"));

    kur_root_->setURL(QDir::homeDirPath() + "/public_html");
    kur_root_->setMode
      (KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

//    setFinishEnabled(page4_, true);
    setFinishEnabled(page5_, true);

    // This slot sets the 'Next' button on page 1 to enabled/disabled
    // depending on whether the path is OK.

    connect
      (
       kur_root_,
       SIGNAL(textChanged(const QString &)),
       SLOT(slotServerRootChanged(const QString &))
      );

    // Used for setting the caption on the file dialog.

    connect
      (
       kur_root_,
       SIGNAL(openFileDialog(KURLRequester *)),
       SLOT(slotOpenFileDialog(KURLRequester *))
      );

    // This slot sets the 'Next' button on page 2 to enabled/disabled
    // depending on whether the port is OK.

    connect
      (
       sb_listenPort_,
       SIGNAL(valueChanged(int)),
       SLOT(slotListenPortChanged(int))
      );

    // Update 'Next' button on page 1.

    slotServerRootChanged(kur_root_->url());

    // Update 'Next' button on page 2.

    slotListenPortChanged(sb_listenPort_->value());
  }

  ServerWizard::~ServerWizard()
  {
  }
  
    void
  ServerWizard::setLocation(const QString & location)
  {
    kur_root_->setURL(location);
  }

    QString
  ServerWizard::root() const
  {
    return kur_root_->url();
  }

    uint
  ServerWizard::listenPort() const
  {
    return sb_listenPort_->value();
  }

    uint
  ServerWizard::bandwidthLimit() const
  {
    return sb_bandwidthLimit_->value();
  }
    QString
    
  ServerWizard::serverName() const
  {
    return le_serverName_->text();
  }

    uint
  ServerWizard::connectionLimit() const
  {
    return Config::DefaultConnectionLimit; // sb_connectionLimit_->value();
  }

    void
  ServerWizard::accept()
  {
    QWizard::accept();
    emit(dying(this));
  }

    void
  ServerWizard::reject()
  {
    QWizard::reject();
    emit(dying(this));
  }

    void
  ServerWizard::slotListenPortChanged(int newPort)
  {
    if (newPort <= 1024)
    {
      setNextEnabled(page2_, false);
      return;
    }

    QPtrList<WebServer>
      serverList(WebServerManager::instance()->serverListLocal());

    for (QPtrListIterator<WebServer> it(serverList); it.current(); ++it)
    {
      if (it.current()->listenPort() == uint(newPort))
      {
        setNextEnabled(page2_, false);
        return;
      }
    }

    setNextEnabled(page2_, true);
  }

    void
  ServerWizard::slotServerRootChanged(const QString & _root)
  {
    QString root(_root);

    kpfDebug << root << endl;

    // Duplicate ?

    if (WebServerManager::instance()->hasServer(root))
    {
      kpfDebug << "Already have a server at " << root << endl;
      setNextEnabled(page1_, false);
      return;
    }

    if ("/" != root.right(1))
      root += "/";

    QFileInfo fi(root);

    if (!fi.isDir()) // || (fi.dirPath() == QDir::homeDirPath()))
    {
      kpfDebug << root << " isn't a dir" << endl; //, or it's $HOME" << endl;
      setNextEnabled(page1_, false);
      return;
    }

    setNextEnabled(page1_, true);
  }

    void
  ServerWizard::slotOpenFileDialog(KURLRequester * urlRequester)
  {
    KFileDialog * fileDialog = urlRequester->fileDialog();

    if (0 == fileDialog)
    {
      kpfDebug << "URL requester's file dialog is 0" << endl;
      return;
    }

    fileDialog->setCaption(i18n("Choose Directory to Share - %1").arg("kpf"));
  }

    void
  ServerWizard::help()
  {
    kapp->invokeHelp("share-config", "kpf");
  }
}

#include "ServerWizard.moc"
// vim:ts=2:sw=2:tw=78:et
