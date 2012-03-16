/*
    Kopete Yahoo Protocol
    Send a file

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendfiletask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qtimer.h>
#include <kapplication.h> // for random()
#include <kdebug.h>
#include <klocale.h>
#include <kstreamsocket.h>
#include <kio/global.h>

using namespace KNetwork;

SendFileTask::SendFileTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	m_transmitted = 0;
	m_socket = 0;

	QTime epoch(0, 0, 0);
}

SendFileTask::~SendFileTask()
{
	m_socket->deleteLater();
	m_socket = 0;
}

bool SendFileTask::forMe( const Transfer *transfer ) const
{
	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	if(!t)
		return false;

	if((t->service() == Yahoo::ServiceFileTransfer7 ||
	    t->service() == Yahoo::ServiceFileTransfer7Accept) &&
	   t->firstParam(265).data() == m_yahooTransferId)
	{
		return true;
	}

	return false;
}

bool SendFileTask::take(Transfer* transfer)
{
	if( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	kdDebug(YAHOO_RAW_DEBUG) << t->service() << endl;

	if(t->service() == Yahoo::ServiceFileTransfer7)
		parseFileTransfer(t);
	else if(t->service() == Yahoo::ServiceFileTransfer7Accept)
		parseTransferAccept(t);

	return true;
}

void SendFileTask::parseFileTransfer( const Transfer *transfer )
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	if(!t)
		return;

	if(t->firstParam(222).toInt() == 4)
	{
		emit declined();
	}
	else if(t->firstParam(222).toInt() == 3)
	{
		sendFileTransferInfo();
	}
	else
	{
		setError();
		emit error(m_transferId, 0, i18n("Unknown error"));
	}
}

void SendFileTask::onGo()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	m_file.setName( m_url.path() );

	m_yahooTransferId = newYahooTransferId();

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7);
	t->setId( client()->sessionID() );

	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 5, m_target.local8Bit() );
	t->setParam( 265,  m_yahooTransferId.local8Bit() );
	t->setParam( 222, 1 );
	t->setParam( 266, 1 );
	t->setParam( 302, 268 );
	t->setParam( 300, 268 );
	t->setParam( 27, m_url.fileName().local8Bit() );
	t->setParam( 28, m_file.size());
	t->setParam( 301, 268 );
	t->setParam( 303, 268 );

	send( t );
}

void SendFileTask::sendFileTransferInfo()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	KResolverResults results = KResolver::resolve("filetransfer.msg.yahoo.com", QString::number(80));
	if(results.count() > 0)
	{
		m_relayHost = results.first().address().toString();
		m_relayHost = m_relayHost.left( m_relayHost.length() - 3 ); // Remove the :80 from the end
		kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << m_relayHost << endl;
	}
	else
	{
		emit error(m_transferId, 0, i18n("Unable to connect to file transfer server"));
		setError();
		return;
	}

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Info);
	t->setId( client()->sessionID() );

	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 5, m_target.local8Bit() );
	t->setParam( 265, m_yahooTransferId.local8Bit() );
	t->setParam( 27, m_url.fileName().local8Bit() );
	t->setParam( 249, 3 );
	t->setParam( 250, m_relayHost.local8Bit() );

	send( t );
}

void SendFileTask::parseTransferAccept(const Transfer *transfer)
{	
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	// Disconnected
	if(t->status() == Yahoo::StatusDisconnected)
	{
		setError();
		return;
	}

	m_token = KURL::encode_string(t->firstParam(251));
	kdDebug(YAHOO_RAW_DEBUG) << "Token: " << m_token << endl;

	m_socket = new KStreamSocket( m_relayHost, QString::number(80) );
	m_socket->setBlocking( true );
	connect( m_socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( m_socket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

	m_socket->connect();

}

void SendFileTask::connectFailed( int i )
{
	QString err = KSocketBase::errorString(m_socket->error());
	kdDebug(YAHOO_RAW_DEBUG) << i << ": " << err << endl;
	emit error( m_transferId, i, err );
	setError();
}

void SendFileTask::connectSucceeded()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );

	if ( m_file.open(IO_ReadOnly ) )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "File successfully opened. Reading..." << endl;
	}
	else
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error opening file: " << m_file.errorString() << endl;
		client()->notifyError( i18n( "An error occurred while sending the file." ), m_file.errorString(), Client::Error );
		setError();
		return;
	}

	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Sizes: File (" << m_url << "): " << m_file.size() << endl;
	QString header =
		"POST /relay?token=" + m_token +
		"&sender=" + client()->userId() +
		"&recver=" + m_target + " HTTP/1.1\r\n"
		"User-Agent: Mozilla/5.0\r\n"
		"Cache-Control: no-cache\r\n"
		"Cookie: T=" + client()->tCookie() + "; Y=" + client()->yCookie() + "\r\n"
		"Host: " + m_relayHost + "\r\n"
		"Content-Length: " + QString::number(m_file.size()) + "\r\n"
		"Connection: Close\r\n\r\n";
	kdDebug() << header << endl;
	stream.writeRawBytes( header.local8Bit(), header.length() );

	if( !m_socket->writeBlock( buffer, buffer.size() ) )
	{
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		m_socket->close();
	}
	else
	{
		connect( m_socket, SIGNAL(readyWrite()), this, SLOT(transmitData()) );
		m_socket->enableWrite( true );
	}
}

void SendFileTask::transmitData()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	int read = 0;
	int written = 0;	
	char buf[1024];

	m_socket->enableWrite( false );
	read = m_file.readBlock( buf, 1024 );
	written = m_socket->writeBlock( buf, read );
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "read:" << read << " written: " << written << endl;

	m_transmitted += read;
	emit bytesProcessed( m_transferId, m_transmitted );

	if( written != read )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Upload Failed!" << endl;
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		setError();
		return;
	}
	if( m_transmitted == m_file.size() )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Upload Successful: " << m_transmitted << endl;
		emit complete( m_transferId );
		setSuccess();
		m_socket->close();
	}
	else
	{
		m_socket->enableWrite( true );
	}
}
void SendFileTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendFileTask::setMessage( const QString &msg )
{
	m_msg = msg;
}

void SendFileTask::setFileUrl( KURL url )
{
	m_url = url;

}

void SendFileTask::setTransferId( unsigned int transferId )
{
	m_transferId = transferId;
}

void SendFileTask::canceled( unsigned int id )
{
	if( m_transferId != id )
		return;
	
	if( m_socket )
		m_socket->close();
	
	setError();
}

QString SendFileTask::newYahooTransferId()
{
	// Adapted from libpurple/protocols/yahoo/yahoo_filexfer.c yahoo_xfer_new_xfer_id()

	QString newId;

	for(int i = 0; i < 22; i++)
	{
		char j = KApplication::random() % 61;

		if(j < 26)
			newId += j + 'a';
		else if(j < 52)
			newId += j - 26 + 'A';
		else
			newId += j - 52 + '0';
	}

	newId += "$$";

	kdDebug() << "New Yahoo Transfer Id: " << newId << endl;

	return newId;
}

#include "sendfiletask.moc"

