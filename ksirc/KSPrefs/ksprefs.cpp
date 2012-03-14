/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>

#include <kdebug.h>

#include "ksprefs.moc"
#include "page_general.h"
#include "page_colors.h"
#include "page_startup.h"
#include "page_rmbmenu.h"
#include "page_servchan.h"
#include "page_font.h"
#include "page_autoconnect.h"
#include "page_irccolors.h"
#include "page_looknfeel.h"
#include "page_shortcuts.h"

KSPrefs::KSPrefs(QWidget * parent, const char * name):
    KDialogBase(KDialogBase::IconList, i18n("Configure KSirc"),
                KDialogBase::Help | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Default,
                KDialogBase::Ok, parent, name)
{
    setWFlags( getWFlags() | WDestructiveClose );
    QFrame *itemLooknFeel= addPage( i18n( "Look and Feel" ), i18n( "Controls how kSirc looks" ), BarIcon( "ksirc", KIcon::SizeMedium ) );
    QFrame *itemGeneral  = addPage( i18n( "General" ), i18n( "General KSirc Settings" ), BarIcon( "ksirc", KIcon::SizeMedium ) );
    QFrame *itemStartup  = addPage( i18n( "Startup" ), i18n( "KSirc Startup Settings" ), BarIcon( "gear", KIcon::SizeMedium ) );
    QFrame *itemColors   = addPage( i18n( "Colors" ), i18n( "KSirc Color Settings" ), BarIcon( "colors", KIcon::SizeMedium ) );
    QFrame *itemIRCColors   = addPage( i18n( "IRC Colors" ), i18n( "KSirc IRC Color Settings" ), BarIcon( "colors", KIcon::SizeMedium ) );

    QFrame *itemRMBMenu  = addPage( i18n( "User Menu" ), i18n( "User Menu Configuration" ), BarIcon( "", KIcon::SizeMedium ) );
    QFrame *itemServChan = addPage( i18n( "Server/Channel" ), i18n( "Server/Channel Configuration" ), BarIcon( "", KIcon::SizeMedium ) );
    QFrame *itemAutoConnect = addPage( i18n( "Auto Connect" ), i18n( "Auto Connect Configuration" ), BarIcon( "", KIcon::SizeMedium ) );
    QFrame *itemFont = addPage( i18n( "Fonts" ), i18n( "Font Settings" ), BarIcon( "fonts", KIcon::SizeMedium ) );
    QFrame *itemShortcuts = addPage( i18n( "Shortcuts" ), i18n( "Shortcut Configuration" ), BarIcon( "", KIcon::SizeMedium ) );

    QVBoxLayout *fontTopLayout  = new QVBoxLayout( itemFont, 0, 6 );


    QVBoxLayout *looknfeelTopLayout= new QVBoxLayout( itemLooknFeel, 0, 6 );
    QVBoxLayout *generalTopLayout  = new QVBoxLayout( itemGeneral, 0, 6 );
    QVBoxLayout *startupTopLayout  = new QVBoxLayout( itemStartup, 0, 6 );
    QVBoxLayout *colorsTopLayout   = new QVBoxLayout( itemColors, 0, 6 );
    QVBoxLayout *ircColorsTopLayout   = new QVBoxLayout( itemIRCColors, 0, 6 );

    QVBoxLayout *rmbMenuTopLayout  = new QVBoxLayout( itemRMBMenu, 0, 6 );
    QVBoxLayout *servChanTopLayout = new QVBoxLayout( itemServChan, 0, 6 );
    QVBoxLayout *autoConnectTopLayout = new QVBoxLayout( itemAutoConnect, 0, 6 );
    QVBoxLayout *autoShortcutsTopLayout = new QVBoxLayout( itemShortcuts, 0, 6 );

    pageLooknFeel= new PageLooknFeel( itemLooknFeel );
    pageGeneral  = new PageGeneral( itemGeneral );
    pageStartup  = new PageStartup( itemStartup );
    pageColors   = new PageColors( itemColors );
    pageIRCColors   = new PageIRCColors( itemIRCColors );
    pageFont     = new PageFont( itemFont );
    pageRMBMenu  = new PageRMBMenu( itemRMBMenu );
    pageServChan = new PageServChan( itemServChan );
    pageAutoConnect = new PageAutoConnect( itemAutoConnect );
    pageShortcuts = new PageShortcuts( itemShortcuts );

    connect(pageLooknFeel, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageGeneral, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageStartup, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageColors, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageIRCColors, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageFont, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageRMBMenu, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageServChan, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageAutoConnect, SIGNAL( modified() ), SLOT( modified() ) );
    connect(pageShortcuts, SIGNAL( modified() ), SLOT( modified() ) );

    connect(this, SIGNAL( applyClicked() ), this, SLOT( saveConfig() ) );
    connect(this, SIGNAL( okClicked() ), this, SLOT( saveConfig() ) );
    connect(this, SIGNAL( defaultClicked() ), this, SLOT(defaultConfig() ) );

    looknfeelTopLayout->addWidget( pageLooknFeel );
    generalTopLayout->addWidget( pageGeneral );
    startupTopLayout->addWidget( pageStartup );
    colorsTopLayout->addWidget( pageColors );
    ircColorsTopLayout->addWidget( pageIRCColors );
    fontTopLayout->addWidget( pageFont );
    rmbMenuTopLayout->addWidget( pageRMBMenu );
    servChanTopLayout->addWidget( pageServChan );
    autoConnectTopLayout->addWidget( pageAutoConnect );
    autoShortcutsTopLayout->addWidget( pageShortcuts );
	//enableButtonSeperator( true );

    readConfig();
    enableButtonApply( false );

    pSCDirty = 0;
    pACDirty = 0;
}

