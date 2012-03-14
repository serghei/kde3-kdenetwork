/*
 * soundpage.cpp - Sound settings for KTalkd
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

#include "soundpage.h"
#include <config.h>

#include <stdlib.h> //for setenv

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#include <klineedit.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kaudioplayer.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kurldrag.h>
#include <kdialog.h>

/* Lots of stuff taken from syssound.cpp */

KSoundPageConfig::KSoundPageConfig( QWidget *parent, const char* name,
	KSimpleConfig *_config, KSimpleConfig *_announceconfig)
    : KCModule (parent, name)
{
	if (!_config) {
		delete_config = true;
		config = new KSimpleConfig("ktalkdrc");
		announceconfig = new KSimpleConfig("");
	}
	else {
		delete_config = false;
		config = _config;
		announceconfig = _announceconfig;
	}

    QBoxLayout* toplay = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint() );

    QGroupBox* extprg_box = new QGroupBox(this);
    extprg_box->setColumnLayout( 0, Qt::Horizontal );
    toplay->addWidget(extprg_box);

    QGridLayout* l = new QGridLayout(extprg_box->layout());

    extprg_edit = new KURLRequester(extprg_box);
    l->addWidget(extprg_edit, 2, 4);

    extprg_label = new QLabel(extprg_edit,i18n("&Announcement program:"), extprg_box);
    l->addWidget(extprg_label, 2, 2);

    client_edit = new KURLRequester(extprg_box);
    l->addWidget(client_edit, 4, 4);

    client_label = new QLabel(client_edit,i18n("&Talk client:"), extprg_box);
    l->addWidget(client_label, 4, 2);

    toplay->addSpacing(10);

    sound_cb = new QCheckBox(i18n("&Play sound"), this);
    toplay->addWidget(sound_cb);

    QGroupBox* sound_box = new QGroupBox(this);
    toplay->addWidget(sound_box);

    QBoxLayout* lay = new QVBoxLayout(sound_box, 10, 10);

	int edit_h = client_edit->height(); // The height of a QLineEdit

    sound_list = new QListBox(sound_box);
    sound_list->setMinimumHeight( 3 * edit_h );
    sound_list->setAcceptDrops(true);
    sound_list->installEventFilter(this);

    sound_label = new QLabel(sound_list,i18n("&Sound file:"), sound_box);
    lay->addWidget(sound_label);

    QBoxLayout* l2 = new QHBoxLayout(lay, 10);
    l2->addWidget(sound_list);

    btn_test = new QPushButton(i18n("&Test"), sound_box);
    l2->addWidget(btn_test);

    sound_tip = new QLabel(
        i18n("Additional WAV files can be dropped onto the sound list."),
        sound_box);

    lay->addWidget(sound_tip);

    QStringList strlist( KGlobal::dirs()->findAllResources( "sound" ) );
    sound_list->insertStringList( strlist );

    load();

    connect(sound_cb, SIGNAL(clicked()), this, SLOT(soundOnOff()));
    connect(btn_test, SIGNAL(clicked()), this, SLOT(playCurrentSound()));

	// emit changed(true) on changes
	connect(extprg_edit->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotChanged()));
	connect(client_edit->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotChanged()));
}

KSoundPageConfig::~KSoundPageConfig( ) {
	if (delete_config) {
		delete config;
		delete announceconfig;
	}
    delete extprg_label;
    delete extprg_edit;
    delete client_label;
    delete client_edit;
    delete sound_cb;
    delete sound_label;
    delete sound_list;
    delete sound_tip;
    delete btn_test;

}

void KSoundPageConfig::slotChanged() {
	emit changed(true);
}

bool KSoundPageConfig::eventFilter(QObject* /*o*/, QEvent* e)
{
    if (e->type() == QEvent::DragEnter) {
        sound_listDragEnterEvent((QDragEnterEvent *) e);
        return true;
    }

    if (e->type() == QEvent::Drop) {
        sound_listDropEvent((QDropEvent *) e);
        return true;
    }

    return false;
}


void KSoundPageConfig::sound_listDragEnterEvent(QDragEnterEvent* e)
{
    e->accept(KURLDrag::canDecode(e));
}

