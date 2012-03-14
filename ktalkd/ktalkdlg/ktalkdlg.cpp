/* This file is part of the KDE project
   Copyright (C) 1998, 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <sys/time.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <qmessagebox.h>
#include <qfile.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include <kaudioplayer.h>
#include <klocale.h>

#define RING_WAIT 30
#define MAX_DLG_LIFE RING_WAIT
/* so that the dialog lasts exactly the time of an announce */

class TimeoutDialog : public QMessageBox {
  public:
    TimeoutDialog (int timeout_ms,
                 const QString& caption, const QString &text, Icon icon,
                 int button0, int button1, int button2,
                 QWidget *parent=0, const char *name=0, bool modal=TRUE,
                 WFlags f=WStyle_DialogBorder  ):
            QMessageBox (caption, text, icon, button0, button1, button2,
                     parent, name, modal, f)
            {startTimer (timeout_ms);}

    ~TimeoutDialog ()
            {killTimers ();}

    virtual void timerEvent (QTimerEvent *)
            {killTimers (); done (Rejected);}
};

static KCmdLineOptions option[] =
{
   { "+user@host", I18N_NOOP("Caller identification"), 0 },
   { "+[callee]", I18N_NOOP("Name of the callee, if he doesn't exist on this system (we're taking his call)"), 0 },
   KCmdLineLastOption
};

static const char description[] =
        I18N_NOOP("Dialog box for incoming talk requests");

static const char version[] = "v1.5.2";

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "ktalkdlg", description, version );
    KCmdLineArgs::addCmdLineOptions( option );
    KLocale::setMainCatalogue( "kcmktalkd" );
    KApplication a;

    struct timeval clock;
    struct timezone zone;
    gettimeofday (&clock, &zone);
    struct tm *localclock = localtime ((const time_t *) &clock.tv_sec);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count() == 0)
       KCmdLineArgs::usage(i18n("'user@host' expected."));

    QString s;
    s.sprintf ("%d:%02d", localclock->tm_hour, localclock->tm_min);
    s = i18n ("Message from talk demon at ") + s + " ...\n" +
        i18n ("Talk connection requested by ") + args->arg(0);

    if ( args->count() == 2 )
    {
      s += '\n';
      QString callee = args->arg(1);
      s += i18n ("for user %1").arg( callee.isEmpty() ? i18n("<nobody>") : callee );
    }

    s += ".";

    TimeoutDialog dialog (MAX_DLG_LIFE * 1000,
                          i18n ("Talk requested..."), s,
                          QMessageBox::Information,
                          QMessageBox::Yes | QMessageBox::Default,
                          QMessageBox::No | QMessageBox::Escape,
                          0 );
    dialog.setButtonText( QMessageBox::Yes, i18n ("Respond") );
    dialog.setButtonText( QMessageBox::No, i18n ("Ignore") );

    a.setTopWidget (&dialog);

    // don't erase this! - ktalkd waits for it!
    printf("#\n");
    fflush(stdout);

    KConfig *cfg = new KConfig ( "ktalkannouncerc" );
    cfg->setGroup ("ktalkannounce");
    bool bSound = cfg->readNumEntry ("Sound", 0);

    if (bSound) {
      QString soundFile = cfg->readPathEntry ("SoundFile");
      if (soundFile[0] != '/')
        soundFile = locate( "sound", soundFile );

      if (!soundFile.isEmpty ()) {
         KAudioPlayer::play (soundFile);
      }
    }
    //if (!audio) a.beep ();  // If no audio is played (whatever reason), beep!

    int result = dialog.exec ();
    if (result == QMessageBox::Yes) {
	dialog.killTimers ();
        kdDebug() << "Running talk client..." << endl;

        QString konsole = locate("exe", "konsole");
        QString konsole_dir = konsole;
        konsole_dir.truncate( konsole.findRev('/') );
        setenv("KDEBINDIR", QFile::encodeName(konsole_dir).data(), 0/*don't overwrite*/);
        QString cmd0 = cfg->readPathEntry ("talkprg", konsole + " -e talk");

	QString cmd = cmd0.stripWhiteSpace();
	cmd += " '";
        cmd += args->arg(0);
        cmd += "' &";

        kdDebug() << cmd << endl;

        // Open /dev/null for stdin, stdout and stderr:
        int fd=open("/dev/null", O_RDWR);
        for (int i = 0; i <= 2; i++) {
            dup2(fd, i);
        }

	/* XXX: The sender's name or hostname may contain `rm -rf .`
	 * 	That's why it's bad to use system()
	 */
        system (QFile::encodeName(cmd));
        kapp->quit();
    }

    return 0;
}
