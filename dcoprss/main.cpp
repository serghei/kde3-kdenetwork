/* $Id: main.cpp 368546 2004-12-05 00:02:52Z dfaure $ */

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>
#include "service.h"
#include "query.h"

int main (int argc, char *argv[])
{
	KLocale::setMainCatalogue("dcoprss");
	KAboutData aboutdata("rssservice", I18N_NOOP("KDE RSS Service"),
				"0.8", I18N_NOOP("A RSS data service."),
				KAboutData::License_GPL, "(C) 2003, Ian Reinhart Geiser");
	aboutdata.addAuthor("Ian Reinhart Geiser",I18N_NOOP("Developer"),"geiseri@kde.org");

	KCmdLineArgs::init( argc, argv, &aboutdata );
	// KCmdLineArgs::addCmdLineOptions( options );
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start())
	{
		kdDebug() << "rssservice is already running!" << endl;
		return (0);
	}

	KUniqueApplication app;
	kdDebug() << "starting rssservice " << endl;
	// This app is started automatically, no need for session management
	app.disableSessionManagement();
	RSSService *service = new RSSService;
	QueryService *query = new QueryService(service);

	return app.exec();
}
