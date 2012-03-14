/* -------------------------------------------------------------

   main.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias H�zer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

 ------------------------------------------------------------- */

#include <config.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdelibs_export.h>

#include "application.h"
#include "toplevel.h"

static KCmdLineOptions knoptions[] =
{
  { "c", 0, 0 },
  { "clipboard", I18N_NOOP("Define X11-clipboard content (selected text)"), 0 },
  { "+[word/phrase]", I18N_NOOP("Lookup the given word/phrase"), 0 },
  KCmdLineLastOption
};


extern "C" KDE_EXPORT int kdemain(int argc, char* argv[])
{
  KAboutData aboutData("kdict",
                        I18N_NOOP("Dictionary"),
                        KDICT_VERSION,
                        I18N_NOOP("The KDE Dictionary Client"),
                        KAboutData:: License_Artistic,
                        "Copyright (c) 1999-2001, Christian Gebauer\nCopyright (c) 1998, Matthias Hoelzer",
                        0,
                        0);

  aboutData.addAuthor("Christian Gebauer",I18N_NOOP("Maintainer"),"gebauer@kde.org");
  aboutData.addAuthor("Matthias Hoelzer",I18N_NOOP("Original Author"),"hoelzer@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( knoptions );
  KUniqueApplication::addCmdLineOptions();

  if (!Application::start())
    return 0;

  Application app;

  return app.exec();
}
