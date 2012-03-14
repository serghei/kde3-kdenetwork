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

#include <klocale.h>

#include "Defines.h"
#include "WebServer.h"
#include "ConfigDialogPage.h"
#include "SingleServerConfigDialog.h"

namespace KPF
{
  SingleServerConfigDialog::SingleServerConfigDialog
    (
     WebServer  * server,
     QWidget    * parent
    )
    : KDialogBase
      (
       parent,
       "KPF::SingleServerConfigDialog",
       false,
       i18n("Configuring Server %1 - kpf").arg(server->root()),
       KDialogBase::Ok | KDialogBase::Cancel,
       KDialogBase::Ok,
       true
      ),
      server_(server)
  {
    widget_ = new ConfigDialogPage(server_, this);

    connect
      (
        widget_,
        SIGNAL(ok(bool)),
        SLOT(slotOk(bool))
      );

    setMainWidget(widget_);

    connect(this, SIGNAL(finished()), SLOT(slotFinished()));

    widget_->checkOk();
  }

  SingleServerConfigDialog::~SingleServerConfigDialog()
  {
    // Empty.
  }

    WebServer *
  SingleServerConfigDialog::server()
  {
    return server_;
  }

    void
  SingleServerConfigDialog::accept()
  {
    widget_->save();
    KDialogBase::accept();
  }

    void
  SingleServerConfigDialog::slotFinished()
  {
    emit(dying(this));
  }

    void
  SingleServerConfigDialog::slotOk(bool ok)
  {
    enableButtonOK(ok);
  }
}

#include "SingleServerConfigDialog.moc"
// vim:ts=2:sw=2:tw=78:et
