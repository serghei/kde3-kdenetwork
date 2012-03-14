/* This file is part of the KDE project
   Copyright (C) 2003 Andrew Stanley-Jones <asj-kde@cban.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/

#include "dccManager.h"
#include "dccNew.h"

#include <qobject.h>
#include <qsignal.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpushbutton.h>

#define COL_FILE 1
#define COL_WHO  0
#define COL_STAT 2
#define COL_SIZE 3
#define COL_CPS  4
#define COL_PER  5

dccItem::dccItem( KListView *parent, dccManager *manager, enum dccType type, const QString &file, const QString& who, enum dccStatus status, unsigned int size )
    : QObject(), KListViewItem(parent), m_who(who), m_file(file), m_type(type)
{
    m_percent = 0;
    m_status = status;
    m_size = size;
    m_stime = 0;
    m_lasttime = 0;
    m_manager = manager;

    setText(COL_FILE, file);
    setText(COL_WHO, who);
    setText(COL_STAT, enumToStatus(status));
    if(m_type == dccChat)
        setText(COL_SIZE, "");
    else
        setText(COL_SIZE, QString("%1").arg(size));
    setText(COL_PER, "");

}

dccItem::dccItem( KListViewItem *parent, dccManager *manager, enum dccType type, const QString &file, const QString& who, enum dccStatus status, unsigned int size )
: QObject(), KListViewItem(parent), m_who(who), m_file(file), m_type(type)
{
    m_percent = 0;
    m_status = status;
    m_size = size;
    m_stime = 0;
    m_manager = manager;

    setText(COL_FILE, file);
    setText(COL_WHO, who);
    setText(COL_STAT, enumToStatus(status));
    if(type != dccChat)
	setText(COL_SIZE, QString("%1").arg(size));
    setText(COL_PER, "");

}

dccItem::~dccItem()
{
}

QString dccItem::enumToStatus(enum dccStatus status)
{
    QString str;
    switch(status){
    case dccRecving:
        str = i18n("Receiving");
        break;
    case dccGotOffer:
        str = i18n("Got Offer");
	break;
    case dccSentOffer:
        str = i18n("Sent Offer");
	break;
    case dccWaitOnResume:
	str = i18n("Resume Requested");
        break;
    case dccResumed:
        str = i18n("Did Resume");
        break;
    case dccSending:
        str = i18n("Sending");
        break;
    case dccOpen:
        str = i18n("dcc status", "Open");
        break;
    case dccDone:
        str = i18n("Done");
        break;
    case dccCancel:
        str = i18n("Canceled");
        break;
    case dccError:
        str = i18n("Error");
        break;
    default:
        str = i18n("Unknown State");
        break;
    }
    return str;
}

void dccItem::setWhoPostfix(const QString &post) {
    m_post = post;
    setText(COL_WHO, QString("%1 %2").arg(m_who).arg(post));
}

void dccItem::changeFilename(const QString &file) {
    setText(COL_FILE, file);
    m_file = file;
}

void dccItem::changeWho(const QString &who) {
    setText(COL_WHO, who);
    m_who = who;
}

void dccItem::changeStatus(enum dccStatus status)
{
    m_manager->doChanged();
    setText(COL_STAT, enumToStatus(status));
    m_status = status;
    emit statusChanged(this);
}

void dccItem::setReceivedBytes(int bytes)
{
    int per;
    time_t ctime = time(NULL);

    if(m_stime == 0)
        m_stime = ctime-1;

    if(m_size)
        per = (100*bytes)/m_size;
    else
        per = 100;

    if((per != (int)m_percent) ||
       (ctime >= (m_lasttime + 2))
      ){
        m_lasttime = ctime;
        setText(COL_SIZE, QString("%1/%2").arg(bytes).arg(m_size));
        setText(COL_PER, QString("%1%").arg(per));
	m_percent = per;
	if(m_status == dccResumed)
            m_stime = 0; /* if we are got a resume request don't update CPS, reset it */
        else
	    setText(COL_CPS, QString("%1").arg(1.0*bytes/(time(NULL) - m_stime), 2));
    }


}

void dccItem::doRename()
{

    if(type() == dccGet){
	setRenameEnabled(COL_FILE, true);
	startRename(COL_FILE);
    }
    else if(type() == dccChat){
	setText(COL_WHO, m_who);
	setRenameEnabled(COL_WHO, true);
	startRename(COL_WHO);
    }

}

void dccItem::okRename(int col)
{
    KListViewItem::okRename(col);
    if(type() == dccGet){
	QString oldfile = m_file;
	changeFilename(text(COL_FILE));
	emit itemRenamed(this, m_who, oldfile);
	setRenameEnabled(COL_FILE, false);
    }
    else if(type() == dccChat){
	QString oldwho = m_who;
	changeWho(text(COL_WHO));
	emit itemRenamed(this, oldwho, m_file);
	setRenameEnabled(COL_WHO, false);
        setWhoPostfix(m_post);
    }
}

void dccItem::cancelRename(int col)
{
    KListViewItem::cancelRename(col);
    if(type() == dccChat){
	setWhoPostfix(m_post);
    }
}

dccManager::dccManager( QWidget *parent, const char *name ) : dccManagerbase( parent, name)
{
    dccNewDialog = 0x0;

    m_getit = new KListViewItem(klvBox, i18n("Get"));
    m_sendit = new KListViewItem(klvBox, i18n("Send"));
    m_chatit = new KListViewItem(klvBox, i18n("Chat"));

    m_getit->setOpen(true);
    m_sendit->setOpen(true);
    m_chatit->setOpen(true);

    m_getit->setSelectable(false);
    m_sendit->setSelectable(false);
    m_chatit->setSelectable(false);

//    connect(klvBox, SIGNAL(clicked(QListViewItem *)),
//	    this,   SLOT(getSelChange(QListViewItem *)));
    connect(klvBox, SIGNAL(currentChanged(QListViewItem *)),
	    this,   SLOT(getSelChange(QListViewItem *)));

    klvBox->setCurrentItem(m_chatit);
    getSelChange(klvBox->currentItem());
}

