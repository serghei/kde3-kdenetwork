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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kconfig.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kprocess.h>

#include "kdictapplet.h"

//*********  PopupBox  ********************************************

PopupBox::PopupBox()
 : QHBox(0, 0, WStyle_Customize | WType_Popup ), popupEnabled(true)
{
}


PopupBox::~PopupBox()
{}


bool PopupBox::showBox()
{
  if (!popupEnabled)    // prevents that the popup is shown again immediatly
    return false;
  else {
    show();
    return true;
  }
}


void PopupBox::hideEvent(QHideEvent *)
{
  emit(hidden());
  popupEnabled = false;
  QTimer::singleShot(100, this, SLOT(enablePopup()));
}


void PopupBox::enablePopup()
{
  popupEnabled = true;
}


//*********  DictApplet  ********************************************

extern "C"
{
  KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
  {
    KGlobal::locale()->insertCatalogue("kdictapplet");
    return new DictApplet(configFile, KPanelApplet::Stretch, 0, parent, "kdictapplet");
  }
}


DictApplet::DictApplet(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
  : KPanelApplet(configFile, type, actions, parent, name), waiting(0)
{
  // first the widgets for a horizontal panel
  baseWidget = new QWidget(this);
  QGridLayout *baseLay = new QGridLayout(baseWidget,2,6,0,1);

  textLabel = new QLabel(i18n("Dictionary:"), baseWidget);
  textLabel->setBackgroundOrigin(AncestorOrigin);
  QFont f(textLabel->font());
  f.setPixelSize(12);
  textLabel->setFont(f);
  baseLay->addWidget(textLabel,0,1);
  QToolTip::add(textLabel,i18n("Look up a word or phrase with Kdict"));

  iconLabel = new QLabel(baseWidget);
  iconLabel->setBackgroundOrigin(AncestorOrigin);
  QPixmap pm = KGlobal::iconLoader()->loadIcon("kdict", KIcon::Panel, KIcon::SizeSmall, KIcon::DefaultState, 0L, true);
  iconLabel->setPixmap(pm);
  baseLay->addWidget(iconLabel,1,0);
  iconLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  iconLabel->setFixedWidth(pm.width()+4);
  QToolTip::add(iconLabel,i18n("Look up a word or phrase with Kdict"));

  f.setPixelSize(10);
  clipboardBtn = new QPushButton(i18n("C"),baseWidget);
  clipboardBtn->setBackgroundOrigin(AncestorOrigin);
  clipboardBtn->setFont(f);
  clipboardBtn->setFixedSize(16,16);
  connect(clipboardBtn, SIGNAL(clicked()), SLOT(queryClipboard()));
  baseLay->addWidget(clipboardBtn,0,3);
  QToolTip::add(clipboardBtn,i18n("Define selected text"));

  defineBtn = new QPushButton(i18n("D"),baseWidget);
  defineBtn->setBackgroundOrigin(AncestorOrigin);
  defineBtn->setFont(f);
  defineBtn->setFixedSize(16,16);
  defineBtn->setEnabled(false);
  connect(defineBtn, SIGNAL(clicked()), SLOT(startDefine()));
  baseLay->addWidget(defineBtn,0,4);
  QToolTip::add(defineBtn,i18n("Define word/phrase"));

  matchBtn = new QPushButton(i18n("M"),baseWidget);
  matchBtn->setBackgroundOrigin(AncestorOrigin);
  matchBtn->setFont(f);
  matchBtn->setFixedSize(16,16);
  matchBtn->setEnabled(false);
  connect(matchBtn, SIGNAL(clicked()), SLOT(startMatch()));
  baseLay->addWidget(matchBtn,0,5);
  QToolTip::add(matchBtn,i18n("Find matching definitions"));

  completionObject = new KCompletion();

  internalCombo = new KHistoryCombo(baseWidget);
  internalCombo->setBackgroundOrigin(AncestorOrigin);
  internalCombo->setCompletionObject(completionObject);
  internalCombo->setFocus();
  internalCombo->clearEdit();
  internalCombo->lineEdit()->installEventFilter( this );
  connect(internalCombo, SIGNAL(returnPressed(const QString&)), SLOT(startQuery(const QString&)));
  connect(internalCombo, SIGNAL(textChanged(const QString&)), SLOT(comboTextChanged(const QString&)));
  QToolTip::add(internalCombo,i18n("Look up a word or phrase with Kdict"));

  baseLay->addMultiCellWidget(internalCombo,1,1,1,5);

  baseLay->setColStretch(2,1);
	
  // widgets for a vertical panel
  verticalBtn = new QPushButton(this);
  connect(verticalBtn, SIGNAL(pressed()), SLOT(showExternalCombo()));	
  QToolTip::add(verticalBtn,i18n("Look up a word or phrase with Kdict"));

  popupBox = new PopupBox();
  popupBox->setFixedSize(160, 22);
  connect(popupBox, SIGNAL(hidden()), SLOT(externalComboHidden()));
  externalCombo = new KHistoryCombo(popupBox);
  externalCombo->setCompletionObject(completionObject);  
  connect(externalCombo, SIGNAL(returnPressed(const QString&)), SLOT(startQuery(const QString&)));
  externalCombo->setFixedSize(160, externalCombo->sizeHint().height());
  
  connect(internalCombo, SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          this, SLOT(updateCompletionMode(KGlobalSettings::Completion)));
  connect(externalCombo, SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          this, SLOT(updateCompletionMode(KGlobalSettings::Completion)));

  // restore history and completion list
  KConfig *c = config();
  c->setGroup("General");

  QStringList list = c->readListEntry("Completion list");
  completionObject->setItems(list);
  int mode = c->readNumEntry("Completion mode", 
                             KGlobalSettings::completionMode());
  internalCombo->setCompletionMode((KGlobalSettings::Completion)mode);  
  externalCombo->setCompletionMode((KGlobalSettings::Completion)mode);  

  list = c->readListEntry("History list");
  internalCombo->setHistoryItems(list);  
  externalCombo->setHistoryItems(list);
}


DictApplet::~DictApplet()
{
  // save history and completion list
  KConfig *c = config();
  c->setGroup("General");

  QStringList list = completionObject->items();
  c->writeEntry("Completion list", list);
  c->writeEntry("Completion mode", (int) internalCombo->completionMode());
  
  list = internalCombo->historyItems();
  c->writeEntry("History list", list);  
  c->sync();
  
  delete completionObject;
}


int DictApplet::widthForHeight(int height) const
{
  if (height >= 38)
    return textLabel->sizeHint().width()+55;
  else
    return textLabel->sizeHint().width()+25;
}


int DictApplet::heightForWidth(int width) const
{
  return width;
}


void DictApplet::resizeEvent(QResizeEvent*)
{
  if (orientation() == Horizontal) {
    verticalBtn->hide();
    baseWidget->show();
    baseWidget->setFixedSize(width(),height());

    if (height() < internalCombo->sizeHint().height())
      internalCombo->setFixedHeight(height());
    else
      internalCombo->setFixedHeight(internalCombo->sizeHint().height());

    if (height() >= 38) {
      textLabel->show();
      clipboardBtn->show();
      defineBtn->show();
      matchBtn->show();
      iconLabel->hide();
      internalCombo->setFixedWidth(width());
    } else {
      textLabel->hide();
      clipboardBtn->hide();
      defineBtn->hide();
      matchBtn->hide();
      iconLabel->show();
      internalCombo->setFixedWidth(width()-iconLabel->width()-1);
    }

    baseWidget->updateGeometry();
	} else 	{     // orientation() == Vertical
    verticalBtn->show();
    baseWidget->hide();
    verticalBtn->setFixedSize(width(),width());

    KIcon::StdSizes sz = 	width() < 32 ?	KIcon::SizeSmall : (width() < 48 ? KIcon::SizeMedium : KIcon::SizeLarge);
    QPixmap pm = KGlobal::iconLoader()->loadIcon("kdict", KIcon::Panel, sz, KIcon::DefaultState, 0L, true);
    verticalBtn->setPixmap(pm);
	}
}


bool DictApplet::eventFilter( QObject *o, QEvent * e)
{
  if (e->type() == QEvent::MouseButtonRelease)
  	emit requestFocus();

  return KPanelApplet::eventFilter(o, e);
}


void DictApplet::sendCommand(const QCString &fun, const QString &data)
{
  if (waiting > 0) {
    waiting = 1;
    delayedFunc = fun.copy();
    delayedData = data;
    return;
  }

  DCOPClient *client = kapp->dcopClient();
  if (!client->isApplicationRegistered("kdict")) {
    KApplication::startServiceByDesktopName("kdict");
    waiting = 1;
    delayedFunc = fun.copy();
    delayedData = data;
    QTimer::singleShot(100, this, SLOT(sendDelayedCommand()));
    return;
  } else {
    QCStringList list = client->remoteObjects("kdict");
    if (list.findIndex("KDictIface")==-1) {
      waiting = 1;
      delayedFunc = fun.copy();
      delayedData = data;
      QTimer::singleShot(100, this, SLOT(sendDelayedCommand()));
      return;
    }
  }

  client->send("kdict","default",fun,data);
}


void DictApplet::sendDelayedCommand()
{
  if (waiting > 100) {   // timeout after ten seconds
    waiting = 0;
    return;
  }

  DCOPClient *client = kapp->dcopClient();
  if (!client->isApplicationRegistered("kdict")) {
    waiting++;
    QTimer::singleShot(100, this, SLOT(sendDelayedCommand()));
    return;
  } else {
    QCStringList list = client->remoteObjects("kdict");
    if (list.findIndex("KDictIface")==-1) {
      waiting++;
      QTimer::singleShot(100, this, SLOT(sendDelayedCommand()));
      return;
    }
  }

  client->send("kdict","default",delayedFunc,delayedData);
  waiting = 0;
}


void DictApplet::startQuery(const QString &s)
{
  QString query = s.stripWhiteSpace();
  if (query.isEmpty())
    return;

  internalCombo->addToHistory(query);
  externalCombo->addToHistory(query);
  internalCombo->clearEdit();
  externalCombo->clearEdit();

  sendCommand("definePhrase(QString)",query);

  if (orientation() == Vertical)
    popupBox->hide();
}


void DictApplet::comboTextChanged(const QString &s)
{
  defineBtn->setEnabled(!s.isEmpty());
  matchBtn->setEnabled(!s.isEmpty());
}


void DictApplet::queryClipboard()
{
  sendCommand("defineClipboardContent()",QString::null);
}


void DictApplet::startDefine()
{
  startQuery(internalCombo->currentText());
}


void DictApplet::startMatch()
{
  QString query = internalCombo->currentText().stripWhiteSpace();
  internalCombo->addToHistory(query);
  externalCombo->addToHistory(query);
  internalCombo->clearEdit();
  externalCombo->clearEdit();

  sendCommand("matchPhrase(QString)",query);
}


void DictApplet::showExternalCombo()
{
  QPoint p;
  if (position() == pLeft)
    p = mapToGlobal(QPoint(-popupBox->width()-1, 0));
  else
    p = mapToGlobal(QPoint(width()+1, 0));
  popupBox->move(p);
  if (popupBox->showBox())
    externalCombo->setFocus();
  else
    verticalBtn->setDown(false);
}


void DictApplet::externalComboHidden()
{
  verticalBtn->setDown(false);
}

void DictApplet::updateCompletionMode(KGlobalSettings::Completion mode)
{
  internalCombo->setCompletionMode(mode);
  externalCombo->setCompletionMode(mode);
}

//--------------------------------

#include "kdictapplet.moc"
