/* -------------------------------------------------------------

   kdictapplet.h (part of The KDE Dictionary Client)

   Copyright (C) 2001 Christian Gebauer <gebauer@kde.org>

   The applet is loosely based on the "Run" applet included in KDE.
   Copyright (c) 2000 Matthias Elter <elter@kde.org> (Artistic License)

   This file is distributed under the Artistic License.
   See LICENSE for details.

    -------------------------------------------------------------

    PopupBox        helper class
    DictApplet      a small kicker-applet

 ------------------------------------------------------------- */

#ifndef _DICTAPPLET_H_
#define _DICTAPPLET_H_

#include <qhbox.h>
#include <kpanelapplet.h>

class QLabel;
class QPushButton;
class KHistoryCombo;


//*********  PopupBox  ********************************************

class PopupBox : public QHBox
{
  Q_OBJECT

public:
  PopupBox();
  ~PopupBox();

  bool showBox();

signals:
  void hidden();

public slots:
  void enablePopup();

protected:
  void hideEvent(QHideEvent *);

private:
  bool popupEnabled;

};

//*********  DictApplet  ********************************************

class DictApplet : public KPanelApplet
{
  Q_OBJECT

public:
  DictApplet(const QString& configFile, Type t = Stretch, int actions = 0, QWidget *parent = 0, const char *name = 0);
  virtual ~DictApplet();

  int widthForHeight(int height) const;
  int heightForWidth(int width) const;

protected:
  void resizeEvent(QResizeEvent*);
  bool eventFilter( QObject *, QEvent * );

  void sendCommand(const QCString &fun, const QString &data);

protected slots:
  void sendDelayedCommand();
  void startQuery(const QString&);
  void comboTextChanged(const QString&);
  void queryClipboard();
  void startDefine();
  void startMatch();
  void showExternalCombo();
  void externalComboHidden();
  void updateCompletionMode(KGlobalSettings::Completion mode);

private:
  KHistoryCombo  *internalCombo, *externalCombo;
  KCompletion    *completionObject;
  QLabel         *textLabel, *iconLabel;
  QPushButton    *verticalBtn, *clipboardBtn, *defineBtn, *matchBtn;
  QWidget        *baseWidget;
  PopupBox       *popupBox;

  int            waiting;
  QCString       delayedFunc;
  QString        delayedData;

};

#endif
