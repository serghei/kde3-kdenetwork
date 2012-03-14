/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "page_autoconnect.h"


#include <qregexp.h>
#include <klocale.h>
#include <klistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <knuminput.h>

#define NAME 0
#define PK   1
#define PASS 2
#define SSL  3

PageAutoConnect::PageAutoConnect( QWidget *parent, const char *name ) : PageAutoConnectBase( parent, name)
{
    KLVAutoConnect->setSorting( 0 );
    //KLVAutoConnect->header()->hide();

    AddPB->setEnabled(FALSE);
    DeletePB->setEnabled(FALSE);
    PortKI->setValue(6667);
}

PageAutoConnect::~PageAutoConnect()
{
}

void PageAutoConnect::saveConfig()
{

    KConfig *conf = kapp->config();
    conf->setGroup("AutoConnect");

    QStringList servers;

    for(QListViewItem *it = KLVAutoConnect->firstChild();
        it != 0;
        it = it->nextSibling()){

	QString server = it->text(NAME);

	if(it->text(PK).length() > 0)
	    server += ":" + it->text(1);
	if(it->text(SSL).length() > 0)
            server += " (SSL)";
	if(it->text(PASS).length() > 0)
	    server += QString(" (pass: %1)").arg(it->text(PASS));

        servers << server;

        QStringList channels;

        for(QListViewItem *ch = it->firstChild();
            ch != 0;
	    ch = ch->nextSibling()){
	    QString channel;

	    channel = ch->text(NAME);

	    if(ch->text(PK).length() > 0)
		channel += QString(" (key: %1)").arg(ch->text(PK));

	    channels << channel;

        }

        channels.sort();
        conf->writeEntry(server, channels);

    }

    servers.sort();

    conf->writeEntry("Servers", servers);

}

void PageAutoConnect::readConfig()
{
    KConfig *conf = kapp->config();
    conf->setGroup("AutoConnect");
    QStringList servers = conf->readListEntry("Servers");
    servers.sort();
    QStringList::ConstIterator ser = servers.begin();
    for( ; ser != servers.end(); ser++){
        QStringList channels = conf->readListEntry(*ser);
	QString server = *ser;
	QString port = "6667";
	QString ssl = QString::null;
	QString pass = QString::null;

	QRegExp rx("(.+) \\(SSL\\)(.*)");
	if(rx.search(server) >= 0){
            server = rx.cap(1) + rx.cap(3);
            ssl = i18n("SSL");
	}
	rx.setPattern("(.+) \\(pass: (\\S+)\\)(.*)");
	if(rx.search(server) >= 0){
            server = rx.cap(1) + rx.cap(3);
            pass = rx.cap(2);
	}
	rx.setPattern("([^: ]+):(\\d+)");
	if(rx.search(server) >= 0){
            kdDebug(5008) << server << ": Has port:" << rx.cap(2)  << endl;
            server = rx.cap(1);
            port = rx.cap(2);
	}
	kdDebug(5008) << server << ": Done " << port << " " << ssl << " " << pass << endl;
        QListViewItem *s = new QListViewItem(KLVAutoConnect, server, port, pass, ssl);
        s->setOpen(TRUE);
        channels.sort();
        QStringList::ConstIterator chan = channels.begin();
	for(; chan != channels.end(); chan++){
            QString channel = *chan;
            QString key = QString::null;
            QRegExp crx("(.+) \\(key: (\\S+)\\)");
            if(crx.search(channel) >= 0){
		channel = crx.cap(1);
		key = crx.cap(2);
            }
            new QListViewItem(s, channel, key);
        }
    }
}

void PageAutoConnect::defaultConfig()
{
    KLVAutoConnect->clear();
    readConfig( );
}

void PageAutoConnect::changed()
{
    emit modified();
}

