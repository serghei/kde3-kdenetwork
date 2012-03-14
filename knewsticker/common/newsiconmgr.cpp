/*
 * newsiconmgr.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "newsiconmgr.h"

#include <dcopclient.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <qbuffer.h>
#include <qfile.h>
#include <qimage.h>

struct KIODownload
{
	KURL       url;
	QByteArray data;
	QIODevice::Offset dataOffset;
};

NewsIconMgr *NewsIconMgr::m_instance = 0;

NewsIconMgr *NewsIconMgr::self()
{
	if (!m_instance)
		m_instance = new NewsIconMgr();

	return m_instance;
}

NewsIconMgr::NewsIconMgr(QObject *parent, const char *name)
	: QObject(parent, name), DCOPObject("NewsIconMgr"),
	m_stdIcon(SmallIcon(QString::fromLatin1("news")))
{
	connectDCOPSignal("kded",
			"favicons", "iconChanged(bool, QString, QString)",
			"slotGotIcon(bool, QString, QString)",
			false);
}

NewsIconMgr::~NewsIconMgr()
{
	delete m_instance;
}

void NewsIconMgr::getIcon(const KURL &url)
{
	if (url.isEmpty()) {
		emit gotIcon(url, m_stdIcon);
		return;
	}

	if (url.isLocalFile()) {
		if (QFile::exists(url.encodedPathAndQuery())) {
			QPixmap icon(url.encodedPathAndQuery());
			if (!icon.isNull()) {
				if (icon.size() != QSize(16, 16)) {
					if (!icon.convertFromImage(icon.convertToImage().smoothScale(16, 16, QImage::ScaleMin))) {
						emit gotIcon(url, m_stdIcon);
						return;
					}
				}
				emit gotIcon(url, icon);
				return;
			}
		}
		emit gotIcon(url, m_stdIcon);
		return;
	}

	if (url.encodedPathAndQuery() == "/favicon.ico") {
		if (favicon(url).isNull()) {
			QByteArray data;
			QDataStream ds(data, IO_WriteOnly);
			ds << url;
			kapp->dcopClient()->send("kded", "favicons", "downloadHostIcon(KURL)", data);
		} else {
			emit gotIcon(url, QPixmap(KGlobal::dirs()->findResource("cache",
							QString::fromLatin1("favicons/%1.png").arg(url.host()))));
		}
	} else {
		KIO::Job *job = KIO::get(url, true, false);
		connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
				SLOT(slotData(KIO::Job *, const QByteArray &)));
		connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
		
		KIODownload download;
		download.url = url;
		download.dataOffset = 0;
		m_kioDownload.insert(job, download);
	}
}

bool NewsIconMgr::isStdIcon(const QPixmap &pixmap) const
{
	if (!pixmap.isNull())
		return pixmap.convertToImage() == m_stdIcon.convertToImage();
	else
		return false;
}

void NewsIconMgr::slotData(KIO::Job *job, const QByteArray &data)
{
	QBuffer buf(m_kioDownload[job].data);
	buf.open(IO_WriteOnly);
	buf.at(m_kioDownload[job].dataOffset);
	buf.writeBlock(data);
	m_kioDownload[job].dataOffset = buf.at();
}

void NewsIconMgr::slotResult(KIO::Job *job)
{
	emit gotIcon(m_kioDownload[job].url, QPixmap(m_kioDownload[job].data));
	m_kioDownload.remove(job);
}

void NewsIconMgr::slotGotIcon(bool isHost, QString hostOrURL, QString iconName)
{
	KURL url = KURL(hostOrURL);
	if (!isHost)
		url.setProtocol(QString::fromLatin1("http"));
	
	if (iconName.isNull())
		emit gotIcon(url, m_stdIcon);
	else
		emit gotIcon(url, QPixmap(KGlobal::dirs()->findResource("cache",
						QString::fromLatin1("favicons/%1.png").arg(url.host()))));
}

QString NewsIconMgr::favicon(const KURL &url) const
{
	QByteArray data, reply;
	QCString replyType;
	QDataStream ds(data, IO_WriteOnly);

	ds << url;
	
	kapp->dcopClient()->call("kded", "favicons", "iconForURL(KURL)", data, replyType, reply);
	
	if (replyType == "QString") {
		QDataStream replyStream(reply, IO_ReadOnly);
		QString result;
		replyStream >> result;
		return result;
	}
	
	return QString::null;
}

#include "newsiconmgr.moc"
