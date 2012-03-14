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

#ifndef KPF_PROPERTIES_DIALOG_PLUGIN_H
#define KPF_PROPERTIES_DIALOG_PLUGIN_H

#include <kpropertiesdialog.h>

namespace KPF
{
  /**
   * Provides an implementation of KPropsDlgPlugin which is plugged into
   * Konqueror's directory properties dialog. Allows creating and configuring
   * WebServer objects via DCOP conversations with the KPFInterface. Also
   * allows starting the kpf applet if it is not already loaded by kicker.
   */
  class PropertiesDialogPlugin : public KPropsDlgPlugin
  {
    Q_OBJECT

    public:

      PropertiesDialogPlugin(KPropertiesDialog *, const char *, const QStringList &);

      virtual ~PropertiesDialogPlugin();

      virtual void applyChanges();

    protected slots:

      void slotSharingToggled(bool);
      void slotStartKPF();
      void slotStartKPFFailed();

      void slotApplicationRegistered(const QCString &);
      void slotApplicationUnregistered(const QCString &);

      void slotChanged();

    protected:

      void getServerRef();

      void updateGUIFromCurrentState();
      void updateWantedStateFromGUI();

      QWidget * createInitWidget(QWidget * parent);
      QWidget * createConfigWidget(QWidget * parent);

      void readSettings();
      void setControlsEnabled(bool);
      bool userAcceptsWarning() const;

    private:

      class Private;
      Private * d;
  };
}

#endif
// vim:ts=2:sw=2:tw=78:et