void PageAutoConnect::add_pressed()
{
    int fnd = 0;
    QListViewItem *s = 0;

    s = KLVAutoConnect->selectedItem();
    if(!s){ /* new item */
	QString server = ServerLE->text();
	QString ssl = QString::null;
	QString port;

	port.setNum(PortKI->value());
	if(sslCB->isChecked())
	    ssl = i18n("SSL");

	s = new QListViewItem(KLVAutoConnect, server, port, PassLE->text(), ssl);
	s->setOpen(TRUE);
	s = new QListViewItem(s, ChannelLE->text(), KeyLE->text());
        KLVAutoConnect->setCurrentItem(s);
    }
    else { /* update the existing one */
	QListViewItem *parent;
	QListViewItem *child;

	if(s->parent()){
	    parent = s->parent();
            child = s;
	}
	else {
	    parent = s;
            child = 0x0;
	}

	parent->setText(NAME, ServerLE->text());
	parent->setText(PK, QString("%1").arg(PortKI->value()));
	parent->setText(PASS, PassLE->text());
	if(sslCB->isChecked())
	    parent->setText(SSL, i18n("SSL"));
	else
	    parent->setText(SSL, QString::null);

	if(child){
	    child->setText(NAME, ChannelLE->text());
	    child->setText(PK, KeyLE->text());
	}
	else {
	    if(ChannelLE->text().length() > 0){
                fnd = 0;
		QListViewItem *c = parent->firstChild();
		for( ; c != 0 && fnd == 0; c = c->nextSibling()){
		    if(c->text(NAME) == ChannelLE->text()){
			c->setText(PK, KeyLE->text());
			fnd = 1;
		    }
		}
		if(fnd == 0){
		    new QListViewItem(parent, ChannelLE->text(), KeyLE->text());
		}
	    }
	}

    }
    changed();
}

void PageAutoConnect::new_pressed()
{
    AddPB->setText(i18n("&Add"));
    ServerLE->clear();
    ChannelLE->clear();
    sslCB->setChecked(false);
    PassLE->clear();
    KeyLE->clear();
    KLVAutoConnect->clearSelection();
    PortKI->setValue(6667) ;
}

void PageAutoConnect::delete_pressed()
{
    for(QListViewItem *it = KLVAutoConnect->firstChild();
        it != 0;
        it = it->nextSibling()){

        if(it->text(NAME) == ServerLE->text()){
            if(ChannelLE->text().isEmpty() == FALSE){
                for(QListViewItem *ch = it->firstChild();
                    ch != 0;
                    ch = ch->nextSibling()){
                    if(ch->text(NAME) == ChannelLE->text()){
                        delete ch;
                        changed();
                        ChannelLE->clear();
                        ServerLE->clear();
                        return;
                    }
                }
            }
            else {
                delete it;
                changed();
                ServerLE->clear();
                return;
            }
        }
    }
    changed();
}

void PageAutoConnect::kvl_clicked(QListViewItem *it)
{
    if(it != 0){
	if(it->parent() != 0){
	    ChannelLE->setText(it->text(NAME));
	    KeyLE->setText(it->text(PK));
	    AddPB->setText(i18n("&Update"));
	    /*
	     * Move it to the parent to setup parent/server
	     * values.  This save writing this code
	     * in two places.
	     */
	    it = it->parent();
	}
	else {
	    AddPB->setText(i18n("&Update/Add"));
	    ChannelLE->clear();
	    KeyLE->clear();
	}

	if(it->parent() == 0){
	    ServerLE->setText(it->text(NAME));
	    PortKI->setValue(it->text(PK).toInt());
            PassLE->setText(it->text(PASS));
	    if(it->text(SSL).length() > 0)
		sslCB->setChecked(true);
	    else
		sslCB->setChecked(false);
	}
	/*
	 * Make sure to do this after changing all the fields
         */
	AddPB->setEnabled(false);
        DeletePB->setEnabled(true);


    }
    else {
	AddPB->setEnabled(false);
	DeletePB->setEnabled(false);
    }
    changed();
}

void PageAutoConnect::item_changed() {
    AddPB->setEnabled(true);
}

#include "page_autoconnect.moc"

