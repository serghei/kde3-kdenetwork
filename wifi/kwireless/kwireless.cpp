/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "kwireless.h"
#include "kwirelesswidget.h"


KWireLess::KWireLess(const QString& configFile, Type type,
                     int actions, QWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name)
{
    ksConfig = config();

    widget = KWireLessWidget::makeWireLessWidget(this);
    widget->show();
}


KWireLess::~KWireLess()
{
}


void KWireLess::about()
{
    KMessageBox::information
        (0,
         i18n("<qt><b>KWireLess</b><br>"
              "Displays information about wireless network devices.<br />"
              "KWireLess is licensed to you under the terms of the GPL.<br />"
              "<i>(C) 2003 Mirko Boehm</i></qt>"),
         i18n("About KWireLess"));
}


void KWireLess::help()
{
//    KMessageBox::information(0, i18n("This is a help box"));
}


void KWireLess::preferences()
{
//    KMessageBox::information(0, i18n("This is a preferences box"));
}

int KWireLess::widthForHeight(int) const
{
    widget->setMode(KWireLessWidget::Vertical);
    return widget->preferredWidth();
}

int KWireLess::heightForWidth(int) const
{
    widget->setMode(KWireLessWidget::Horizontal);
    return widget->preferredHeight();
}

void KWireLess::resizeEvent(QResizeEvent *)
{
    widget->setGeometry(0, 0, width(), height());
}


extern "C"
{
    KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString configFile)
    {
        KGlobal::locale()->insertCatalogue("kwireless");
        return new KWireLess(configFile, KPanelApplet::Normal,
                             KPanelApplet::About,
                             // | KPanelApplet::Help | KPanelApplet::Preferences,
                             parent, "kwireless");
    }
}

#include "kwireless.moc"
