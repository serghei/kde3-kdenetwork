/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_COLORS_H
#define PAGE_COLORS_H

#include "page_colorsbase.h"
#include "ksopts.h"

#include <qlistbox.h>
#include <qstringlist.h>
#include <qdict.h>

class PageColors : public PageColorsBase
{
Q_OBJECT

public:
	PageColors( QWidget *parent = 0, const char *name = 0 );
	~PageColors();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOColors * = ksopts );

protected slots:
    void changed();

public slots:
    virtual void theme_clicked(QListBoxItem*);
    virtual void themeNewPB_clicked();
    virtual void themeAddPB_clicked();
    virtual void themeDelPB_clicked();
    virtual void theme_sel();

signals:
    void modified();

private:
    void coloursSetEnable();

private:
    int changing;
    QDict<KSOColors> m_dcol;
};

#endif
