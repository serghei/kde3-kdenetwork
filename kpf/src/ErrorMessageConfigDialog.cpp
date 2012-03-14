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

#include "ErrorMessageConfigDialog.h"
#include "ErrorMessageConfigDialog.moc"

#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>

#include <kurlrequester.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdialog.h>

#include "Defines.h"
#include "Defaults.h"
#include "Utils.h"

namespace KPF
{
  ErrorMessageConfigDialog::ErrorMessageConfigDialog
  (
   WebServer  * webServer,
   QWidget    * parent
  )
    : KDialogBase
    (
     parent,
     "ErrorMessageConfigDialog",
     false,
     i18n("Configure error messages"),
     KDialogBase::Ok | KDialogBase::Cancel,
     KDialogBase::Cancel,
     true // Use a separator.
    ),
    server_(webServer)
  {
    QValueList<uint> codeList;

    codeList << 400 << 403 << 404 << 412 << 416 << 500 << 501;

    QFrame * w = makeMainWidget();

    QVBoxLayout * layout =
      new QVBoxLayout(w, KDialog::marginHint(), KDialog::spacingHint());

    QLabel * info =
      new QLabel
      (
       i18n
       (
        "<p>Here you may select files to use instead of the default error"
        " messages passed to a client.</p>"
        "<p>The files may contain anything you wish, but by convention"
        " you should report the error code and the English version of"
        " the error message (e.g. \"Bad request\"). Your file should"
        " also be valid HTML.</p>"
        "<p>The strings ERROR_MESSAGE, ERROR_CODE and RESOURCE, if"
        " they exist in the file, will be replaced with the English"
        " error message, the numeric error code and the path of the"
        " requested resource, respectively.</p>"
       ),
       w
      );

    layout->addWidget(info);

    QGridLayout * grid = new QGridLayout(layout, codeList.count(), 2);

    QString pattern(i18n("%1 %2"));

    KConfig config(Config::name());

    config.setGroup("ErrorMessageOverrideFiles");

    QValueList<uint>::ConstIterator it;

    for (it = codeList.begin(); it != codeList.end(); ++it)
    {
      QString originalPath =
        config.readPathEntry(QString::number(*it));

      QString responseName(translatedResponseName(*it));

      KURLRequester * requester = new KURLRequester(originalPath, w);

      itemList_.append(new Item(*it, requester, responseName, originalPath));

      QLabel * l = new QLabel(pattern.arg(*it).arg(responseName), w);

      l->setBuddy(requester);

      grid->addWidget(l,          *it, 0);
      grid->addWidget(requester,  *it, 1);
    }
  }

  ErrorMessageConfigDialog::~ErrorMessageConfigDialog()
  {
    itemList_.setAutoDelete(true);
    itemList_.clear();
  }

    void
  ErrorMessageConfigDialog::slotURLRequesterTextChanged(const QString &)
  {
  }

    void
  ErrorMessageConfigDialog::accept()
  {
    KConfig config(Config::name());

    config.setGroup("ErrorMessageOverrideFiles");

    QPtrListIterator<Item> it(itemList_);

    for (; it.current(); ++it)
    {
      config.writePathEntry
        (
         QString::number(it.current()->code),
         it.current()->urlRequester->url()
        );
    }

    config.sync();

    KDialogBase::accept();
  }
}

// vim:ts=2:sw=2:tw=78:et
