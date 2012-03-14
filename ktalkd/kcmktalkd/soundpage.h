/*
 * soundpage.h
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

#ifndef __KCONTROL_KSOUNDPAGE_H__
#define __KCONTROL_KSOUNDPAGE_H__

#include <qdir.h>
/* has to be before everything because of #define Unsorted 0 in X11/X.h !! */

#include <qobject.h>

#include <kcmodule.h>


class QDragMoveEvent;
class QDragEnterEvent;
class QDropEvent;

class KSimpleConfig;
class KURLRequester;
class QCheckBox;
class QLabel;
class QListBox;
class QPushButton;

class KSoundPageConfig : public KCModule
{
  Q_OBJECT

public:
    KSoundPageConfig( QWidget *parent=0, const char* name=0,
			KSimpleConfig *config=0, KSimpleConfig *announceconfig=0);
    ~KSoundPageConfig( );
	
    void load();
    void save();
    void defaults();

    bool eventFilter(QObject* o, QEvent* e);

private slots:
    void soundOnOff();
    void playCurrentSound();
    void slotChanged();

    // Sound DnD
    void sound_listDragEnterEvent(QDragEnterEvent* e);
    void sound_listDropEvent(QDropEvent* e);
 
private:
	KSimpleConfig *config;
	KSimpleConfig *announceconfig;
	bool delete_config;

    QLabel *extprg_label;
    KURLRequester *extprg_edit;
    QLabel *client_label;
    KURLRequester *client_edit;
    QCheckBox *sound_cb;
    QLabel *sound_label;
    QListBox *sound_list;
    QLabel *sound_tip;
    QPushButton *btn_test;

    int findInSound_List(QString sound);
    bool addToSound_List(QString sound);
};

#endif

