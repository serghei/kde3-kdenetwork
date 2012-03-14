/* $Id: service.cpp 253735 2003-09-24 22:56:36Z tokoe $ */
/***************************************************************************
                          service.cpp  -  A DCOP Service to provide RSS data
                             -------------------
    begin                : Saturday 15 February 2003
    copyright            : (C) 2003 by Ian Reinhart Geiser
    email                : geiseri@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include "service.h"
#include "cache.h"

RSSService::RSSService() :
	DCOPObject("RSSService")
{
	m_list.setAutoDelete( true );

	loadLinks();
}

RSSService::~RSSService()
{
}


QStringList RSSService::list()
{
	QStringList lst;
	QDictIterator<RSSDocument> itr(m_list);
	for(; itr.current(); ++itr)
		lst.append(itr.currentKey());
	return lst;
}

DCOPRef RSSService::add(QString id)
{
	if(m_list.find(id) == 0L) { // add a new one only if we need to
		m_list.insert(id, new RSSDocument(id));
		added(id);
		saveLinks();
        }
	return document(id);
}

void RSSService::remove(QString id)
{
	m_list.remove(id);
	removed(id);
	saveLinks();
}

DCOPRef RSSService::document(QString id)
{
	if( m_list[id] )
		return DCOPRef(m_list[id]);
	else
		return DCOPRef();
}

void RSSService::exit()
{
	//Save all current RSS links.
	saveLinks();
	Cache::self().save();
	kapp->quit();
}


void RSSService::loadLinks()
{
	KConfig *conf = kapp->config();
	conf->setGroup("RSS Links");	
	const QStringList links = conf->readListEntry ("links"); 
	QStringList::ConstIterator it = links.begin();
	QStringList::ConstIterator end = links.end();
	for ( ; it != end; ++it )
		add( *it );
}

void RSSService::saveLinks()
{
	KConfig *conf = kapp->config();
	conf->setGroup("RSS Links");	
        QStringList lst;
        QDictIterator<RSSDocument> itr(m_list);
        for(; itr.current(); ++itr)
                lst.append(itr.currentKey());

	conf->writeEntry("links", lst);	
	conf->sync();
}



