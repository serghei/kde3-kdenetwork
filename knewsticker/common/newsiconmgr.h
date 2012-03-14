/*
 * newsiconmgr.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSICONMGR_H
#define NEWSICONMGR_H

#include <qobject.h>
#include <qpixmap.h>

#include <dcopobject.h>
#include <kio/job.h>
#include <kurl.h>

class DCOPClient;

struct KIODownload;
typedef QMap<KIO::Job *, KIODownload> KIODownloadMap;

class NewsIconMgr : public QObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP

	public:
		static NewsIconMgr *self();
	
		void getIcon(const KURL &);
		bool isStdIcon(const QPixmap &) const;
	
	k_dcop:
		void slotGotIcon(bool, QString, QString);
	
	signals:
		void gotIcon(const KURL &, const QPixmap &);
	
	protected:
		NewsIconMgr(QObject * = 0L, const char * = 0L);
		~NewsIconMgr();
		
	private slots:
		void slotData(KIO::Job *, const QByteArray &);
		void slotResult(KIO::Job *);

	private:
		QString favicon(const KURL &) const;

		QPixmap        m_stdIcon;
		KIODownloadMap m_kioDownload;

		static NewsIconMgr *m_instance;
};

#endif // NEWSICONMGR_H
