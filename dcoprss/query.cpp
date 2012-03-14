/* $Id: query.cpp 280737 2004-01-18 11:35:09Z mueller $ */

/***************************************************************************
                          query.cpp  -  A query interface to select RSS feeds.
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
#include "cache.h"
#include "query.h"

#include <kdebug.h>
#include <krfcdate.h>

#include "service.h"
#include "xmlrpciface.h"

using KXMLRPC::Server;

void SlotCaller::call( QObject *object, const char *slot,
                       const KXMLRPC::Query::Result &result )
{
	SlotCaller caller;
	connect( &caller, SIGNAL( signal( const KXMLRPC::Query::Result &) ),
	         object, slot );
	emit caller.signal( result );
}

QueryService::QueryService( RSSService *service ) : QObject(), DCOPObject( "RSSQuery" ),
	m_service( service )
{
	m_xmlrpcServer = new KXMLRPC::Server( KURL( "http://www.syndic8.com/xmlrpc.php"), this );
}

QStringList QueryService::listActive()
{
	if ( !m_service )
		return QStringList();
	return m_service->list();
}

void QueryService::cachedCall( const QString &method,
                               const QValueList<QVariant> &args,
                               const char *slot )
{
	kdDebug() << "Calling " << method << endl;

	const QString cacheKey = Cache::getCacheKey( m_xmlrpcServer->url().url(),
	                                             method, args );

	CacheEntry *cacheEntry = Cache::self().find( cacheKey );
	if ( cacheEntry != 0 && cacheEntry->isValid() ) {
		kdDebug() << "Using cached result." << endl;
		SlotCaller::call( this, slot, cacheEntry->result() );
	} else {
		kdDebug() << "No cached result found, querying server." << endl;
		m_xmlrpcServer->call( method, args, this, slot );
	}
}

void QueryService::updateCache( const KXMLRPC::Query::Result &result )
{
	const QString cacheKey = Cache::getCacheKey( result.server(),
	                                             result.method(),
	                                             result.args() );

	CacheEntry *cacheEntry = Cache::self().find( cacheKey );
	if ( cacheEntry == 0 ) {
		kdDebug() << "Inserting returned result into cache." << endl;
		Cache::self().insert( cacheKey, result );
	}
}

void QueryService::findFeeds( const QString &query )
{
	kdDebug() << "QueryService::findFeeds()" << endl;

	QStringList args;
	args << query << "headlines_rank";

	cachedCall( "syndic8.FindFeeds", Server::toVariantList( args ),
	            SLOT( slotFoundFeeds( const KXMLRPC::Query::Result & ) ) );
}

void QueryService::findSites( const QString& query )
{
	kdDebug() << "QueryService::findSites()" << endl;

	cachedCall( "syndic8.FindSites", Server::toVariantList( query ),
	            SLOT( slotFoundFeeds( const KXMLRPC::Query::Result & ) ) );
}

void QueryService::getFeedInfo( const QVariant& ids )
{
	kdDebug() << "QueryService::getFeedInfo()" << endl;

	cachedCall( "syndic8.GetFeedInfo", Server::toVariantList( ids ),
	            SLOT( slotGotFeedInfo( const KXMLRPC::Query::Result & ) ) );
}

void QueryService::getCategories( const QString &category )
{
	kdDebug() << "QueryService::getCategories()" << endl;

	if ( category == "Top" ) {
		cachedCall( "syndic8.GetCategoryRoots", Server::toVariantList( QString::fromLatin1( "DMOZ" ) ),
		            SLOT( slotGotCategories( const KXMLRPC::Query::Result & ) ) );
	} else {
		QStringList args;
		args << "DMOZ" << category;
		cachedCall( "syndic8.GetCategoryChildren", Server::toVariantList( args ),
		            SLOT( slotGotCategories( const KXMLRPC::Query::Result & ) ) );
	}
}

void QueryService::getFeedsInCategory( const QString &category )
{
	kdDebug() << "QueryService::getFeedsInCategory()" << endl;

	QStringList args;
	args << "DMOZ" << category;

	cachedCall( "syndic8.GetFeedsInCategory", Server::toVariantList( args ),
	            SLOT( slotGotFeedsInCategory( const KXMLRPC::Query::Result & ) ) );
}

void QueryService::slotFoundFeeds( const KXMLRPC::Query::Result &result )
{
	kdDebug() << "QueryService::slotFoundFeeds()" << endl;
	if ( !result.success() ) {
		kdWarning() << "Failed to query for feeds: " << result.errorString() << endl;
		return;
	}

	updateCache( result );

	QValueList<int> ids;

	const QValueList<QVariant> values = result.data()[ 0 ].toList();
	QValueList<QVariant>::ConstIterator it = values.begin();
	QValueList<QVariant>::ConstIterator end = values.end();
	for ( ; it != end; ++it ) {
		ids << ( *it ).toInt();
		kdDebug() << "Found feed #" << ( *it ).toInt() << endl;
	}
	feedIds( ids );
}

void QueryService::slotGotFeedInfo( const KXMLRPC::Query::Result &result )
{
	kdDebug() << "QueryService::slotGotFeedInfo()" << endl;
	if ( !result.success() ) {
		kdWarning() << "Failed to get feed info: " << result.errorString() << endl;
		return;
	}

	updateCache( result );

	QMap<QString, QString> links;
	QValueList<RSSNewsFeed> feeds;

	const QValueList<QVariant> feedInfos = result.data();
	QValueList<QVariant>::ConstIterator it = feedInfos.begin();
	QValueList<QVariant>::ConstIterator end = feedInfos.end();
	for ( ; it != end; ++it ) {
		const QMap<QString, QVariant> feedInfo = ( *it ).toMap();

		const QString name = feedInfo[ "sitename" ].toString();
		const QString link = feedInfo[ "dataurl" ].toString();
		links[ name ] = link;

		RSSNewsFeed feed;
		feed.m_id = feedInfo[ "feedid" ].toUInt();
		feed.m_name = feedInfo[ "sitename" ].toString();
		feed.m_homePage = feedInfo[ "siteurl" ].toString();
		feed.m_sourceFile = feedInfo[ "dataurl" ].toString();
		feed.m_imageUrl = feedInfo[ "imageurl" ].toString();
		feed.m_webmaster = feedInfo[ "webmaster" ].toString();
		feed.m_editor = feedInfo[ "editor" ].toString();
		feed.m_publisher = feedInfo[ "publisher" ].toString();
		feed.m_creator = feedInfo[ "creator" ].toString();
		QDateTime dateTime;
		dateTime.setTime_t( KRFCDate::parseDate( feedInfo[ "date_created" ].toString() ) );
		feed.m_dateCreated = dateTime;
		dateTime.setTime_t( KRFCDate::parseDate( feedInfo[ "date_approved" ].toString() ) );
		feed.m_dateApproved = dateTime;
		dateTime.setTime_t( KRFCDate::parseDate( feedInfo[ "date_xml_changed" ].toString() ) );
		feed.m_dateXmlChanged = dateTime;
		feed.m_fetchable = feedInfo[ "fetchable" ].toBool();
		feed.m_description = feedInfo[ "description" ].toString();
		feed.m_origin = feedInfo[ "origin" ].toString();
		feed.m_languageCode = feedInfo[ "lang_code" ].toString();
		feed.m_status = feedInfo[ "status" ].toString();
		feed.m_version = feedInfo[ "rss_version" ].toString();
		feed.m_views = feedInfo[ "views" ].toUInt();
		feed.m_headlinesPerDay = feedInfo[ "headlines_per_day" ].toUInt();
		feed.m_headlinesRank = feedInfo[ "headlines_rank" ].toUInt();
		feed.m_toolkit = feedInfo[ "toolkit" ].toString();
		feed.m_toolkitVersion = feedInfo[ "toolkit_version" ].toString();
		feed.m_pollingInterval = feedInfo[ "cur_polling_interval" ].toUInt();
		dateTime.setTime_t( feedInfo[ "last_poll_time" ].toUInt() );
		feed.m_lastPoll = dateTime;
		// ### feed.m_categories missing here!

		feeds << feed;

		kdDebug() << "Retrieved data for newsfeed '" << name << "' <" << link << ">" << endl;
	}

	feedInfo( links );
	feedInfo( feeds );
}

void QueryService::slotGotCategories( const KXMLRPC::Query::Result &result )
{
	kdDebug() << "QueryService::slotGotCategories()" << endl;
	if ( !result.success() ) {
		kdWarning() << "Failed to get the list of categories: " << result.errorString() << endl;
		return;
	}

	updateCache( result );

	QStringList categories;

	const QValueList<QVariant> cats = result.data()[ 0 ].toList();
	QValueList<QVariant>::ConstIterator it = cats.begin();
	QValueList<QVariant>::ConstIterator end = cats.end();
	for (  ; it != end; ++it )
		categories << (  *it ).toString();

	kdDebug() << "Got categories: " << categories.join(  ", " ) << endl;
	gotCategories(  categories );

}

void QueryService::slotGotFeedsInCategory( const KXMLRPC::Query::Result &result )
{
	kdDebug() << "QueryService::slotGotFeedsInCategory()" << endl;
	if ( !result.success() ) {
		kdWarning() << "Failed to get the feeds in the given category: " << result.errorString() << endl;
		return;
	}

	updateCache( result );

	QValueList<int> ids;

	const QValueList<QVariant> values = result.data()[ 0 ].toList();
	QValueList<QVariant>::ConstIterator it = values.begin();
	QValueList<QVariant>::ConstIterator end = values.end();
	for (  ; it != end; ++it ) {
		ids << (  *it ).toInt();
		kdDebug() << "Got feed in category: #" << ( *it ).toInt() << endl;
	}

	gotFeedsInCategory(  ids );
}

#include "query.moc"
// vim:ts=4:sw=4:noet
