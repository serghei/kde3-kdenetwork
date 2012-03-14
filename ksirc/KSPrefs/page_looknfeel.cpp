/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include "page_looknfeel.h"

PageLooknFeel::PageLooknFeel( QWidget *parent, const char *name ) : PageLooknFeelBase( parent, name)
{
    modePreview->setPixmap(QPixmap(locate("data","ksirc/pics/sdi.png")));
    wallpaperPathLE->fileDialog()->setFilter( "*.jpg *.png *.gif" );
}

PageLooknFeel::~PageLooknFeel()
{
}

void PageLooknFeel::saveConfig()
{
    if ( mdiCB->isChecked() ) ksopts->displayMode = KSOptions::MDI;
    if ( sdiCB->isChecked() ) ksopts->displayMode = KSOptions::SDI;

    ksopts->backgroundFile = wallpaperPathLE->url();

}

void PageLooknFeel::readConfig( const KSOGeneral *opts )
{
    if ( opts->displayMode == KSOptions::MDI )
        mdiCB->setChecked( true );
    else
        sdiCB->setChecked( true );

    wallpaperPathLE->setURL( opts->backgroundFile );
}

void PageLooknFeel::defaultConfig()
{
    KSOGeneral opts;
    readConfig( &opts );
}

void PageLooknFeel::setPreviewPixmap( bool isSDI )
{
    if (isSDI == true)
        modePreview->setPixmap( QPixmap( locate("data", "ksirc/pics/sdi.png" ) ) );
    else
        modePreview->setPixmap( QPixmap( locate("data", "ksirc/pics/mdi.png" ) ) );
}

void PageLooknFeel::showWallpaperPixmap( const QString &url )
{
    wallpaperPreview->setPixmap( QPixmap( url ) );
}

void PageLooknFeel::changed()
{
    emit modified();
}

#include "page_looknfeel.moc"