void KSoundPageConfig::sound_listDropEvent(QDropEvent* e){

  KURL::List list;
  // This should never happen, but anyway...
  if(!KURLDrag::decode(e, list))
      return;

  // For now, we do only accept FILES ending with .wav...
  for( KURL::List::ConstIterator it = list.begin();
       it != list.end(); ++it)
  {
    const KURL &url = *it;

    if (!url.isLocalFile()) {      // for now, only file URLs are supported

      KMessageBox::sorry(this,
        i18n("This type of URL is currently unsupported "\
              "by the KDE system sound module."),
		i18n("Unsupported URL"));

    }
    else
    { // Now check for the ending ".wav"

      if (url.path().right(4).upper() != ".WAV") {
        QString msg = i18n("%1\ndoes not appear "\
                            "to be a WAV file.").arg(url.path());

        KMessageBox::sorry(this, msg, i18n("Improper File Extension"));

      }
      else
      {  // Hurra! Finally we've got a WAV file to add to the list

        if (!addToSound_List(url.path())) {
          // did not add file because it is already in the list
          QString msg = i18n("The file %1 is already in the list").arg(url.path());

          KMessageBox::information(this, msg, i18n("File Already in List"));

        }
      }
    }
  }
}

int KSoundPageConfig::findInSound_List(QString sound) {
// Searches for <sound> in sound_list. Returns position or -1 if not found

  bool found = false;

  int i = 0;
  int len = sound_list->count();

  while ((!found) && (i < len)) {

    found = sound == sound_list->text(i);
    i++;
  }
  return (found ? i-1 : -1);
}

bool KSoundPageConfig::addToSound_List(QString sound){
// Add "sound" to the sound list, but only if it is not already there

 bool found = (findInSound_List(sound) != -1);
 if (!found) {   // Fine, the sound is not already in the sound list!

   QString *tmp = new QString(sound); // take a copy...
   sound_list->insertItem(*tmp);
   sound_list->setTopItem(sound_list->count()-1);

   slotChanged();
 }

 return !found;
}

void KSoundPageConfig::playCurrentSound()
{
  QString hlp, sname;
  int soundno;

  soundno = sound_list->currentItem();
  if (soundno != -1) {
    sname = sound_list->text(soundno);
    if (sname[0] != '/')
      KAudioPlayer::play(locate("sound", sname));
    else
      KAudioPlayer::play(sname);
  }
}

void KSoundPageConfig::soundOnOff()
{
    bool b = sound_cb->isChecked();
    sound_label->setEnabled(b);
    sound_list->setEnabled(b);
    btn_test->setEnabled(b);
    sound_tip->setEnabled(b);

	slotChanged();
}

void KSoundPageConfig::defaults() {

    extprg_edit->lineEdit()->setText(KStandardDirs::findExe("ktalkdlg"));
    client_edit->lineEdit()->setText(KStandardDirs::findExe("konsole")+" -e talk");
       // will be ktalk when ktalk is in CVS.
    sound_cb->setChecked(true);

    // Activate things according to configuration
    soundOnOff();
}

void KSoundPageConfig::load() {

    config->setGroup("ktalkd");
    announceconfig->setGroup("ktalkannounce");

    setenv("KDEBINDIR",QFile::encodeName(KStandardDirs::kde_default("exe")),false/*don't overwrite*/);
           // for the first reading of  the config file

    extprg_edit->lineEdit()->setText(config->readPathEntry("ExtPrg",
	KStandardDirs::findExe("ktalkdlg")));
    client_edit->lineEdit()->setText(announceconfig->readPathEntry("talkprg",
	KStandardDirs::findExe("konsole")+" -e talk")); // will be ktalk when ktalk is in CVS

    bool b = announceconfig->readBoolEntry("Sound",true/*default value*/);
    sound_cb->setChecked(b);

    const QString soundFile = announceconfig->readPathEntry("SoundFile");
    if (!soundFile.isEmpty())
    {
        int pos = findInSound_List(soundFile);
        if (pos != -1) sound_list->setSelected(pos,true);
        else {
            addToSound_List(soundFile);
            sound_list->setSelected(sound_list->count()-1,true);
        }
    } else { sound_list->setSelected(0,true); }

    // Activate things according to configuration
    soundOnOff();

	emit changed(false);
}

void KSoundPageConfig::save() {

    config->setGroup("ktalkd");
    config->writePathEntry("ExtPrg", extprg_edit->lineEdit()->text());
    config->sync();
    announceconfig->setGroup("ktalkannounce");
    announceconfig->writePathEntry("talkprg", client_edit->lineEdit()->text());
    announceconfig->writeEntry("Sound", sound_cb->isChecked());
    announceconfig->writePathEntry("SoundFile",sound_list->text(sound_list->currentItem()));
    announceconfig->sync();
}

#include "soundpage.moc"
