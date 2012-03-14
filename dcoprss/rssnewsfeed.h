/*
 * rssnewsfeed.h
 *
 * Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef RSSNEWSFEED_H
#define RSSNEWSFEED_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <kdatastream.h>

class QueryService;

class RSSNewsFeed
{
	friend QDataStream &operator>>( QDataStream &stream, RSSNewsFeed &feed );
	friend QDataStream &operator<<( QDataStream &stream, const RSSNewsFeed &feed );
	friend class QueryService;
	public:
		unsigned int id() const { return m_id; }
		QString name() const { return m_name; }
		QString description() const { return m_description; }
		QString origin() const { return m_origin; }
		QString languageCode() const { return m_languageCode; }
		QString status() const { return m_status; }
		QString version() const { return m_version; }
		QString homePage() const { return m_homePage; }
		QString sourceFile() const { return m_sourceFile; }
		QString imageUrl() const { return m_imageUrl; }
		QString webmaster() const { return m_webmaster; }
		QString editor() const { return m_editor; }
		QString publisher() const { return m_publisher; }
		QString creator() const { return m_creator; }
		const QDateTime &dateCreated() const { return m_dateCreated; }
		const QDateTime &dateApproved() const { return m_dateApproved; }
		const QDateTime dateXmlChanged() const { return m_dateXmlChanged; }
		bool fetchable() const { return m_fetchable; }
		unsigned int views() const { return m_views; }
		unsigned int headlinesPerDay() const { return m_headlinesPerDay; }
		unsigned int headlinesRank() const { return m_headlinesRank; }
		QString toolkit() const { return m_toolkit; }
		QString toolkitVersion() const { return m_toolkitVersion; }
		unsigned int pollingInterval() const { return m_pollingInterval; }
		const QDateTime &lastPoll() const { return m_lastPoll; }
		QStringList categories() const { return m_categories; }
	
	private:
		unsigned int m_id;
		QString m_name;
		QString m_description;
		QString m_origin;
		QString m_languageCode;
		QString m_status;
		QString m_version;
		QString m_homePage;
		QString m_sourceFile;
		QString m_imageUrl;
		QString m_webmaster;
		QString m_editor;
		QString m_publisher;
		QString m_creator;
		QDateTime m_dateCreated;
		QDateTime m_dateApproved;
		QDateTime m_dateXmlChanged;
		bool m_fetchable;
		unsigned int m_views;
		unsigned int m_headlinesPerDay;
		unsigned int m_headlinesRank;
		QString m_toolkit;
		QString m_toolkitVersion;
		unsigned int m_pollingInterval;
		QDateTime m_lastPoll;
		QStringList m_categories;
};

inline QDataStream &operator<<( QDataStream &stream, const RSSNewsFeed &feed )
{
	return stream << feed.m_id << feed.m_name << feed.m_description
	              << feed.m_origin << feed.m_languageCode << feed.m_status
	              << feed.m_version << feed.m_homePage << feed.m_sourceFile
	              << feed.m_imageUrl << feed.m_webmaster << feed.m_publisher
	              << feed.m_creator << feed.m_dateCreated << feed.m_dateApproved
	              << feed.m_dateXmlChanged << feed.m_fetchable << feed.m_views 
	              << feed.m_headlinesPerDay << feed.m_headlinesRank
	              << feed.m_toolkit << feed.m_toolkitVersion
	              << feed.m_pollingInterval << feed.m_lastPoll
	              << feed.m_categories;
}

inline QDataStream &operator>>( QDataStream &stream, RSSNewsFeed &feed )
{
	int i;
	stream >> feed.m_id >> feed.m_name >> feed.m_description
	       >> feed.m_origin >> feed.m_languageCode >> feed.m_status
	       >> feed.m_version >> feed.m_homePage >> feed.m_sourceFile
	       >> feed.m_imageUrl >> feed.m_webmaster >> feed.m_publisher
	       >> feed.m_creator >> feed.m_dateCreated >> feed.m_dateApproved
	       >> feed.m_dateXmlChanged >> i >> feed.m_views 
	       >> feed.m_headlinesPerDay >> feed.m_headlinesRank
	       >> feed.m_toolkit >> feed.m_toolkitVersion
	       >> feed.m_pollingInterval >> feed.m_lastPoll
	       >> feed.m_categories;
	feed.m_fetchable = i != 0;
	return stream;
}

#endif
// vim:ts=4:sw=4:noet
