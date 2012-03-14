/*
 * kxmlrpcclient.cpp - (c) 2003 Frerich Raabe <raabe@kde.org>
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
#include "xmlrpciface.h"

#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmdcodec.h>

#include <qdom.h>

using namespace KXMLRPC;

Query *Query::create( QObject *parent, const char *name )
{
	return new Query( parent, name );
}

void Query::call( const QString &server, const QString &method,
                  const QValueList<QVariant> &args, const QString &userAgent )
{
	m_buffer.open( IO_ReadWrite );
	m_server = server;
	m_method = method;
	m_args = args;

	const QString xmlMarkup = markupCall( method, args );

	QByteArray postData;
	QDataStream stream( postData, IO_WriteOnly );
	stream.writeRawBytes( xmlMarkup.utf8(), xmlMarkup.length() );

	KIO::TransferJob *job = KIO::http_post( KURL( server ), postData, false );
	job->addMetaData( "UserAgent", userAgent );
	job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
	connect( job, SIGNAL( infoMessage( KIO::Job *, const QString & ) ),
	         this, SLOT( slotInfoMessage( KIO::Job *, const QString & ) ) );
	connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
	         this, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
	connect( job, SIGNAL( result( KIO::Job * ) ),
	         this, SLOT( slotResult( KIO::Job * ) ) );
}

void Query::slotInfoMessage( KIO::Job *, const QString &msg )
{
	emit infoMessage( msg );
}

void Query::slotData( KIO::Job *, const QByteArray &data )
{
	m_buffer.writeBlock( data );
}

void Query::slotResult( KIO::Job *job )
{
	Result response;
	response.m_server = m_server;
	response.m_method = m_method;
	response.m_args = m_args;

	response.m_success = false;

	if ( job->error() != 0 ) {
		response.m_errorCode = job->error();
		response.m_errorString = job->errorString();
		emit finished( response );
		delete this;
		return;
	}

	QDomDocument doc;
	if ( !doc.setContent( m_buffer.buffer() ) ) {
			response.m_errorCode = -1;
			response.m_errorString = i18n( "Received invalid XML markup" );
			emit finished( response );
			delete this;
			return;
	}

	m_buffer.close();

	if ( isMessageResponse( doc ) )
		response = parseMessageResponse( doc );
	else if ( isFaultResponse( doc ) )
		response = parseFaultResponse( doc );
	else {
		response.m_errorCode = 1;
		response.m_errorString = i18n( "Unknown type of XML markup received" );
	}

	// parserMessageResponse and parseFaultResponse overwrite these fields.
	response.m_server = m_server;
	response.m_method = m_method;
	response.m_args = m_args;

	emit finished( response );
	delete this;
}

bool Query::isMessageResponse( const QDomDocument &doc ) const
{
	return doc.documentElement().firstChild().toElement().tagName().lower() == "params";
}

Query::Result Query::parseMessageResponse( const QDomDocument &doc ) const
{
	Result response;
	response.m_success = true;

	QDomNode paramNode = doc.documentElement().firstChild().firstChild();
	while ( !paramNode.isNull() ) {
		response.m_data << demarshal( paramNode.firstChild().toElement() );
		paramNode = paramNode.nextSibling();
	}

	return response;
}

bool Query::isFaultResponse( const QDomDocument &doc ) const
{
	return doc.documentElement().firstChild().toElement().tagName().lower() == "fault";
}

Query::Result Query::parseFaultResponse( const QDomDocument &doc ) const
{
	Result response;
	response.m_success = false;

	QDomNode errorNode = doc.documentElement().firstChild().firstChild();
	const QVariant errorVariant = demarshal( errorNode.toElement() );
	response.m_errorCode = errorVariant.toMap()[ "faultCode" ].toInt();
	response.m_errorString = errorVariant.toMap()[ "faultString" ].toString();

	return response;
}

QString Query::markupCall( const QString &cmd,
                           const QValueList<QVariant> &args ) const
{
	QString markup = "<?xml version='1.0' ?><methodCall>";

	markup += "<methodName>" + cmd + "</methodName>";

	if ( !args.isEmpty() ) {
		markup += "<params>";
		QValueList<QVariant>::ConstIterator it = args.begin();
		QValueList<QVariant>::ConstIterator end = args.end();
		for ( ; it != end; ++it )
			markup += "<param>" + marshal( *it ) + "</param>";
		markup += "</params>";
	}

	markup += "</methodCall>";

	return markup;
}

QString Query::marshal( const QVariant &arg )
{
	QString s = "<value>";
	switch ( arg.type() ) {
		case QVariant::String:
		case QVariant::CString:
			s += "<string>" + arg.toString() + "</string>";
			break;
		case QVariant::Int:
			s += "<int>" + QString::number( arg.toInt() ) + "</int>";
			break;
		case QVariant::Double:
			s += "<double>" + QString::number( arg.toDouble() ) + "</double>";
			break;
		case QVariant::Bool:
			s += "<boolean>";
			s += arg.toBool() ? "true" : "false";
			s += "</boolean>";
			break;
		case QVariant::ByteArray:
			s += "<base64>" + KCodecs::base64Encode( arg.toByteArray() ) + "</base64>";
			break;
		case QVariant::DateTime:
			s += "<datetime.iso8601>" + arg.toDateTime().toString( Qt::ISODate ) + "</datetime.iso8601>";
			break;
		case QVariant::List: {
			s += "<array><data>";
			const QValueList<QVariant> args = arg.toList();
			QValueList<QVariant>::ConstIterator it = args.begin();
			QValueList<QVariant>::ConstIterator end = args.end();
			for ( ; it != end; ++it )
				s += marshal( *it );
			s += "</data></array>";
			break;
		}
		case QVariant::Map: {
			s += "<struct>";
			QMap<QString, QVariant> map = arg.toMap();
			QMap<QString, QVariant>::ConstIterator it = map.begin();
			QMap<QString, QVariant>::ConstIterator end = map.end();
			for ( ; it != end; ++it ) {
				s += "<member>";
				s += "<name>" + it.key() + "</name>";
				s += marshal( it.data() );
				s += "</member>";
			}
			s += "</struct>";
			break;
		}
		default:
			kdWarning() << "Failed to marshal unknown variant type: " << arg.type() << endl;
			return "<value/>";
	};
	return s + "</value>";
}

QVariant Query::demarshal( const QDomElement &elem )
{
	Q_ASSERT( elem.tagName().lower() == "value" );

	if ( !elem.firstChild().isElement() )
		return QVariant( elem.text() );

	const QDomElement typeElement = elem.firstChild().toElement();
	const QString typeName = typeElement.tagName().lower();

	if ( typeName == "string" )
		return QVariant( typeElement.text() );
	else if ( typeName == "i4" || typeName == "int" )
		return QVariant( typeElement.text().toInt() );
	else if ( typeName == "double" )
		return QVariant( typeElement.text().toDouble() );
	else if ( typeName == "boolean" ) {
		if ( typeElement.text().lower() == "true" || typeElement.text() == "1" )
			return QVariant( true );
		else
			return QVariant( false );
	} else if ( typeName == "base64" )
		return QVariant( KCodecs::base64Decode( typeElement.text().latin1() ) );
	else if ( typeName == "datetime" || typeName == "datetime.iso8601" )
		return QVariant( QDateTime::fromString( typeElement.text(), Qt::ISODate ) );
	else if ( typeName == "array" ) {
		QValueList<QVariant> values;
		QDomNode valueNode = typeElement.firstChild().firstChild();
		while ( !valueNode.isNull() ) {
			values << demarshal( valueNode.toElement() );
			valueNode = valueNode.nextSibling();
		}
		return QVariant( values );
	} else if ( typeName == "struct" ) {
		QMap<QString, QVariant> map;
		QDomNode memberNode = typeElement.firstChild();
		while ( !memberNode.isNull() ) {
			const QString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
			const QVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
			map[ key ] = data;
			memberNode = memberNode.nextSibling();
		}
		return QVariant( map );
	} else
		kdWarning() << "Cannot demarshal unknown type " << typeName << endl;

	return QVariant();
}

Query::Query( QObject *parent, const char *name ) : QObject( parent, name )
{
}

QValueList<QVariant> Server::toVariantList( const QVariant &arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( int arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( bool arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( double arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( const QString &arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( const QCString &arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( const QByteArray &arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( const QDateTime &arg )
{
	QValueList<QVariant> args;
	args << arg ;
	return args;
}

QValueList<QVariant> Server::toVariantList( const QStringList &arg )
{
	QValueList<QVariant> args;
	QStringList::ConstIterator it = arg.begin();
	QStringList::ConstIterator end = arg.end();
	for ( ; it != end; ++it )
		args << QVariant( *it );
	return args;
}

Server::Server( const KURL &url, QObject *parent, const char *name )
	: QObject( parent, name )
{
	if ( url.isValid() )
		m_url = url;
}

void Server::setUrl( const KURL &url )
{
	m_url = url.isValid() ? url : KURL();
}

void Server::call( const QString &method, const QValueList<QVariant> &args,
                   QObject *receiver, const char *slot )
{
	if ( m_url.isEmpty() ) {
		kdWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;
		return;
	}

	Query *query = Query::create( this );
	connect( query, SIGNAL( infoMessage( const QString & ) ),
	         this, SIGNAL( infoMessage( const QString & ) ) );
	connect( query, SIGNAL( finished( const KXMLRPC::Query::Result & ) ),
	         receiver, slot );
	query->call( m_url.url(), method, args, m_userAgent );
}

void Server::call( const QString &method, const QValueList<QVariant> &args,
                   QObject *receiver, const char *slot,
                   QObject *infoObject, const char *infoSlot )
{
	if ( m_url.isEmpty() ) {
		kdWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;
		return;
	}

	Query *query = Query::create( this );
	connect( query, SIGNAL( infoMessage( const QString &msg ) ),
	         infoObject, infoSlot );
	connect( query, SIGNAL( finished( const KXMLRPC::Query::Result & ) ),
	         receiver, slot );
	query->call( m_url.url(), method, args, m_userAgent );
}

#include "xmlrpciface.moc"
// vim:ts=4:sw=4:noet
