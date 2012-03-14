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

#ifndef KPF_ERROR_MESSAGE_CONFIG_DIALOG_H
#define KPF_ERROR_MESSAGE_CONFIG_DIALOG_H

#include <qmap.h>
#include <kdialogbase.h>

class KURLRequester;

namespace KPF
{
  class WebServer;

  /**
   * Currently unused pending implementation.
   */
  class ErrorMessageConfigDialog : public KDialogBase
  {
    Q_OBJECT

    public:

      ErrorMessageConfigDialog(WebServer *, QWidget * parent);

      virtual ~ErrorMessageConfigDialog();

    protected slots:

      void slotURLRequesterTextChanged(const QString &);

    protected:

      void accept();

    private:

      WebServer * server_;

      /**
       * Provides a graphical interface to allow the user to pick a file
       * to be used when reporting an error code.
       */
      class Item
      {
        public:

          Item(uint i, KURLRequester * r, QString s, QString p)
            : code          (i),
              urlRequester  (r),
              report        (s),
              originalPath  (p)
          {
          }

          uint            code;
          KURLRequester * urlRequester;
          QString         report;
          QString         originalPath;
      };

      QPtrList<Item> itemList_;
  };
}

#endif
// vim:ts=2:sw=2:tw=78:et
