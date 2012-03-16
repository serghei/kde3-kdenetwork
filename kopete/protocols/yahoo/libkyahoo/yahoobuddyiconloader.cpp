/*
    yahoobuddyiconloader.cpp - Fetches YahooBuddyIcons

    Copyright (c) 2005 by André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoobuddyiconloader.h"

// QT Includes
#include <qfile.h>

// KDE Includes
#include <kdebug.h>
#include <ktempfile.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "yahootypes.h"
#include "client.h"

YahooBuddyIconLoader::YahooBuddyIconLoader( Client *c )
: m_client( c )
{
}

YahooBuddyIconLoader::~YahooBuddyIconLoader()
{
}

void YahooBuddyIconLoader::fetchBuddyIcon( const QString &who, KURL url, int checksum )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << url << endl;
	KIO::TransferJob *transfer;
	QString Url = url.url();
	QString ext = Url.left( Url.findRev( "?" ) );
	ext = ext.right( ext.length() - ext.findRev( "." ) );

	transfer = KIO::get( url, false, false );
	connect( transfer, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
	connect( transfer, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );

	m_jobs[transfer].url = url;
	m_jobs[transfer].who = who;
	m_jobs[transfer].checksum = checksum;

}

void YahooBuddyIconLoader::slotData( KIO::Job *job, const QByteArray& data )
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	// FIXME need to check
	//m_jobs[transfer].icon.append( data );
	int oldsize = m_jobs[transfer].icon.size();
	m_jobs[transfer].icon.resize( data.size() + oldsize );
	memcpy( m_jobs[transfer].icon.data() + oldsize, data.data(), data.size() );
}

void YahooBuddyIconLoader::slotComplete( KIO::Job *job )
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kdDebug(YAHOO_RAW_DEBUG) << "An error occurred while downloading buddy icon." << endl;
		if( m_client )
			m_client->notifyError( i18n( "An error occurred while downloading a buddy icon (%1)").arg( m_jobs[transfer].url.url() ), job->errorString(), Client::Info );
	}
	else
	{
		emit fetchedBuddyIcon( m_jobs[transfer].who, m_jobs[transfer].icon, m_jobs[transfer].checksum );
	}

	m_jobs.remove( transfer );
}



#include "yahoobuddyiconloader.moc"

