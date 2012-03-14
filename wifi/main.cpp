/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sam Apr  7 11:44:20 CEST 2001
    copyright            : (C) 2001 by Stefan Winter
    email                : mail@stefan-winter.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>

#include "kwifimanager.h"
#ifndef VERSION
#define VERSION "1_2_0"
#endif

static const char description[] =
I18N_NOOP ("KWiFiManager - Wireless LAN Manager for KDE");


static KCmdLineOptions options[] = {
  KCmdLineLastOption
};

int
main (int argc, char *argv[])
{
  KAboutData aboutData ("kwifimanager", I18N_NOOP ("KWiFiManager"), VERSION,
			description, KAboutData::License_GPL,
			"(c) 2001-05, Stefan Winter");
  aboutData.addAuthor ("Stefan Winter", I18N_NOOP("Original Author and Maintainer"), "swinter@kde.org");
  aboutData.addCredit ("Helge Deller", I18N_NOOP("Lots of Fixes and Optimizations, added Session Management"),
			"deller@kde.org");
  KCmdLineArgs::init (argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions (options);	// Add our own options.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs ();
  args->clear ();

  KApplication app;

  if (app.isRestored ())
    {
      RESTORE (KWiFiManagerApp(0,"mainWindow"));
      KWiFiManagerApp *kwifimanager = (KWiFiManagerApp *) app.mainWidget();
      if (kwifimanager && kwifimanager->startDocked())
	kwifimanager->hide();
    }
  else
    {
      KWiFiManagerApp *kwifimanager = new KWiFiManagerApp (0,"mainWindow");
      kwifimanager->show();
    }

  return app.exec ();
}
