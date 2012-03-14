/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_IRCCOLORS_H
#define PAGE_IRCCOLORS_H

#include "page_irccolorsbase.h"
#include "ksopts.h"

class PageIRCColors : public PageIRCColorsBase
{
Q_OBJECT

public:
    PageIRCColors( QWidget *parent = 0, const char *name = 0 );
    ~PageIRCColors();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOColors * = ksopts );

protected slots:
    void changed();

public slots:

signals:
    void modified();

private:

};

#endif
