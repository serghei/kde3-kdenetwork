/*************************************************************************

 Main KSirc start

 $$Id: ksirc.cpp 364679 2004-11-21 03:08:10Z mueller $$

 Main start file that defines 3 global vars, etc

*************************************************************************/

/*
 * Needed items
 * 4. Send a /quit and/or kill dsirc on exit
 * */

#include <stdlib.h>

#include <qsessionmanager.h>

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

//#include "cdate.h"
#include "ksopts.h"
#include "servercontroller.h"
#include "version.h"

static const char description[] =
        I18N_NOOP("KDE IRC client");

//QDict<KSircTopLevel> TopList;
//QDict<KSircMessageReceiver> TopList;

class KCmdLineOptions options[] =
{
    { "nick <nickname>", I18N_NOOP( "Nickname to use" ), 0 } ,
    { "server <server>", I18N_NOOP( "Server to connect to on startup" ), 0 },
    { "channel <#channel>", I18N_NOOP( "Channel to connect to on startup" ), 0 },
    { "o", 0, 0 },
    { "noautoconnect", I18N_NOOP( "Do not autoconnect on startup" ), 0 },
    KCmdLineLastOption
};

class KSircSessionManaged : public KSessionManaged
{
public:
    KSircSessionManaged() {}

    virtual bool commitData( QSessionManager &sm )
    {
        servercontroller *controller = servercontroller::self();
        if ( !controller || !sm.allowsInteraction() ) return true;

        // if the controller is hidden KMWSessionManaged won't send the fake close event.
        // we want it in any way however.
        if ( controller->isHidden() ) {
            QCloseEvent e;
            QApplication::sendEvent( controller, &e );
        }

        return true;
    }
};

extern "C" KDE_EXPORT int kdemain( int argc, char ** argv )
{
    KAboutData aboutData( "ksirc", I18N_NOOP("KSirc"),
        KSIRC_VERSION, description, KAboutData::License_Artistic,
        I18N_NOOP("(c) 1997-2002, The KSirc Developers"));
    aboutData.addAuthor("Andrew Stanley-Jones",I18N_NOOP("Original Author"), "asj-ksirc@cban.com");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");
    aboutData.addAuthor("Carsten Pfeiffer",0, "pfeiffer@kde.org");
    aboutData.addAuthor("Malte Starostik",0, "malte@kde.org");
    aboutData.addAuthor("Daniel Molkentin",0, "molkentin@kde.org");
    aboutData.addAuthor("Simon Hausmann",0, "hausmann@kde.org");
    aboutData.addAuthor("Alyssa Mejawohld", I18N_NOOP("Icons Author"), "amejawohld@bellsouth.net");
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) {
          exit(0);
    }

    // Start the KDE application
    KUniqueApplication app;

    KSircSessionManaged sm;

    // Options
    KSOptions opts;
    opts.load();

    servercontroller *sc = new servercontroller(0, "servercontroller");
    app.setMainWidget(sc);

    if (KMainWindow::canBeRestored(1))
    {
        sc->restore(1, false );
    }
    else
    { // no Session management -> care about docking, geometry, etc.

        if( !opts.geometry.isEmpty() )
            sc->setGeometry( opts.geometry );

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        QCString nickName = args->getOption( "nick" );
        QCString server = args->getOption( "server" );
        QCString channel = args->getOption( "channel" );

        if ( !nickName.isEmpty() )
            ksopts->server["global"].nick = nickName;

	if ( !server.isEmpty() ) {
	    QString ser = QString::fromLocal8Bit( server );
            QString port = "6667";
	    if(ser.contains(":")){
		port = ser.section(":", 1, 1);
		ser = ser.section(":", 0, 0);
	    }
	    KSircServer kss(ser, port, "", "", false);
            sc->new_ksircprocess( kss  );
            if ( !channel.isEmpty() ) {
                QStringList channels = QStringList::split( ',', QString::fromLocal8Bit( channel ) );
                QStringList::ConstIterator it = channels.begin();
                QStringList::ConstIterator end = channels.end();
		for ( ; it != end; ++it ) {
		    sc->new_toplevel( KSircChannel(ser, *it), true );
		}
            }
        }
        else {
            if(args->isSet("autoconnect") == TRUE){
                sc->start_autoconnect();
            }
        }

        args->clear();
    }

    return app.exec();
}

/* vim: et sw=4
 */
