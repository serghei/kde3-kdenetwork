/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_SHORTCUTS_H
#define PAGE_SHORTCUTS_H

#include "page_shortcutsbase.h"
#include "ksopts.h"

class KKeyChooser;

class PageShortcuts : public PageShortcutsBase
{
Q_OBJECT

public:
    PageShortcuts( QWidget *parent = 0, const char *name = 0 );
    ~PageShortcuts();

    void saveConfig();
    void defaultConfig();
    void readConfig( const KSOGeneral * = ksopts );

signals:
    void modified();

public slots:
    virtual void changed();

private:
    KKeyChooser *m_key;
};

#endif
