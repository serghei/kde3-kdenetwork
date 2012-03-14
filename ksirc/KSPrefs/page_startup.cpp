/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <keditlistbox.h>

#include <kdebug.h>

#include "page_startup.h"

PageStartup::PageStartup( QWidget *parent, const char *name ) : PageStartupBase( parent, name)
{
    notifyLB->upButton()->hide();
    notifyLB->downButton()->hide();
    serverLB->upButton()->hide();
    serverLB->downButton()->hide();

    QListBox *lb = serverLB->listBox();
    connect(lb, SIGNAL(highlighted(int)),
	    this, SLOT(clickedLB(int)));

    changing = false;

}

PageStartup::~PageStartup()
{
}

void PageStartup::saveConfig()
{
    KSOServer glb = ksopts->server["global"];
    QStringList items = serverLB->items();

    ksopts->server.clear();

    QStringList::iterator it = items.begin();
    for( ; it != items.end(); ++it){
        ksopts->server[*it] = server[*it];
    }
    if(!ksopts->server.contains("global")){
        ksopts->server["global"] = glb;
    }

}

void PageStartup::readConfig( const KSOptions *opts )
{
    server = opts->server;

    changing = true;
    ServerOpMap::Iterator it;
    for ( it = server.begin(); it != server.end(); ++it ) {
        if(it.data().globalCopy == false)
	    serverLB->insertItem(it.key());
    }
    QListBoxItem *item = serverLB->listBox()->findItem("global");
    serverLB->listBox()->setSelected(item, true);
    changing = false;
    clickedLB(serverLB->listBox()->index(item));

}

void PageStartup::defaultConfig()
{
    KSOptions opts;
    readConfig( &opts );
}

void PageStartup::changed()
{
    emit modified();

    QString ser = serverLB->currentText();
    if(ser.isEmpty())
	return;

    if(changing)
        return;

    kdDebug(5008) << "got changed for: " << ser <<endl;

    server[ser].nick = nickLE->text();
    server[ser].altNick = altNickLE->text();
    server[ser].realName = rnLE->text();
    server[ser].userID = uiLE->text();

    server[ser].notifyList.clear();
    for ( int i = 0; i < notifyLB->count(); ++i)
	server[ser].notifyList.append( notifyLB->text( i ) );
    server[ser].globalCopy = false;

}

void PageStartup::clickedLB(int index)
{

    QString text = serverLB->text(index);
    if(!server.contains(text)){
	server[text] = server["global"];
        server[text].globalCopy = true;
    }

    changing = true;
    notifyLB->clear();

    nickLE->setText( server[text].nick );
    altNickLE->setText( server[text].altNick );
    rnLE->setText( server[text].realName );
    uiLE->setText( server[text].userID );
    notifyLB->insertStringList( server[text].notifyList );
    changing = false;
}

#include "page_startup.moc"
