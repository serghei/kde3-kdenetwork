/*
 * answmachpage.cpp - Answering machine settings for KTalkd
 *
 * Copyright (C) 1998 David Faure, faure@kde.org
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "answmachpage.h"


#include <ksimpleconfig.h>

#include <stdlib.h>
#include <klocale.h> // for getenv

KAnswmachPageConfig::KAnswmachPageConfig( QWidget *parent, const char* name, 
	KSimpleConfig *_config) 
    : KCModule (parent, name)
{
	if (!_config) {
		delete_config = true;
		config = new KSimpleConfig("ktalkdrc");
	}
	else {
		delete_config = false;
		config = _config;
	}
		
    answmach_cb = new QCheckBox(i18n("&Activate answering machine"), this);
    answmach_cb->adjustSize();
    mail_edit = new QLineEdit(this);
    mail_edit->adjustSize();
    mail_edit->setMinimumWidth(150);
    mail_label = new QLabel(mail_edit,i18n("&Mail address:"),this);
    mail_label->adjustSize();
    mail_label->setAlignment( ShowPrefix | AlignVCenter );

    subj_edit = new QLineEdit(this);
    subj_edit->adjustSize();
    subj_edit->setMinimumWidth(150);
    subj_label = new QLabel(subj_edit, i18n("Mail s&ubject:"),this);
    subj_label->adjustSize();
    subj_label->setAlignment( ShowPrefix | AlignVCenter );
    subj_tip = new QLabel(i18n("Use %s for the caller name"),this);
    subj_tip->setAlignment( ShowPrefix );

    head_edit = new QLineEdit(this);
    head_edit->adjustSize();
    head_edit->setMinimumWidth(150);
    head_label = new QLabel(head_edit, i18n("Mail &first line:"),this);
    head_label->adjustSize();
    head_label->setAlignment( ShowPrefix | AlignVCenter );
    head_tip = new QLabel(
        i18n("Use first %s for caller name, and second %s for caller hostname"),
        this);
    head_tip->setAlignment( ShowPrefix );

    emptymail_cb = new QCheckBox(i18n("&Receive a mail even if no message left"), this);
    emptymail_cb->adjustSize();

    msg_ml = new QMultiLineEdit(this);
    msg_ml->adjustSize();
    msg_ml->setMinimumWidth(150);
    msg_label = new QLabel(msg_ml, i18n("&Banner displayed on answering machine startup:"),this);
    msg_label->adjustSize();
    msg_label->setAlignment( ShowPrefix | AlignVCenter );

    int h = 10 + answmach_cb->height() + mail_edit->height() +
        subj_edit->height() + subj_tip->height() + head_edit->height()
        + head_tip->height() + emptymail_cb->height() +
        msg_label->height() + msg_ml->height() + 30;
    setMinimumSize(400, h); // 400 : otherwise, buttons may overlap
    msg_default = new QString(i18n("The person you are asking to talk with is not answering.\n"
"Please leave a message to be delivered via email.\n"
"Just start typing and when you have finished, exit normally."));

    load();

    connect(answmach_cb, SIGNAL(clicked()), this, SLOT(answmachOnOff()));

	// Emit changed(true) when anything changes
	connect(answmach_cb, SIGNAL(clicked()), this, SLOT(slotChanged()));
	connect(mail_edit, SIGNAL(textChanged(const QString&)), 
			this, SLOT(slotChanged()));
	connect(subj_edit, SIGNAL(textChanged(const QString&)), 
			this, SLOT(slotChanged()));
	connect(head_edit, SIGNAL(textChanged(const QString&)), 
			this, SLOT(slotChanged()));
	connect(emptymail_cb, SIGNAL(clicked()), this, SLOT(slotChanged()));
	connect(msg_ml, SIGNAL(textChanged()), this, SLOT(slotChanged()));
	
}

KAnswmachPageConfig::~KAnswmachPageConfig( ) {
	if (delete_config) delete config;
    delete answmach_cb;
    delete mail_label;
    delete mail_edit;
    delete subj_label;
    delete subj_edit;
    delete subj_tip;
    delete head_label;
    delete head_edit;
    delete head_tip;
    delete emptymail_cb;
    delete msg_label;
    delete msg_ml;
    delete msg_default;
}

void KAnswmachPageConfig::slotChanged() {
	emit changed(true);
}

void KAnswmachPageConfig::resizeEvent(QResizeEvent *) {

    int h_txt = answmach_cb->height(); // taken for the general label height
    int h_edt = mail_edit->height();   // taken for the general QLineEdit height
    int spc = h_txt / 3;
    int w = rect().width();

    int leftedits = mail_label->width();
    if ( subj_label->width() > leftedits )
      leftedits = subj_label->width();
    if ( head_label->width() > leftedits )
      leftedits = head_label->width();
    leftedits += 20;

    int h = 10 + spc*2;
    answmach_cb->move(10, h);
    h += h_txt+spc;
    mail_label->setFixedHeight(h_edt); // same size -> vertical center aligned
    mail_label->move(10, h);
    mail_edit->setGeometry(leftedits, h, w-leftedits-10, h_edt);
    h += h_edt+spc;

    subj_label->setFixedHeight(h_edt); // same size -> vertical center aligned
    subj_label->move(10, h);
    subj_edit->setGeometry(leftedits, h, w-leftedits-10, h_edt);
    h += h_edt+spc;
    subj_tip->setFixedWidth(w-20);
    subj_tip->move(leftedits, h);
    h += h_txt+spc;

    head_label->setFixedHeight(h_edt); // same size -> vertical center aligned
    head_label->move(10, h);
    head_edit->setGeometry(leftedits, h, w-leftedits-10, h_edt);
    h += h_edt+spc;
    head_tip->setFixedWidth(w-20);
    head_tip->move(leftedits, h);
    h += h_txt+spc;

    emptymail_cb->move(10,h);
    h += h_txt+spc;

    msg_label->move(10, h);
    h += h_txt+spc;

    msg_ml->setGeometry(10, h, w-20, height() - h - 10);
    h += h_edt+spc;
}

void KAnswmachPageConfig::answmachOnOff()
{
    bool b = answmach_cb->isChecked();
    mail_label->setEnabled(b);
    mail_edit->setEnabled(b);
    subj_label->setEnabled(b);
    subj_edit->setEnabled(b);
    subj_tip->setEnabled(b);
    head_label->setEnabled(b);
    head_edit->setEnabled(b);
    head_tip->setEnabled(b);
    emptymail_cb->setEnabled(b);
    msg_label->setEnabled(b);
    msg_ml->setEnabled(b);
}

void KAnswmachPageConfig::defaults() {

    answmach_cb->setChecked(true);

    mail_edit->setText(getenv("REPLYTO"));
    subj_edit->setText(i18n("Message from %s"));
    head_edit->setText(i18n("Message left on the answering machine, by %s@%s"));

    emptymail_cb->setChecked(true);
    msg_ml->setText(*msg_default);

    // Activate things according to configuration
    answmachOnOff();

}

void KAnswmachPageConfig::load() {
   
    config->setGroup("ktalkd");

    answmach_cb->setChecked(config->readBoolEntry("Answmach",true));

    mail_edit->setText(config->readEntry("Mail",getenv("REPLYTO")));
    subj_edit->setText(config->readEntry("Subj",i18n("Message from %s")));
    head_edit->setText(config->readEntry("Head",
           i18n("Message left on the answering machine, by %s@%s")));

    emptymail_cb->setChecked(config->readBoolEntry("EmptyMail",true));

    msg_ml->clear();
    char m[]="Msg1"; // used as key to read configuration
    QString msg;
    while (!(msg=config->readEntry(m)).isNull())
    {
        msg_ml->append(msg);
        ++m[3];
    }

    if (m[3]=='1') // nothing in the config file
        msg_ml->setText(*msg_default);

    // Activate things according to configuration
    answmachOnOff();

	emit changed(false);
}

void KAnswmachPageConfig::save() {

    config->setGroup("ktalkd");
    config->writeEntry("Answmach", answmach_cb->isChecked());
    config->writeEntry("Mail",mail_edit->text());
    config->writeEntry("Subj",subj_edit->text());
    config->writeEntry("Head",head_edit->text());
    config->writeEntry("EmptyMail", emptymail_cb->isChecked());
    char m[]="Msg1"; // used as key to read configuration
    int linenr=0;
    QString msg;
    while ((linenr<8) && (linenr<msg_ml->numLines()))
    {
        config->writeEntry(m,msg_ml->textLine(linenr));
        ++m[3]; ++linenr;
    }
    config->deleteEntry(m,false/*non localized*/);
    // necessary to avoid old msg lines to reappear after
    // deleting some. 
    
    config->sync();
}

#include "answmachpage.moc"
