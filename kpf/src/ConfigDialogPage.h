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

#ifndef KPF_CONFIG_DIALOG_PAGE_H
#define KPF_CONFIG_DIALOG_PAGE_H

#include <qptrlist.h>
#include <qwidget.h>

class QLabel;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QLineEdit;

namespace KPF
{
  class WebServer;
  class ErrorMessageConfigDialog;

  /**
   * Allows user configuration of a WebServer object.
   */
  class ConfigDialogPage : public QWidget
  {
    Q_OBJECT

    public:

      ConfigDialogPage(WebServer *, QWidget * parent);

      virtual ~ConfigDialogPage();

      /**
       * Read settings from associated WebServer object and update controls.
       */
      void load();

      /**
       * Set attributes of associated WebServer object from controls.
       */
      void save();

      void checkOk();

    protected slots:

      void slotConfigureErrorMessages();
      void slotCustomErrorMessagesToggled(bool);
      void slotListenPortChanged(int);
      void slotBandwidthLimitChanged(int);
      void slotFollowSymlinksToggled(bool);

    protected:

      void checkOkAndEmit();

    signals:

      void ok(bool);

    private:

      WebServer * server_;

      QLabel    * l_listenPort_;
      QLabel    * l_bandwidthLimit_;
      QLabel    * l_connectionLimit_;
      QLabel	* l_serverName_;

      QSpinBox  * sb_listenPort_;
      QSpinBox  * sb_bandwidthLimit_;
      QSpinBox  * sb_connectionLimit_;

      QCheckBox * cb_followSymlinks_;
      
      QLineEdit * le_serverName_;

      QCheckBox     * cb_customErrorMessages_;
      QPushButton   * pb_errorMessages_;

      ErrorMessageConfigDialog * errorMessageConfigDialog_;
  };
}

#endif
// vim:ts=2:sw=2:tw=78:et
