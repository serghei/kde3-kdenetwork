/*
 * answmachpage.h
 *
 * Copyright (c) 1998 David Faure
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

#ifndef __KCONTROL_KANSWMACHPAGE_H__
#define __KCONTROL_KANSWMACHPAGE_H__

#include <qdir.h>
/* has to be before everything because of #define Unsorted 0 in X11/X.h !! */

#include <qobject.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qbutton.h>
#include <qcheckbox.h> 

#include <kcmodule.h>

class KSimpleConfig;

class KAnswmachPageConfig : public KCModule
{
  Q_OBJECT

public:
	KAnswmachPageConfig( QWidget *parent=0, const char* name=0, 
	                     KSimpleConfig *config=0 );
    ~KAnswmachPageConfig( );
	
    void load();
    void save();
    void defaults();

protected:
    void  resizeEvent(QResizeEvent *e);

private slots:
    void answmachOnOff();
    void slotChanged();
 
private:
	KSimpleConfig *config;
	bool delete_config;

    QCheckBox *answmach_cb;
    QLabel *mail_label;
    QLineEdit *mail_edit;
    QLabel *subj_label;
    QLineEdit *subj_edit;
    QLabel *subj_tip;
    QLabel *head_label;
    QLineEdit *head_edit;
    QLabel *head_tip;
    QCheckBox *emptymail_cb;
    QLabel *msg_label;
    QMultiLineEdit *msg_ml;

    QString *msg_default;
};

#endif

