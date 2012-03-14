/* $Id: query.h 222876 2003-04-30 04:14:55Z raabe $ */
#ifndef _QUERY_SERVICE
#define _QUERY_SERVICE

/***************************************************************************
                          query.h  -  A query interface to select RSS feeds.
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

#include "rssnewsfeed.h"
#include "xmlrpciface.h"

#include <dcopobject.h>

#include <qmap.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qvariant.h>

class RSSService;

/**
 * Helper class which just calls the slot given it's QObject/const char*
 * representation.
 */
class SlotCaller : public QObject
{
	Q_OBJECT
	public:
		static void call( QObject *object, const char *slot,
		                  const KXMLRPC::Query::Result &value );

	signals:
		void signal( const KXMLRPC::Query::Result &value );

	private:
		SlotCaller() { }
};

class QueryService : public QObject, public DCOPObject
{
	K_DCOP
	Q_OBJECT
	public:
		QueryService( RSSService *service );

	k_dcop_signals:
		void feedIds( QValueList<int> ids );
		void feedInfo(QMap<QString, QString> links);
		void feedInfo(QValueList<RSSNewsFeed> feeds);
		void gotCategories( const QStringList &categories );
		void gotFeedsInCategory( const QValueList<int> &ids );

	k_dcop:
		/**
		 * Lists the active feeds in use...
		 **/
		QStringList listActive(); // just for testing...

		/**
		 * Query the www.syndic8.com XML-RPC interface for the
		 * string. The RSS ids are treturned in a integer list.
		 **/
		void findFeeds( const QString& query );

		/**
		 * Query the www.syndic8.com XML-RPC interface for the
		 * string and matches it against the SiteURL feed of 
		 * each feed in the feed list. The RSS ids are treturned 
		 * in a integer list.
		 **/
		void findSites( const QString& query );

		/**
		 * Query the www.syndic8.com XML-RPC interface for the
		 * requested RSS feed(s).  Returned is a QMap with the format
		 * of Name (Site URL), RSS URL
		 **/
		void getFeedInfo( const QVariant& ids );

		/**
		 * Returns the list of subcategories in the specified category.
		 * If no "Top" is specified, the root categories are returned.
		 */
		void getCategories( const QString &category );

		/**
		 * Queries the database for the list of needsfeed ID's which are
		 * associated with the given category.
		 */
		void getFeedsInCategory( const QString &category );


	private slots:
		void slotFoundFeeds( const KXMLRPC::Query::Result &result );
		void slotGotFeedInfo( const KXMLRPC::Query::Result &result );
		void slotGotCategories( const KXMLRPC::Query::Result &result );
		void slotGotFeedsInCategory( const KXMLRPC::Query::Result &result );

	private:
		void cachedCall( const QString &method, const QValueList<QVariant> &args,
		                 const char *slot );
		void updateCache( const KXMLRPC::Query::Result &result );

		RSSService *m_service;
		KXMLRPC::Server *m_xmlrpcServer;
};
#endif
