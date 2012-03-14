/*
 * knewstickerstub.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <dcopclient.h>


#include "configaccess.h"

static const char name[] = "knewstickerstub";
static const char verbname[] = I18N_NOOP("KNewsTickerStub");
static const char version[] = "0.3";
static const char description[] = I18N_NOOP("A frontend to the KNewsTicker configuration");
static const char copyright[] = I18N_NOOP("(c)2000, 2001 Frerich Raabe");

static const KCmdLineOptions options[] =
{
	{"a", 0, 0},
	{"addrdf <url>", I18N_NOOP("Add the RDF/RSS file referenced by <url>"), 0},
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KLocale::setMainCatalogue("knewsticker");

	KAboutData aboutData(name, verbname, version, description,
			KAboutData::License_BSD, copyright);
	aboutData.addAuthor("Frerich Raabe", I18N_NOOP("Author"), "raabe@kde.org");

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	
	KApplication app(false, false);

	DCOPClient *dcopClient = app.dcopClient();
	dcopClient->attach();

	KConfig cfg(QString::fromLatin1("knewsticker_panelappletrc"), false, false);
	ConfigAccess configFrontend(&cfg);

	if (args->isSet("addrdf")) {
		QStringList newsSources = configFrontend.newsSources();

		// TODO: Use the "Suggest" functionality from addnewssourcedlgimpl.cpp here.
		QString name = i18n("Unknown");
		if (newsSources.contains(name))
			for (unsigned int i = 0; ; i++)
				if (!newsSources.contains(i18n("Unknown %1").arg(i))) {
						name = i18n("Unknown %1").arg(i);
						break;
				}

		newsSources += name;

		QString sourceFile = QString(args->getOption("addrdf"));
		configFrontend.setNewsSource(NewsSourceBase::Data(name, sourceFile));
		configFrontend.setNewsSources(newsSources);

		QByteArray data;
		dcopClient->send("knewsticker", "KNewsTicker", "reparseConfig()", data);
	}

	args->clear();

	return 0;
}
