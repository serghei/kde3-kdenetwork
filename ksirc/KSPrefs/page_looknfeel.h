/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_LOOKNFEEL_H
#define PAGE_LOOKNFEEL_H

#include "page_looknfeelbase.h"
#include "ksopts.h"

class PageLooknFeel : public PageLooknFeelBase
{
Q_OBJECT

public:
    PageLooknFeel( QWidget *parent = 0, const char *name = 0 );
    ~PageLooknFeel();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOGeneral * = ksopts );

signals:
    void modified();

public slots:
    virtual void setPreviewPixmap( bool isSDI );
    virtual void showWallpaperPixmap( const QString &url );
    virtual void changed();
};

#endif
