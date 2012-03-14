/*
 * main.cpp for lisa,reslisa,kio_lan and kio_rlan kcm module
 *
 *  Copyright (C) 2000,2001,2005 Alexander Neundorf <neundorf@kde.org>
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

#include <klocale.h>
#include <kdebug.h>

#include <qdir.h>
#include <qdatetime.h>

#include <unistd.h>
#include <sys/types.h>

#include "kcmlisa.h"
#include "kcmreslisa.h"
#include "kcmkiolan.h"

extern "C"
{
  KDE_EXPORT KCModule *create_lisa(QWidget *parent, const char *)
  {
    return new LisaSettings("/etc/lisarc", parent);
  }
  KDE_EXPORT KCModule *create_reslisa(QWidget *parent, const char *)
  {
    return new ResLisaSettings(QDir::homeDirPath()+"/.reslisarc", parent);
  }
  KDE_EXPORT KCModule *create_kiolan(QWidget *parent, const char *)
  {
    return new IOSlaveSettings("kio_lanrc", parent);
  }
}

