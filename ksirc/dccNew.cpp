/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qlistbox.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <klineedit.h>

#include "alistbox.h"
#include "dccNew.h"
#include "objFinder.h"

dccNew::dccNew( QWidget *parent, const char *name, int type, QString nick )
    : dccNewBase( parent, name)
{

    QColorGroup cg_mainw = kapp->palette().active();
    cg_mainw.setColor(QColorGroup::Base, ksopts->backgroundColor);
    cg_mainw.setColor(QColorGroup::Text, ksopts->textColor);
    cg_mainw.setColor(QColorGroup::Link, ksopts->linkColor);
    cg_mainw.setColor(QColorGroup::Highlight, ksopts->selBackgroundColor);
    cg_mainw.setColor(QColorGroup::HighlightedText, ksopts->selForegroundColor);
    nickList->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));

    QStringList allalist = objFinder::allObjects().grep(I18N_NOOP("aListBox::"));

    for ( QStringList::Iterator it = allalist.begin();
	  it != allalist.end();
	  ++it ) {
	QString name = (*it).section("::", 1);
	kdDebug(5008) << "Looking at: " << *it << "/" << name << endl;

	aListBox *a = static_cast<aListBox *>(objFinder::find(name.latin1(), "aListBox"));
	if(a){
	    QListBoxItem *i;
	    for(i = a->firstItem(); i != 0x0; i = i->next()){
		nickListItem *it = new nickListItem(*a->item(a->index(i)));
		nickList->inSort(it);
	    }
	}
	else {
	    kdDebug(5008) << "Didn't find: " << name << endl;
	}
    }

    KCompletion *comp = cbNicks->completionObject();

    QListBoxItem *i;
    for(i = nickList->firstItem(); i != 0x0; i = i->next()){
        comp->addItem(i->text());
        cbNicks->insertItem(i->text());
    }
    cbNicks->setCurrentText(nick);

    KConfig *kConfig = kapp->config();
    kConfig->setGroup("dccNew");

    bool chatChecked = kConfig->readBoolEntry("chatChecked", false);

    /*
     * allow type to override
     * the config setting
     */
    if(type == Chat){
        chatChecked = true;
    }
    else if(type == Send){
        chatChecked = false;
    }

    if(chatChecked) {
	rbChat->setChecked(true);
	chatClicked();
    }
    else {
	rbFileSend->setChecked(true);
	fileSendClicked();
    }

    connect(nickList, SIGNAL(highlighted(const QString &)),
	    cbNicks, SLOT(setEditText(const QString &)));

    connect(pbCancel, SIGNAL(clicked()),
	    this, SLOT(reject()));

    connect(pbSend, SIGNAL(clicked()),
            this, SLOT(accept()));


}

dccNew::~dccNew()
{
}


void dccNew::chatClicked()
{
    gbFile->setEnabled(false);
}

void dccNew::fileSendClicked()
{
    gbFile->setEnabled(true);
}

void dccNew::sendClicked()
{
    KConfig *kConfig = kapp->config();
    kConfig->setGroup("dccNew");
    kConfig->writeEntry("chatChecked",rbChat->isChecked());
    int type = Chat;
    if(rbFileSend->isChecked())
        type = Send;
    emit accepted(type, cbNicks->currentText(), leFile->text());
}

void dccNew::fileClicked()
{
    QString file =
	KFileDialog::getOpenFileName();

    leFile->setText(file);
}

QString dccNew::getFile() {
    return leFile->text() ;
}

QString dccNew::getNick() {
    return cbNicks->currentText();
}

int dccNew::getType() {
    int type = Chat; if(rbFileSend->isChecked()) type = Send;
    return type;
}

void dccNew::reject()
{
    emit accepted(-1, QString::null, QString::null);
}


#include "dccNew.moc"
