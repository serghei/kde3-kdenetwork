/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kcharsets.h>
#include "page_general.h"
#include "../servercontroller.h"

PageGeneral::PageGeneral( QWidget *parent, const char *name ) : PageGeneralBase( parent, name)
{
}

PageGeneral::~PageGeneral()
{
}

void PageGeneral::saveConfig()
{
    ksopts->runDocked = dockedOnlyCB->isChecked();
    servercontroller::self()->checkDocking();
    ksopts->autoCreateWin = autoCreateWindowCB->isChecked();
    ksopts->autoCreateWinForNotice = autoCreateWindowForNoticeCB->isChecked();
    ksopts->nickCompletion = nickCompletionCB->isChecked();
    ksopts->displayTopic = displayTopicCB->isChecked();
    ksopts->colorPicker = colorPickerPopupCB->isChecked();
    ksopts->autoRejoin = autoRejoinCB->isChecked();
    ksopts->oneLineEntry = oneLineEditCB->isChecked();
    ksopts->useColourNickList = useColourNickListCB->isChecked();
    ksopts->dockPopups = dockPopupsCB->isChecked();
    ksopts->autoSaveHistory = autoSaveHistoryCB->isChecked();
    ksopts->windowLength = historySB->value();

    ksopts->channel["global"]["global"].timeStamp = timeStampCB->isChecked();
    ksopts->channel["global"]["global"].beepOnMsg = beepCB->isChecked();
    ksopts->channel["global"]["global"].topicShow = showTopicCB->isChecked();
    ksopts->channel["global"]["global"].logging = enLoggingCB->isChecked();
    ksopts->channel["global"]["global"].encoding = encodingsCB->currentText();
    ksopts->channel["global"]["global"].filterJoinPart = joinPartCB->isChecked();

    if(applyGloballyCB->isChecked()){
        ksopts->applyChannelGlobal();
    }

    ksopts->publicAway = publicAway->isChecked();
}

void PageGeneral::readConfig( const KSOGeneral *opts )
{
    dockedOnlyCB->setChecked( opts->runDocked );
    autoCreateWindowCB->setChecked( opts->autoCreateWin );
    autoCreateWindowForNoticeCB->setChecked( opts->autoCreateWinForNotice );
    nickCompletionCB->setChecked( opts->nickCompletion );
    displayTopicCB->setChecked( opts->displayTopic );
    colorPickerPopupCB->setChecked( opts->colorPicker );
    autoRejoinCB->setChecked( opts->autoRejoin );
    oneLineEditCB->setChecked( opts->oneLineEntry );
    useColourNickListCB->setChecked( opts->useColourNickList );
    dockPopupsCB->setChecked( opts->dockPopups );
    dockPopupsCB->setEnabled(dockedOnlyCB->isChecked());

    autoSaveHistoryCB->setChecked( opts->autoSaveHistory );

    historySB->setValue( opts->windowLength );

    timeStampCB->setChecked( ksopts->channel["global"]["global"].timeStamp );
    beepCB->setChecked( ksopts->channel["global"]["global"].beepOnMsg );
    showTopicCB->setChecked( ksopts->channel["global"]["global"].topicShow );
    enLoggingCB->setChecked( ksopts->channel["global"]["global"].logging );
    joinPartCB->setChecked( ksopts->channel["global"]["global"].filterJoinPart );

    publicAway->setChecked( ksopts->publicAway );

    encodingsCB->clear();


    QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();

    // remove utf16/ucs2 as it just doesn't work for IRC
    QStringList::Iterator encodingIt = encodings.begin();
    while ( encodingIt != encodings.end() ) {
	if ( ( *encodingIt ).find( "utf16" ) != -1 ||
	     ( *encodingIt ).find( "iso-10646" ) != -1 )
	    encodingIt = encodings.remove( encodingIt );
	else
	    ++encodingIt;
    }
    encodings.prepend( i18n( "Default" ) );
    encodingsCB->insertStringList(encodings);
    int eindex = encodings.findIndex(ksopts->channel["global"]["global"].encoding);
    if(eindex == -1)
	encodingsCB->setCurrentItem(0);
    else
        encodingsCB->setCurrentItem(eindex);
}

void PageGeneral::defaultConfig()
{
    KSOGeneral opts;
    readConfig( &opts );
}

void PageGeneral::changed()
{
    dockPopupsCB->setEnabled(dockedOnlyCB->isChecked());
    emit modified();
}

#include "page_general.moc"
