/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qgroupbox.h>

#include <kglobalaccel.h>

#include <kkeydialog.h>
#include "page_shortcuts.h"
#include "../servercontroller.h"

PageShortcuts::PageShortcuts( QWidget *parent, const char *name ) : PageShortcutsBase( parent, name)
{
    globalGB->setColumnLayout( 0, Qt::Horizontal );

    m_key = new KKeyChooser(servercontroller::self()->getGlobalAccel(), globalGB);
    connect(m_key, SIGNAL(keyChange()), this, SLOT(changed()));
    globalGB->layout()->add(m_key);
}

PageShortcuts::~PageShortcuts()
{
}

void PageShortcuts::saveConfig()
{
    m_key->commitChanges();
    servercontroller::self()->getGlobalAccel()->writeSettings();
    servercontroller::self()->getGlobalAccel()->updateConnections();
}

void PageShortcuts::readConfig( const KSOGeneral *opts )
{
    servercontroller::self()->getGlobalAccel()->readSettings();
}

void PageShortcuts::defaultConfig()
{
    KSOGeneral opts;
    readConfig( &opts );
    m_key->allDefault();
}


void PageShortcuts::changed()
{
    emit modified();
}

#include "page_shortcuts.moc"
