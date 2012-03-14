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

#ifndef KPF_SINGLE_SERVER_CONFIG_DIALOG_H
#define KPF_SINGLE_SERVER_CONFIG_DIALOG_H

#include <kdialogbase.h>

namespace KPF
{
  class WebServer;
  class ConfigDialogPage;

  /**
   * Used to allow configuration of a WebServer object.
   */
  class SingleServerConfigDialog : public KDialogBase
  {
    Q_OBJECT

    public:

      SingleServerConfigDialog(WebServer *, QWidget * parent);

      virtual ~SingleServerConfigDialog();

      WebServer * server();

    protected slots:

      void slotFinished();
      void accept();
      void slotOk(bool);

    signals:

      void dying(SingleServerConfigDialog *);

    private:

      WebServer * server_;

      ConfigDialogPage * widget_;
  };
}

#endif
// vim:ts=2:sw=2:tw=78:et
