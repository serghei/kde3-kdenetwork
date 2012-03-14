/*
 * kxmlrpcclient.h - (c) 2003 Frerich Raabe <raabe@kde.org>
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
#ifndef KXMLRPCCLIENT_H
#define KXMLRPCCLIENT_H

#include <kurl.h>

#include <qbuffer.h>
#include <qdatastream.h>
#include <qobject.h>
#include <qvariant.h>
#include <qvaluelist.h>

class QDomDocument;
class QDomElement;

namespace KIO
{
	class Job;
}

namespace KXMLRPC
{
	class Query;
	class QueryResult;
	class Server;

	class Query : public QObject
	{
		Q_OBJECT
		public:
			class Result
			{
				friend class Query;
				friend QDataStream &operator>>( QDataStream &s, Query::Result &r );
				public:
					Result() { }

					bool success() const { return m_success; }
					int errorCode() const { return m_errorCode; }
					QString errorString() const { return m_errorString; }
					QValueList<QVariant> data() const { return m_data; }
					QString server() const { return m_server; }
					QString method() const { return m_method; }
					QValueList<QVariant> args() const { return m_args; }

				private:
					bool m_success;
					int m_errorCode;
					QString m_errorString;
					QValueList<QVariant> m_data;
					QString m_server;
					QString m_method;
					QValueList<QVariant> m_args;
			};

			static Query *create( QObject *parent = 0, const char *name = 0 );
			static QString marshal( const QVariant &v );
			static QVariant demarshal( const QDomElement &e );

		public slots:
			void call( const QString &server, const QString &method,
			           const QValueList<QVariant> &args = QValueList<QVariant>(),
			           const QString &userAgent = "KDE-XMLRPC" );

		signals:
			void infoMessage( const QString &msg );
			void finished( const KXMLRPC::Query::Result &result );

		private slots:
			void slotInfoMessage( KIO::Job *job, const QString &msg );
			void slotData( KIO::Job *job, const QByteArray &data );
			void slotResult( KIO::Job *job );

		private:
			bool isMessageResponse( const QDomDocument &doc ) const;
			bool isFaultResponse( const QDomDocument &doc ) const;

			Result parseMessageResponse( const QDomDocument &doc ) const;
			Result parseFaultResponse( const QDomDocument &doc ) const;

			QString markupCall( const QString &method,
			                    const QValueList<QVariant> &args ) const;

			Query( QObject *parent = 0, const char *name = 0 );

			QBuffer m_buffer;
			QString m_server;
			QString m_method;
			QValueList<QVariant> m_args;
	};

	class Server : public QObject
	{
		Q_OBJECT
		public:
			Server( const KURL &url = KURL(),
			        QObject *parent = 0, const char *name = 0 );

			const KURL &url() const { return m_url; }
			void setUrl( const KURL &url );

			QString userAgent() const { return m_userAgent; }
			void setUserAgent( const QString &userAgent) { m_userAgent = userAgent; }

			template <typename T>
			void call( const QString &method, const QValueList<T> &arg,
			           QObject *object, const char *slot );

			static QValueList<QVariant> toVariantList( const QVariant &arg );
			static QValueList<QVariant> toVariantList( int arg );
			static QValueList<QVariant> toVariantList( bool arg );
			static QValueList<QVariant> toVariantList( double arg );
			static QValueList<QVariant> toVariantList( const QString &arg );
			static QValueList<QVariant> toVariantList( const QCString &arg );
			static QValueList<QVariant> toVariantList( const QByteArray &arg );
			static QValueList<QVariant> toVariantList( const QDateTime &arg );
			static QValueList<QVariant> toVariantList( const QStringList &arg );

		signals:
			void infoMessage( const QString &msg );

		public slots:
			void call( const QString &method, const QValueList<QVariant> &args,
			           QObject *object, const char *slot );
			void call( const QString &method, const QValueList<QVariant> &args,
			           QObject *object, const char *slot,
			           QObject *infoObject, const char *infoSlot );

		private:
			KURL m_url;
			QString m_userAgent;
	};

	inline QDataStream &operator>>( QDataStream &s, Query::Result &r )
	{
		return s >> r.m_errorCode >> r.m_errorString >> r.m_data
		         >> r.m_server >> r.m_method >> r.m_args;
	}
}

template <typename T>
void KXMLRPC::Server::call( const QString &method, const QValueList<T> &arg,
                            QObject *object, const char *slot )
{
	QValueList<QVariant> args;

	typename QValueList<T>::ConstIterator it = arg.begin();
	typename QValueList<T>::ConstIterator end = arg.end();
	for ( ; it != end; ++it )
		args << QVariant( *it );

	call( method, args, object, slot );
}

inline QDataStream &operator<<( QDataStream &s, const KXMLRPC::Query::Result &r )
{
	return s << r.errorCode() << r.errorString() << r.data()
	         << r.server() << r.method() << r.args();
}

#endif // KXMLRPCCLIENT_H
// vim:ts=4:sw=4:noet
