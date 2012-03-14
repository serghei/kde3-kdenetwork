/*
  main.cpp - The KControl module for ktalkd

  Copyright (C) 1998 by David Faure, faure@kde.org
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   
  */


#include "main.h"
#include "soundpage.h"
#include "answmachpage.h"
#include "forwmachpage.h"
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <qtabwidget.h>
#include <qlayout.h>

KTalkdConfigModule::KTalkdConfigModule(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    config = new KSimpleConfig("ktalkdrc");
    announceconfig = new KSimpleConfig("ktalkannouncerc");

	QVBoxLayout *layout = new QVBoxLayout(this);
    
	tab = new QTabWidget(this);
    
	layout->addWidget(tab);

	soundpage = new KSoundPageConfig(this, "soundpage", config, announceconfig);
	answmachpage = new KAnswmachPageConfig(this, "answmachpage", config);
	forwmachpage = new KForwmachPageConfig(this, "forwmachpage", config);

	tab->addTab(soundpage, i18n("&Announcement"));
	tab->addTab(answmachpage, i18n("Ans&wering Machine"));
	tab->addTab(forwmachpage, i18n("forward call", "&Forward"));
	    
	connect(soundpage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	connect(answmachpage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	connect(forwmachpage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	    }
	    
KTalkdConfigModule::~KTalkdConfigModule() {
	delete config;
	delete announceconfig;
	}

void KTalkdConfigModule::defaults() 
{
    if (soundpage) soundpage->defaults();
    if (answmachpage) answmachpage->defaults();    
    if (forwmachpage) forwmachpage->defaults();    
}

void KTalkdConfigModule::save()
{
    if (soundpage) soundpage->save();
    if (answmachpage) answmachpage->save();
    if (forwmachpage) forwmachpage->save();
}

void KTalkdConfigModule::load()
{
    if (soundpage) soundpage->load();
    if (answmachpage) answmachpage->load();
    if (forwmachpage) forwmachpage->load();
}

void KTalkdConfigModule::resizeEvent(QResizeEvent *)
{
	tab->setGeometry(0,0,width(),height());
}

extern "C"
{
	KDE_EXPORT KCModule *create_ktalkd(QWidget *parent, const char *)
	{
		return new KTalkdConfigModule(parent, "kcmktalkd");
}

	KDE_EXPORT KCModule *create_ktalkd_answmach(QWidget *parent, const char *)
{
		return new KAnswmachPageConfig(parent, "kcmktalkd");
	}

	KDE_EXPORT KCModule *create_ktalkd_sound(QWidget *parent, const char *)
	{
		return new KSoundPageConfig(parent, "kcmktalkd");
	}

	KDE_EXPORT KCModule *create_ktalkd_forwmach(QWidget *parent, const char *)
	{
		return new KForwmachPageConfig(parent, "kcmktalkd");
	}
}

#include "main.moc"

