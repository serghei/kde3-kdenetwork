/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.
 *                                                                         *
 ***************************************************************************/

#include "page_rmbmenu.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <kdebug.h>

#include "../usercontrolmenu.h"



PageRMBMenu::PageRMBMenu( QWidget *parent, const char *name ) : PageRMBMenuBase( parent, name)
{
    UserControlMenu *ucm;

    UserControlMenu::parseKConfig();

    commandLB->clear();
    for(ucm = UserControlMenu::UserMenu.first();
	ucm != 0;
	ucm = UserControlMenu::UserMenu.next()){

	if(ucm->type == UserControlMenu::Seperator){
            commandLB->insertItem("--------------", -1);
	}
	else{
	    commandLB->insertItem(ucm->title, -1);
	}
    }

    changeItemPB->hide();

    connect(commandLB, SIGNAL(highlighted( int )),
            this, SLOT(highlighted( int )));
    connect(moveUpPB, SIGNAL(clicked()),
            this, SLOT(moveDown()));
    connect(moveDownPB, SIGNAL(clicked()),
            this, SLOT(moveUp()));

    connect(insertSeperatorPB, SIGNAL(clicked()),
            this, SLOT(insSeperator()));

    connect(insertItemPB, SIGNAL(clicked()),
            this, SLOT(insCommand()));

    connect(deleteItemPB, SIGNAL(clicked()),
            this, SLOT(delCommand()));

}

PageRMBMenu::~PageRMBMenu()
{
}

void PageRMBMenu::saveConfig()
{
    UserControlMenu::writeKConfig();
}

void PageRMBMenu::readConfig( const KSORMBMenu * )
{
}

void PageRMBMenu::defaultConfig()
{
}

void PageRMBMenu::highlighted(int index)
{
    UserControlMenu *ucm;

    ucm = UserControlMenu::UserMenu.at(index);

    if(ucm == 0)
        return;

    if(ucm->type == UserControlMenu::Seperator){
	entryLE->setEnabled(false);
	commandLE->setEnabled(false);
	opEnableCB->setEnabled(false);
	opEnableCB->setChecked(false);
	changeItemPB->setEnabled(false);
    }
    else{
	entryLE->setEnabled(true);
	commandLE->setEnabled(true);
	opEnableCB->setEnabled(true);
	opEnableCB->setChecked(true);
	changeItemPB->setEnabled(true);

	entryLE->setText(ucm->title);
	commandLE->setText(ucm->action);
	opEnableCB->setChecked(ucm->op_only);
    }

    if(index == 0){
	moveUpPB->setEnabled(true);
	moveDownPB->setEnabled(false);
    }
    else if((uint)index == (commandLB->count()-1)){
	moveUpPB->setEnabled(false);
	moveDownPB->setEnabled(true);
    }
    else {
	moveUpPB->setEnabled(true);
	moveDownPB->setEnabled(true);
    }
}

void PageRMBMenu::moveUp()
{
    int item = commandLB->currentItem();

    QString txt = commandLB->text(item);
    commandLB->removeItem(item);

    commandLB->insertItem(txt, item-1);
    commandLB->setCurrentItem(item-1);

    UserControlMenu *ucm = UserControlMenu::UserMenu.take(item);
    UserControlMenu::UserMenu.insert(item-1,ucm);

    highlighted(item-1);
    emit modified();
}

void PageRMBMenu::moveDown()
{
    int item = commandLB->currentItem();

    QString txt = commandLB->text(item);
    commandLB->removeItem(item);

    commandLB->insertItem(txt, item+1);
    commandLB->setCurrentItem(item+1);

    UserControlMenu *ucm = UserControlMenu::UserMenu.take(item);
    UserControlMenu::UserMenu.insert(item+1,ucm);

    highlighted(item+1);
    emit modified();
}

void PageRMBMenu::insSeperator()
{
    int item = commandLB->currentItem();

    QString txt = commandLB->text(item);

    commandLB->insertItem("--------------", item);
    commandLB->setCurrentItem(item);

    UserControlMenu::UserMenu.insert(item,new UserControlMenu); // Defaults to a separator

    highlighted(item);
    emit modified();
}

void PageRMBMenu::insCommand()
{
    int item = commandLB->currentItem();

    QString te = entryLE->text();
    QString ce = commandLE->text();

    commandLB->insertItem(te, item);
    commandLB->setCurrentItem(item);

    UserControlMenu::UserMenu.insert(item,
				     new UserControlMenu(
							 te,
							 ce,
							 0x0,
							 UserControlMenu::Text
							)); // Defaults to a separator

    highlighted(item);
    emit modified();
}

void PageRMBMenu::delCommand()
{
    int item = commandLB->currentItem();

    QString txt = commandLB->text(item);
    commandLB->removeItem(item);

    UserControlMenu::UserMenu.remove(item);

    highlighted(item);
    emit modified();
}


#include "page_rmbmenu.moc"
