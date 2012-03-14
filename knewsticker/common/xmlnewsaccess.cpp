/*
 * xmlnewsaccess.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "xmlnewsaccess.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>

#include <qbuffer.h>
#include <qdom.h>
#include <qregexp.h>

XMLNewsArticle::XMLNewsArticle(const QString &headline, const KURL &address)
	: m_headline(headline),
	m_address(address)
{
}

XMLNewsArticle &XMLNewsArticle::operator=(const XMLNewsArticle &other)
{
	m_headline = other.m_headline;
	m_address = other.m_address;
	return *this;
}

bool XMLNewsArticle::operator==(const XMLNewsArticle &a)
{
	return m_headline == a.headline() && m_address == a.address();
}

XMLNewsSource::XMLNewsSource() : QObject(),
 	m_name(QString::null),
	m_link(QString::null),
	m_description(QString::null),
	m_downloadData(0)
{
}

XMLNewsSource::~XMLNewsSource()
{
	delete m_downloadData; // Might exist if we are in the middle of a download
}

void XMLNewsSource::loadFrom(const KURL &url)
{
	if ( m_downloadData != 0 ) {
		kdDebug( 5005 ) << "XMLNewsSource::loadFrom(): Busy, ignoring load "
		                   "request for " << url << endl;
		return;
	}
	m_downloadData = new QBuffer;
	m_downloadData->open(IO_WriteOnly);

	KIO::Job *job = KIO::get(url, false, false);
	job->addMetaData(QString::fromLatin1("UserAgent"),
	                 QString::fromLatin1("KNewsTicker v0.2"));
	connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
			SLOT(slotData(KIO::Job *, const QByteArray &)));
	connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
}

void XMLNewsSource::slotData(KIO::Job *, const QByteArray &data)
{
	m_downloadData->writeBlock(data.data(), data.size());
}

void XMLNewsSource::slotResult(KIO::Job *job)
{
	kdDebug(5005) << "XMLNewsSource::slotResult(): Finished downloading data (" << job->error() << ")." << endl;
	processData(m_downloadData->buffer(), !job->error());
	delete m_downloadData;
	m_downloadData = 0;
}

void XMLNewsSource::processData(const QByteArray &data, bool okSoFar)
{
	bool validContent = okSoFar;
	kdDebug(5005) << "XMLNewsSource::processData(): validContent = " << validContent << endl;
	
	if (okSoFar) {
		/*
		 * Some servers prepend whitespace before the <?xml...?> declaration.
		 * Since QDom doesn't like that we strip this first.
		 */
		QDomDocument domDoc;

		const char *charData = data.data();
		int len = data.count();

		while (len && (*charData == ' ' || *charData == '\n' || *charData == '\t' || *charData == '\r') ) {
			len--;
			charData++;
		}

		QByteArray tmpData;
		tmpData.setRawData(charData, len);

		if (validContent = domDoc.setContent(tmpData)) {
			QDomNode channelNode = domDoc.documentElement().namedItem(QString::fromLatin1("channel"));
	
			m_name = channelNode.namedItem(QString::fromLatin1("title")).toElement().text().simplifyWhiteSpace();
			kdDebug(5005) << "XMLNewsSource::processData(): Successfully updated " << m_name << endl;
			m_link = channelNode.namedItem(QString::fromLatin1("link")).toElement().text().simplifyWhiteSpace();
			m_description = channelNode.namedItem(QString::fromLatin1("description")).toElement().text().simplifyWhiteSpace();

			QDomNodeList items = domDoc.elementsByTagName(QString::fromLatin1("item"));
			m_articles.clear();
			QDomNode itemNode;
			QString headline, address;
			for (unsigned int i = 0; i < items.count(); i++) {
				itemNode = items.item(i);
				headline = KCharsets::resolveEntities(itemNode.namedItem(QString::fromLatin1("title")).toElement().text().simplifyWhiteSpace());
				address = KCharsets::resolveEntities(itemNode.namedItem(QString::fromLatin1("link")).toElement().text().simplifyWhiteSpace());
				m_articles.append(XMLNewsArticle(headline, KURL( address )));
			}
		}
		kdDebug(5005) << "XMLNewsSource::processData(): validContent = " << validContent << endl;
		tmpData.resetRawData(charData, len);
	}

	emit loadComplete(this, validContent);
}

#include "xmlnewsaccess.moc"
