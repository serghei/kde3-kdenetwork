/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_SERVCHAN_H
#define PAGE_SERVCHAN_H

#include "page_servchanbase.h"
#include "ksopts.h"

class PageServChan : public PageServChanBase
{
Q_OBJECT

public:
    PageServChan( QWidget *parent = 0, const char *name = 0 );
    ~PageServChan();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOServChan * = ksopts );

signals:
    void modified();

public slots:
    void deletePressedSL();
    void addPressedSL();
    void deletePressedCL();
    void addPressedCL();
};

#endif