KSPrefs::~KSPrefs()
{
}

void KSPrefs::readConfig()
{
	// apply by calling readConfig in each page

        pageLooknFeel->readConfig();
	pageGeneral->readConfig();
        pageColors->readConfig();
        pageIRCColors->readConfig();
	pageStartup->readConfig();
	pageFont->readConfig();
	pageRMBMenu->readConfig();
        pageServChan->readConfig();
	pageAutoConnect->readConfig();
        pageShortcuts->readConfig();

}

void KSPrefs::saveConfig()
{
	// apply by calling saveConfig in each page
	// use setDirty flag for each page and
	// emit update() appropriate

    if ( dirty & KSOptions::General )
        pageGeneral->saveConfig();
    if ( dirty & KSOptions::Startup )
        pageStartup->saveConfig();
    if ( dirty & KSOptions::Colors )
        pageColors->saveConfig();
    if ( dirty & KSOptions::Colors )
        pageIRCColors->saveConfig();
    if ( dirty & KSOptions::Colors )
        pageFont->saveConfig();
    if ( dirty & KSOptions::RMBMenu )
        pageRMBMenu->saveConfig();
    if ( dirty & pSCDirty )
        pageServChan->saveConfig();
    if ( dirty & pACDirty )
	pageAutoConnect->saveConfig();
    if ( dirty & pLFDirty )
        pageLooknFeel->saveConfig();
    if ( dirty & pShortDirty )
        pageShortcuts->saveConfig();


    ksopts->save( dirty );
    emit update( dirty );
    enableButtonApply( false );
}


void KSPrefs::defaultConfig()
{
	// apply by calling defaultConfig in current page

    switch (activePageIndex())
    {
    case 0:
        kdDebug(5008) << "Doing looknfeel" << endl;
        pageLooknFeel->defaultConfig();
        break;
    case 1:
        kdDebug(5008) << "Doing general" << endl;
        pageGeneral->defaultConfig();
        break;
    case 2:
        kdDebug(5008) << "Doing startup" << endl;
        pageStartup->defaultConfig();
        break;
    case 3:
        kdDebug(5008) << "Doing colors" << endl;
        pageColors->defaultConfig();
        break;
    case 4:
        kdDebug(5008) << "Doing IRC colors" << endl;
        pageIRCColors->defaultConfig();
        break;
    case 5:
        kdDebug(5008) << "Doing RMBMenu" << endl;
        pageRMBMenu->defaultConfig();
        break;
    case 6:
        kdDebug(5008) << "Doing ServerChan" << endl;
        pageServChan->defaultConfig();
        break;
    case 7:
        kdDebug(5008) << "Doing AutoConnect" << endl;
        pageAutoConnect->defaultConfig();
    case 8:
        kdDebug(5008) << "Doing font" << endl;
        pageFont->defaultConfig();
        break;
    case 9:
        kdDebug(5008) << "Doing shortcuts" << endl;
        pageShortcuts->defaultConfig();
        break;

    }
}

void KSPrefs::modified()
{
    if ( sender() == pageGeneral )
        dirty |= static_cast<int>( KSOptions::General );
    if ( sender() == pageStartup ) {
	dirty |= static_cast<int>( KSOptions::Startup );
	dirty |= static_cast<int>( KSOptions::Servers );
    }
    if ( sender() == pageColors )
        dirty |= static_cast<int>( KSOptions::Colors );
    if ( sender() == pageIRCColors )
        dirty |= static_cast<int>( KSOptions::Colors );
    if ( sender() == pageFont )
        dirty |= static_cast<int>( KSOptions::Colors );
    if ( sender() == pageRMBMenu )
        dirty |= static_cast<int>( KSOptions::RMBMenu );
    if ( sender() == pageServChan ) {
        dirty |= 1;
        pSCDirty = 1;
    }
    if ( sender() == pageAutoConnect ) {
        dirty |= 1;
        pACDirty = 1;
    }
    if ( sender() == pageLooknFeel ) {
        dirty |= 1;
        pLFDirty = 1;
    }
    if ( sender() == pageShortcuts ) {
        dirty |= 1;
        pShortDirty = 1;
    }

    enableButtonApply( dirty );
}
