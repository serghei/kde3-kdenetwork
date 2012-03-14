/*
 * xmlnewsaccess.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef XMLNEWSACCESS_H
#define XMLNEWSACCESS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qvaluelist.h>
#include <qobject.h>

#include <kio/job.h>
#include <kurl.h>

class QBuffer;
class QDomDocument;
class XMLNewsSource;

class XMLNewsArticle
{
	public:
		typedef QValueList<XMLNewsArticle> List;
		
		XMLNewsArticle() {}
		XMLNewsArticle(const QString &headline, const KURL &addresss);
		XMLNewsArticle(const XMLNewsArticle &other) { (*this) = other; }
		XMLNewsArticle &operator=(const XMLNewsArticle &other);

		QString headline() const { return m_headline; }
		void setHeadline(const QString &headline) { m_headline = headline; }

		KURL address() const { return m_address; }
		void setAddress(const KURL &address) { m_address = address; }

		bool operator== (const XMLNewsArticle &a);
		bool operator!= (const XMLNewsArticle &a) { return !operator==(a); }

	private:
		QString        m_headline;
		KURL           m_address;
};

class XMLNewsSource : public QObject
{
	Q_OBJECT
		
	public:
		XMLNewsSource();
		virtual ~XMLNewsSource();

		void loadFrom(const KURL &);

		virtual QString newsSourceName() const { return m_name; }
		virtual QString link() const { return m_link; }
		virtual QString description() const { return m_description; }
		const XMLNewsArticle::List &articles() const { return m_articles; }

	signals:
		void loadComplete(XMLNewsSource *, bool);
	
	protected:
		void processData(const QByteArray &, bool = true);
		
		QString m_name;
		QString m_link;
		QString m_description;

	private slots:
		void slotData(KIO::Job *, const QByteArray &);
		void slotResult(KIO::Job *);
	
	private:
		XMLNewsArticle::List m_articles;
		QBuffer             *m_downloadData;
};

#endif // XMLNEWSACCESS_H
