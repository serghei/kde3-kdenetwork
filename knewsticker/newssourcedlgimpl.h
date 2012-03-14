/*
 * newssourcedlgimpl.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSSOURCEDLGIMPL_H
#define NEWSSOURCEDLGIMPL_H
#include "newssourcedlg.h"
#include "newsengine.h"

#include <kdialogbase.h>
#include <kurl.h>

#include <qpixmap.h>

class XMLNewsSource;
class NewsIconMgr;
class QProgressBar;
class QTimer;

class SuggestProgressDlg : public KDialogBase
{
	Q_OBJECT

	public:
		SuggestProgressDlg(const KURL &url, QWidget *parent, const char *name = 0);
		virtual ~SuggestProgressDlg();

		XMLNewsSource *xmlSrc() { return m_xmlSrc; }
		QPixmap icon() const { return m_icon; }
		const KURL &iconURL() const { return m_iconURL; }

	private slots:
		void slotTimeoutTick();
		void slotLoadComplete(XMLNewsSource *, bool);
		void slotGotIcon(const KURL &, const QPixmap &);

	private:
		bool m_gotSourceFile;
		bool m_gotIcon;
		XMLNewsSource  *m_xmlSrc;
		bool m_succeeded;
		QPixmap m_icon;
		KURL m_iconURL;
		QProgressBar *m_progressBar;
		QTimer *m_timeoutTimer;
};

class NewsSourceDlgImpl : public NewsSourceDlg
{ 
	Q_OBJECT

	public:
		NewsSourceDlgImpl(QWidget * = 0, const char * = 0, bool = FALSE, WFlags = 0);

		void setup(const NewsSourceBase::Data &, bool);

	signals:
		void newsSource(const NewsSourceBase::Data &);

	protected slots:
		void slotCancelClicked();
		void slotOkClicked();
		void slotSourceFileChanged();
		void slotSuggestClicked();
		void slotModified();
		KURL polishedURL(const KURL &) const;
		bool validateURL(const KURL &);

	private slots:
		void slotGotIcon(const KURL &, const QPixmap &);

	private:
		bool           m_modified;
};

#endif // NEWSSOURCEDLGIMPL_H
