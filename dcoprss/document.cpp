/* $Id: document.cpp 371819 2004-12-19 04:28:15Z geiseri $ */
/***************************************************************************
                          document.cpp  -  A DCOP Service to provide RSS data
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
#include <qdatetime.h>
#include <kurl.h>
#include "service.h"

RSSDocument::RSSDocument(const QString& url) :
	QObject(), DCOPObject(), m_Url(url)
{

	m_list.setAutoDelete( true );
	m_Doc = 0L;
	m_pix = QPixmap();
	m_isLoading = false;
	m_maxAge = 60;
	m_Timeout = QDateTime::currentDateTime();
	m_state.clear();
}

RSSDocument::~RSSDocument()
{
	kdDebug() << "Document going away..." << endl;

	delete m_Doc;
}

void RSSDocument::loadingComplete(Loader *ldr, Document doc, Status stat)
{


	if( m_Doc != 0L)
	{
		delete m_Doc;

	}

	if (stat != RSS::Success)
	{
		kdDebug() << "Document error! Loader:" << ldr->errorCode() << " Parser:" << stat << endl;

		m_isLoading = false;
		m_Doc = 0L;
		if( stat == RSS::ParseError )
			documentUpdateError(DCOPRef(this), 1);
		else if( stat == RSS::RetrieveError )
			documentUpdateError(DCOPRef(this), 2);
		else
			documentUpdateError(DCOPRef(this), 3);
	}
	else
	{
		kdDebug() << "New Document is done..." << endl;
		m_Doc = new Document(doc);
		m_list.clear();
		Article::List list = doc.articles();
		for(Article::List::ConstIterator it = list.begin(); it != list.end(); ++it)
		{
			int state = m_state[(*it).title()];
			if( state == 0 ) m_state[(*it).title()] = 1; // new
			else if( state == 1 ) m_state[(*it).title()] = 2; // old message now
			m_list.append( new RSSArticle( new Article(*it)));
		}
		Image *img = m_Doc->image();
		if ( img )
		{
			connect(img, SIGNAL(gotPixmap(const QPixmap &)),
				SLOT(pixmapLoaded(const QPixmap &)));
			img->getPixmap();
			pixmapUpdating(DCOPRef(this));
		}
		m_isLoading = false;
		documentUpdated(DCOPRef(this));

		kdDebug() << "Old Mod time " << m_Timeout.toString() << endl;
		m_Timeout = m_Timeout.addSecs(m_maxAge * 60 );
		kdDebug() << "New Mod time " << m_Timeout.toString() << endl;

	}
}

void RSSDocument::pixmapLoaded(const QPixmap &pix )
{
	m_pix = pix;
	pixmapUpdated(DCOPRef(this));
}

QString RSSDocument::webMaster()
{
	if( m_Doc != 0L)
		return m_Doc->webMaster();
	else
		return "";
}

QString RSSDocument::managingEditor()
{
	if( m_Doc != 0L)
			return m_Doc->managingEditor();
	else
		return "";
}

QString RSSDocument::rating()
{
	if( m_Doc != 0L)
		return m_Doc->rating();
	else
		return "";
}

QDateTime RSSDocument::lastBuildDate()
{
	if( m_Doc != 0L)
		return m_Doc->lastBuildDate();
	else
		return QDateTime::currentDateTime();
}

QDateTime RSSDocument::pubDate()
{
	if( m_Doc != 0L)
		return m_Doc->pubDate();
	else
		return QDateTime::currentDateTime();
}

QString RSSDocument::copyright()
{
	if( m_Doc != 0L)
		return m_Doc->copyright();
	else
		return "";
}

QStringList RSSDocument::articles()
{
	if( m_Doc != 0L)
	{
		kdDebug() << "Document giving articles..." << endl;
		Article::List list = m_Doc->articles();
		QStringList stringList;

		for(Article::List::ConstIterator it = list.begin(); it != list.end(); ++it)
				stringList.append((*it).title());
		return stringList;
	}
	else
		return QStringList();
}

DCOPRef RSSDocument::article(int idx)
{
	if(m_list.at(idx))
		return DCOPRef(m_list.at(idx));
	else
		return DCOPRef();
}

int RSSDocument::count()
{
	if( m_Doc != 0L)
		return m_Doc->articles().count();
	return 0;
}

QString RSSDocument::link()
{
	if( m_Doc != 0L)
		return m_Doc->link().prettyURL();
	else
		return "";
}

QString RSSDocument::description()
{
	if( m_Doc != 0L)
		return m_Doc->description();
	else
		return "";
}

QString RSSDocument::title()
{
	if( m_Doc != 0L)
		return m_Doc->title();
	else
		return "";
}

QString RSSDocument::verbVersion()
{
	if( m_Doc != 0L)
		return m_Doc->verbVersion();
	else
		return "";
}

QString RSSDocument::pixmapURL()
{
	if( m_Doc != 0L)
		if( m_Doc->image() )
			return m_Doc->image()->url().prettyURL();
		else
			return "";
	else
		return "";
}

QPixmap RSSDocument::pixmap()
{
	return m_pix;
}

bool RSSDocument::documentValid()
{
	if (m_Doc != 0L)
		return true;
	else
		return false;
}

bool RSSDocument::pixmapValid()
{
	return !m_pix.isNull();
}

void RSSDocument::refresh()
{
	kdDebug() << "Mod time " << m_Timeout.toString() << endl;
	kdDebug() << "Current time " << QDateTime::currentDateTime().toString() << endl;

	if(!m_isLoading  && (QDateTime::currentDateTime() >= m_Timeout))
	{
		kdDebug() << "Document going to refresh" << endl;
		m_isLoading = true;
		Loader *loader = Loader::create(this,
			SLOT(loadingComplete(Loader *, Document, Status)));
		loader->loadFrom(KURL( m_Url ),  new FileRetriever());
		documentUpdating(DCOPRef(this));
	}
	else
	{
		documentUpdated(DCOPRef(this));
		if(pixmapValid())
			pixmapUpdated(DCOPRef(this));
		/*
		else
		{
			// Refactor this!
			Image *img = m_Doc->image();
			if ( img )
			{
				connect(img, SIGNAL(gotPixmap(const QPixmap &)),
					SLOT(pixmapLoaded(const QPixmap &)));
				img->getPixmap();
				pixmapUpdating(DCOPRef(this));
			}
		}
		*/
	}

}

int RSSDocument::maxAge()
{
	return m_maxAge;
}

void RSSDocument::setMaxAge(int _min)
{
	m_Timeout.addSecs(-m_maxAge);
	m_maxAge = _min;
	m_Timeout.addSecs(m_maxAge);
}

int RSSDocument::state( const QString &title) const
{
	return m_state[title];
}

void RSSDocument::setState( const QString &title, int s )
{
	m_state[title] = s;
}

void RSSDocument::read( const QString &title)
{
	m_state[title] = 3;
}

#include "service.moc"
