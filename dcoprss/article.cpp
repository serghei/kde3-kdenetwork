/* $Id: article.cpp 227737 2003-05-25 10:46:40Z mcamen $ */
/***************************************************************************
                          article.cpp  -  A DCOP Service to provide RSS data
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
#include <kurl.h>
#include "service.h"

RSSArticle::RSSArticle(Article *art) :
	DCOPObject(), m_Art(art)
{
	kdDebug() << "New article..." << endl;
	kdDebug() << m_Art->link().prettyURL() << endl;
}

RSSArticle::~RSSArticle()
{
	kdDebug() << "Article going away..." << endl;
	delete m_Art;
}

QString RSSArticle::title()
{
	//kdDebug() << "Get title " <<  m_Art->title() << endl;
	return m_Art->title();
}

QString RSSArticle::description()
{
	return m_Art->description();
}

QString RSSArticle::link()
{
	return m_Art->link().prettyURL();
}
