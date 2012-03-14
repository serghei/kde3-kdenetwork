/* -------------------------------------------------------------

   application.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

 ------------------------------------------------------------- */

#include <kwin.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include "application.h"
#include "toplevel.h"


Application::Application()
 : KUniqueApplication()
{
  m_mainWindow = new TopLevel( 0, "mainWindow" );
}


Application::~Application()
{
  delete m_mainWindow;
}


int Application::newInstance()
{
  kdDebug(5004) << "Application::newInstance()" << endl;
  KUniqueApplication::newInstance();

  // process parameters...
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  m_mainWindow->show();

  if (args->isSet("clipboard"))
  {
    m_mainWindow->defineClipboard();
  }
  else
  {
    if (args->count()>0)
    {
      QString phrase;
      for (int i=0;i<args->count();i++)
      {
        phrase += QString::fromLocal8Bit(args->arg(i));
        if (i+1 < args->count())
          phrase += " ";
      }
      m_mainWindow->define(phrase);
    }
    else
    {
      m_mainWindow->normalStartup();
    }
  }

  return 0;
}

//--------------------------------

#include "application.moc"
