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

#include <qdir.h>
#include <qfileinfo.h>

#include "DirSelectWidget.h"
#include "DirSelectWidget.moc"

namespace KPF
{
  class DirSelectWidget::Private
  {
    public:

      QString pathToMakeVisible;
  };

  DirSelectWidget::DirSelectWidget
    (
      const QString & pathToMakeVisible,
      QWidget       * parent,
      const char    * name
    )
    : KListView(parent, name)
  {
    d = new Private;
    d->pathToMakeVisible = pathToMakeVisible;

    setRootIsDecorated(true);

    connect
      (
        this,
        SIGNAL(expanded(QListViewItem *)),
        SLOT(slotExpanded(QListViewItem *))
      );

    QListViewItem * root = new QListViewItem(this, "/");

    root->setExpandable(true);

    startTimer(0);
  }

  DirSelectWidget::~DirSelectWidget()
  {
    delete d;
  }

    void
  DirSelectWidget::timerEvent(QTimerEvent *)
  {
    killTimers();

    if (0 != firstChild())
      firstChild()->setOpen(true);
  }

    void
  DirSelectWidget::slotExpanded(QListViewItem * item)
  {
    if (0 != item->firstChild())
      return;

    QString p(path(item));

    QDir dir(p);

    const QFileInfoList * entryInfoList =
      dir.entryInfoList(QDir::Dirs | QDir::Readable);

    for (QFileInfoListIterator it(*entryInfoList); it.current(); ++it)
    {
      if (it.current()->isDir() && it.current()->isReadable())
      {
        QListViewItem * i = new QListViewItem(item, it.current()->fileName());
        i->setExpandable(true);
      }
    }
  }

    QString
  DirSelectWidget::path(QListViewItem * item) const
  {
    QString ret(item->text(0));

    while (0 != (item = item->parent()))
      ret.prepend("/" + item->text(0));

    return ret;
  }
}

// vim:ts=2:sw=2:tw=78:et
