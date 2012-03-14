/*
 * cache.h - (c) 2003 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef CACHE_H
#define CACHE_H

#include <qcstring.h>
#include <qdatetime.h>
#include <qdict.h>

#include <xmlrpciface.h>

class CacheEntry
{
	friend class Cache;
	friend QDataStream &operator>>( QDataStream &s, CacheEntry &e );
	public:
		const QDateTime &timeStamp() const { return m_timeStamp; }
		const KXMLRPC::Query::Result result() const { return m_result; }
		bool isValid() const;
	
	private:
		QDateTime m_timeStamp;
		KXMLRPC::Query::Result m_result;
};

class Cache : public QDict<CacheEntry>
{
	public:
		static Cache &self();

		static QString getCacheKey( const QString &server,
		                            const QString &method,
		                            const QValueList<QVariant> &args );

		void load();
		void save();

		void touch( const QString &key );

		void insert( const QString &key, const KXMLRPC::Query::Result &result );

	private:
		Cache();
		Cache( const Cache &rhs ); // disabled
		Cache &operator=( const Cache &rhs ); // disabled
		~Cache();

		QString cacheFileName() const;

		static Cache *m_instance;
};

inline QDataStream &operator<<( QDataStream &s, const CacheEntry &e )
{
	return s << e.timeStamp() << e.result();
}

inline QDataStream &operator>>( QDataStream &s, CacheEntry &e )
{
	return s >> e.m_timeStamp >> e.m_result;
}

#endif // CACHE_H
// vim:ts=4:sw=4:noet
