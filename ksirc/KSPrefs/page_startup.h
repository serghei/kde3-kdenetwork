/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_STARTUP_H
#define PAGE_STARTUP_H

#include "page_startupbase.h"
#include "ksopts.h"

class PageStartup : public PageStartupBase
{
Q_OBJECT

public:
    PageStartup( QWidget *parent = 0, const char *name = 0 );
    ~PageStartup();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOptions * = ksopts );

signals:
    void modified();

protected slots:
    void changed();
    void clickedLB(int index);

private:
    ServerOpMap server;
    bool changing;
};

#endif
