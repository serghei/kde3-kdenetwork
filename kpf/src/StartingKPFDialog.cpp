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
#include <qlabel.h>
#include <qframe.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <klocale.h>

#include "Defines.h"
#include "StartingKPFDialog.h"

namespace KPF
{
  class StartingKPFDialog::Private
  {
    public:

      Private()
      {
        // Empty.
      }

      QTimer timer;
  };

  StartingKPFDialog::StartingKPFDialog(QWidget * parent)
    :
    KDialogBase
    (
     parent,
     "StartingKPFDialog",
     true, /* modal */
     i18n("Starting KDE public fileserver applet"),
     KDialogBase::Ok | KDialogBase::Cancel,
     KDialogBase::Cancel,
     true
    )
  {
    d = new Private;

    QFrame * mainWidget = makeMainWidget();

    QLabel * about =
      new QLabel
      (
       i18n("Starting kpf..."),
       mainWidget
      );

    QVBoxLayout * layout = new QVBoxLayout(mainWidget);

    layout->addWidget(about);

    kapp->dcopClient()->setNotifications(true);

    connect
      (
       kapp->dcopClient(),
       SIGNAL(applicationRegistered(const QCString &)),
       SLOT(slotApplicationRegistered(const QCString &))
      );

    kapp->dcopClient()
      ->send("kicker", "default", "addApplet(QString)", "kpfapplet.desktop");

    connect(&d->timer, SIGNAL(timeout()), SLOT(slotTimeout()));

    enableButtonOK(false);
    enableButtonCancel(true);

    d->timer.start(8 * 1000 /* 8 seconds */, true /* single shot */);
  }

  StartingKPFDialog::~StartingKPFDialog()
  {
    delete d;
    d = 0;
  }

    void
  StartingKPFDialog::slotTimeout()
  {
    enableButtonOK(true);
    enableButtonCancel(false);

    if (kpfRunning())
    {
      kpfDebug << "slotTimeout: kpf is running now" << endl;
    }
    else
    {
      kpfDebug << "slotTimeout: kpf still not running" << endl;
    }
  }

    bool
  StartingKPFDialog::kpfRunning()
  {
    return kapp->dcopClient()->isApplicationRegistered("kpf");
  }

    void
  StartingKPFDialog::slotApplicationRegistered(const QCString & id)
  {
    if ("kpf" == id)
    {
      kpfDebug << "kpf just started up" << endl;
      enableButtonOK(true);
      enableButtonCancel(false);
    }
  }
}

#include "StartingKPFDialog.moc"
// vim:ts=2:sw=2:tw=78:et
