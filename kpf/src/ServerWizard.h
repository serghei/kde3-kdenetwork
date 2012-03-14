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

#ifndef KPF_SERVER_WIZARD_H
#define KPF_SERVER_WIZARD_H

#include <kwizard.h>

class QSpinBox;
class KURLRequester;
class QLineEdit;

namespace KPF
{
  /**
   * Used to allow easy creation and configuration of a WebServer object.
   */
  class ServerWizard : public KWizard
  {
    Q_OBJECT

    public:

      ServerWizard(QWidget * parent = 0);

      virtual ~ServerWizard();

      void          setLocation(const QString &);

      QString root()            const;
      uint    listenPort()      const;
      uint    bandwidthLimit()  const;
      uint    connectionLimit() const;
      QString serverName()      const;

    signals:

      void dying(ServerWizard *);

    protected slots:

      virtual void accept();
      virtual void reject();
      void slotServerRootChanged(const QString &);
      void slotListenPortChanged(int);
      void slotOpenFileDialog(KURLRequester *);

    protected:

      virtual void help();

    private:

        
      KURLRequester * kur_root_;
      QSpinBox      * sb_listenPort_;
      QSpinBox      * sb_bandwidthLimit_;
      QSpinBox      * sb_connectionLimit_;
      QLineEdit     * le_serverName_;

      QWidget       * page1_;
      QWidget       * page2_;
      QWidget       * page3_;
      QWidget       * page4_;
      QWidget       * page5_;
  };
}

#endif // KPF_SERVER_WIZARD_H
// vim:ts=2:sw=2:tw=78:et
