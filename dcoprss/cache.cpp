/*
 * cache.cpp - (c) 2003 Frerich Raabe <raabe@kde.org>
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
#include "cache.h"
#include "xmlrpciface.h"

#include <kdebug.h>
#include <kstandarddirs.h>

#include <qdatastream.h>
#include <qfile.h>

bool CacheEntry::isValid() const
{
	// Cache entries get invalid after on hour. One shouldn't hardcode this
	// but for now it'll do.
	return m_timeStamp.secsTo( QDateTime::currentDateTime() ) < 3600;
}

Cache *Cache::m_instance = 0;

Cache &Cache::self()
{
	if ( !m_instance )
		m_instance = new Cache;
	return *m_instance;
}

QString Cache::getCacheKey( const QString &server, const QString &method,
                            const QValueList<QVariant> &args )
{
	QString key;
	key = server + QString::fromLatin1( "__" );
	key += method + QString::fromLatin1( "__" );
	QValueList<QVariant>::ConstIterator it = args.begin();
	QValueList<QVariant>::ConstIterator end = args.end();
	for ( ; it != end; ++it )
		key += KXMLRPC::Query::marshal( *it );
	
	return key;
}

Cache::Cache()
{
	load();
}

Cache::~Cache()
{
	save();
}

void Cache::load()
{
	QFile file( cacheFileName() );
	if ( !file.open( IO_ReadOnly ) ) {
		kdDebug() << "Failed to open cache file " << cacheFileName() << endl;
		return;
	}
	
	QDataStream stream( &file );
	while ( !stream.atEnd() ) {
		QString key;
		stream >> key;

		CacheEntry *entry = new CacheEntry;
		stream >> *entry;

		QDict<CacheEntry>::insert( key, entry );
	}
}

void Cache::save()
{
	QFile file( cacheFileName() );
	if ( !file.open( IO_WriteOnly ) ) {
		kdDebug() << "Failed to open cache file " << cacheFileName() << endl;
		return;
	}

	QDataStream stream( &file );

	QDictIterator<CacheEntry> it( *this );
	for ( ; it.current() != 0; ++it )
		stream << it.currentKey() << *it.current();
}

void Cache::touch( const QString &key )
{
	CacheEntry *entry = find( key );
	if ( !entry )
		return;
	entry->m_timeStamp = QDateTime::currentDateTime();
}

void Cache::insert( const QString &key, const KXMLRPC::Query::Result &result )
{
	CacheEntry *entry = new CacheEntry;
	entry->m_timeStamp = QDateTime::currentDateTime();
	entry->m_result = result;
	QDict<CacheEntry>::insert( key, entry );
}

QString Cache::cacheFileName() const
{
	return locateLocal( "appdata", "cache/dcoprss.cache" );
}

