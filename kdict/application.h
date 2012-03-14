/* -------------------------------------------------------------

   application.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

 ------------------------------------------------------------- */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <kuniqueapplication.h>
#include <qguardedptr.h>

#define KDICT_VERSION "0.6"

class TopLevel;

class Application : public KUniqueApplication
{
  Q_OBJECT

  public:
    Application();
    ~Application();

    /** Create new instance of Kdict. Make the existing
        main window active if Kdict is already running */
    int newInstance();

  private:
    QGuardedPtr<TopLevel> m_mainWindow;

};

#endif