dccManager::~dccManager()
{
}

dccItem *dccManager::newSendItem(QString file, QString who, enum dccItem::dccStatus status, unsigned int size)
{
    emit changed(false, i18n("dcc activity"));
    dccItem *it = new dccItem(m_sendit, this, dccItem::dccSend, file, who, status, size);
    connect(it,   SIGNAL(statusChanged(QListViewItem *)),
            this, SLOT(getSelChange(QListViewItem *)));
    return it;
}

dccItem *dccManager::newGetItem(QString file, QString who, enum dccItem::dccStatus status, unsigned int size)
{
    emit changed(false, i18n("dcc activity"));
    dccItem *it = new dccItem(m_getit, this, dccItem::dccGet, file, who, status, size);
    connect(it,   SIGNAL(statusChanged(QListViewItem *)),
	    this, SLOT(getSelChange(QListViewItem *)));
    return it;

}

dccItem *dccManager::newChatItem(QString who, enum dccItem::dccStatus status)
{
    emit changed(false, i18n("dcc activity"));
    dccItem *it = new dccItem(m_chatit, this, dccItem::dccChat, "", who, status, 0);
    connect(it,   SIGNAL(statusChanged(QListViewItem *)),
	    this, SLOT(getSelChange(QListViewItem *)));
    return it;

}

void dccManager::kpbNew_clicked()
{
    if(dccNewDialog){
	dccNewDialog->show();
	dccNewDialog->raise();
        return;
    }

    dccNewDialog = new dccNew();
    dccNewDialog->show();

    connect(dccNewDialog, SIGNAL(accepted(int, QString, QString)),
	    this, SLOT(dccNewAccepted(int, QString, QString)));

}

void dccManager::dccNewAccepted(int type, QString nick, QString file){
    kdDebug(5008) << "Type: " << type << " nick: " << nick << " file: " << file << endl;
    if(type == dccNew::Chat){
	QCString cstr = QCString("/dcc chat ") + nick.latin1() + "\n";
	kdDebug(5008) << "Output: " << cstr << endl;
        emit outputLine(cstr);
    }
    else if(type == dccNew::Send){
	QCString cstr = QCString("/dcc send ") + nick.latin1() + " " + file.latin1() + "\n";
	emit outputLine(cstr);
    }
    delete dccNewDialog;
    dccNewDialog = 0x0;
}

void dccManager::kpbConnect_clicked()
{
    dccItem *it = dynamic_cast<dccItem *>(klvBox->currentItem());
    if(it){
        emit dccConnectClicked(it);
    }

}
void dccManager::kpbResume_clicked()
{
    dccItem *it = dynamic_cast<dccItem *>(klvBox->currentItem());
    if(it){
        emit dccResumeClicked(it);
    }
}
void dccManager::kpbRename_clicked()
{
    dccItem *it = dynamic_cast<dccItem *>(klvBox->currentItem());
    if(it){
        emit dccRenameClicked(it);
    }
}
void dccManager::kpbAbort_clicked()
{
    dccItem *it = dynamic_cast<dccItem *>(klvBox->currentItem());
    if(it){
        emit dccAbortClicked(it);
    }
}

void dccManager::getSelChange(QListViewItem *_i) {
    kpbAbort->setEnabled(false);
    kpbResume->setEnabled(false);
    kpbRename->setEnabled(false);
    kpbConnect->setEnabled(false);

    if(_i == 0)
        return;
    dccItem *it = dynamic_cast<dccItem *>(_i);
    if(!it)
        return;

    /*
     * Everything is off by default
     * We need to turn on what we want
     */
    switch(it->type()){
    case dccItem::dccChat:
        kdDebug(5008) << "is a chat" << endl;
        kpbAbort->setEnabled(true);
	switch(it->status()){
	case dccItem::dccGotOffer:
	    kpbConnect->setEnabled(true);
	    break;
	case dccItem::dccOpen:
	    kpbRename->setEnabled(true);
	    break;
	default:
            break;
	}
	break;
    case dccItem::dccGet:
	kdDebug(5008) << "is a get" << endl;
	kpbAbort->setEnabled(true);
	switch(it->status()){
	case dccItem::dccGotOffer:
	case dccItem::dccWaitOnResume:
	case dccItem::dccResumed:
	    kpbResume->setEnabled(true);
	    kpbConnect->setEnabled(true);
	    kpbRename->setEnabled(true);
	    break;
	default:
	    break;
	}
	break;
    case dccItem::dccSend:
	kdDebug(5008) << "is a send" << endl;
	kpbAbort->setEnabled(true);
	break;
    default:
	break;
    }


}
void dccManager::sendSelChange(QListViewItem *) {
    /*
    if(_i == 0)
        return;
    dccItem *it = static_cast<dccItem *>(_i);

    kdDebug(5008) << "got: " << it->who() << " file: " << it->file() << endl;

    switch(it->status()){
    case dccItem::dccSending:
    case dccItem::dccRecving:
        kpbSendStop->setText(i18n("Abort"));
        break;
    case dccItem::dccOffer:
        kpbSendStop->setText(i18n("Forget"));
        break;
    case dccItem::dccDone:
        kpbSendStop->setText(i18n("Clear"));
        break;
        }
        */

}


#include "dccManager.moc"

