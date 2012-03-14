/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "page_servchan.h"

#include <qlistbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kapplication.h>
#include <kconfig.h>

PageServChan::PageServChan( QWidget *parent, const char *name ) : PageServChanBase( parent, name)
{
    connect(serverDeleteItemPB, SIGNAL(pressed()), this, SLOT(deletePressedSL()));
    connect(ServerAddItemPB, SIGNAL(pressed()), this, SLOT(addPressedSL()));
    connect(chanDeleteItmPB, SIGNAL(pressed()), this, SLOT(deletePressedCL()));
    connect(ChanAddItemPB, SIGNAL(pressed()), this, SLOT(addPressedCL()));

}

PageServChan::~PageServChan()
{
}

void PageServChan::saveConfig()
{
    QStringList sLB;
    uint i;
    for(i = 0; i < serverLB->count(); i++){
        QString txt = serverLB->text(i);
        if(!txt.isNull())
            sLB << txt;
    }
    KConfig *conf = kapp->config();
    conf->setGroup("ServerList");
    conf->writeEntry("RecentServers", sLB);

    QStringList sCL;
    for(i = 0; i < channelLB->count(); i++){
        QString txt = channelLB->text(i);
        if(!txt.isNull())
            sCL << txt;
    }
    conf->setGroup("Recent");
    conf->writeEntry("Channels", sCL);
}

void PageServChan::readConfig( const KSOServChan * )
{
    KConfig *conf = kapp->config();
    conf->setGroup("ServerList");
    QStringList recent = conf->readListEntry("RecentServers");
    recent.sort();
    serverLB->insertStringList(recent);
    conf->setGroup("Recent");
    recent = conf->readListEntry("Channels");
    recent.sort();
    channelLB->insertStringList(recent);
}

void PageServChan::defaultConfig()
{
    readConfig();
}

void PageServChan::deletePressedSL() {
    int item = serverLB->currentItem();
    if(item >= 0){
        serverLB->removeItem(item);
    }
    emit modified();
}

void PageServChan::addPressedSL() {
    uint i;
    QString txt = LineEdit6->text();

    for(i = 0; i < serverLB->count(); i++){
        if(txt == serverLB->text(i)){
            qWarning("Server already in the list!");
            return;
        }
    }

    serverLB->insertItem(txt);
    serverLB->sort();
    LineEdit6->clear();
    emit modified();
    for(i = 0; i < serverLB->count(); i++){
        if(txt == serverLB->text(i)){
            serverLB->setCurrentItem(i);
            serverLB->ensureCurrentVisible();
            return;
        }
    }
}

void PageServChan::deletePressedCL() {
    int item = channelLB->currentItem();
    if(item >= 0){
        channelLB->removeItem(item);
    }
    emit modified();
}

void PageServChan::addPressedCL() {
    uint i;
    QString txt = LineEdit6_2->text();

    for(i = 0; i < channelLB->count(); i++){
        if(txt == channelLB->text(i)){
            qWarning("Server already in the list!");
            return;
        }
    }

    channelLB->insertItem(txt);
    channelLB->sort();
    LineEdit6_2->clear();
    emit modified();
    for(i = 0; i < channelLB->count(); i++){
        if(txt == channelLB->text(i)){
            channelLB->setCurrentItem(i);
            channelLB->ensureCurrentVisible();
            return;
        }
    }
}
#include "page_servchan.moc"
