/*
 * knewsticker.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef KNEWSTICKER_H
#define KNEWSTICKER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopobject.h>

#include <qtoolbutton.h>
#include <kpanelapplet.h>
#include <kpopupmenu.h>

#include "configiface.h"
#include "configaccess.h"
#include "newsengine.h"

class KInstance;
class NewsSource;
class KArrowButton;
class NewsIconMgr;
class NewsScroller;
class KNewsTickerMenu;
class KAboutData;
class QTimer;

class KNewsTicker : public KPanelApplet, virtual public ConfigIface,
		virtual public DCOPObject
{
	Q_OBJECT
	K_DCOP

	friend class KNewsTickerMenu;

	public:
		KNewsTicker(const QString &, Type, int, QWidget * = 0, const char * = 0);
		virtual ~KNewsTicker();

		virtual int widthForHeight(int) const;
		virtual int heightForWidth(int) const;

	k_dcop:
		virtual void reparseConfig();
		virtual void updateNews() { slotUpdateNews(); }
		virtual uint interval() const { return m_cfg->interval(); }
		virtual uint scrollingSpeed() const { return m_cfg->scrollingSpeed(); }
		virtual uint mouseWheelSpeed() const { return m_cfg->mouseWheelSpeed(); }
		virtual uint scrollingDirection() const { return m_cfg->scrollingDirection(); }
		virtual bool customNames() const { return m_cfg->customNames(); }
		virtual bool scrollMostRecentOnly() const { return m_cfg->scrollMostRecentOnly(); }
		virtual bool offlineMode() const { return m_cfg->offlineMode(); }
		virtual bool underlineHighlighted() const { return m_cfg->underlineHighlighted(); }
		virtual bool showIcons() const { return m_cfg->showIcons(); }
		virtual bool slowedScrolling() const { return m_cfg->slowedScrolling(); }
		virtual QColor foregroundColor() const { return m_cfg->foregroundColor(); }
		virtual QColor backgroundColor() const { return m_cfg->backgroundColor(); }
		virtual QColor highlightedColor() const { return m_cfg->highlightedColor(); }
		virtual QStringList newsSources() const { return m_cfg->newsSources(); }
		virtual void setInterval(const uint);
		virtual void setScrollingSpeed(const uint);
		virtual void setMouseWheelSpeed(const uint);
		virtual void setScrollingDirection(const uint);
		virtual void setCustomNames(bool);
		virtual void setScrollMostRecentOnly(bool);
		virtual void setOfflineMode(bool);
		virtual void setUnderlineHighlighted(bool);
		virtual void setShowIcons(bool);
		virtual void setSlowedScrolling(bool);
		virtual void setForegroundColor(const QColor &);
		virtual void setBackgroundColor(const QColor &);
		virtual void setHighlightedColor(const QColor &);
		virtual void setNewsSources(const QStringList &) {}

	public slots:
		void slotUpdateNews();
		void slotOpenContextMenu();

	protected:
		virtual void preferences();
		virtual void about();
		virtual void help();
		virtual void reportBug();
		virtual void mousePressEvent(QMouseEvent *);
		virtual void positionChange(Position);

	protected slots:
		void slotArrowButtonPressed();
		void slotNewsSourceUpdated(const NewsSourceBase::Ptr &, bool = false);
		void slotNewsSourceFailed(const NewsSourceBase::Ptr &);
		void slotContextMenuAboutToHide();
		void slotNotifyOfFailures();

	private:
		void setupArrowButton();

		KInstance            *m_instance;
		DCOPClient           *m_dcopClient;
		ConfigAccess         *m_cfg;
		KArrowButton         *m_arrowButton;
		QTimer               *m_newsTimer;
		QTimer               *m_updateTimer;
		NewsIconMgr          *m_newsIconMgr;
		NewsScroller         *m_scroller;
		KAboutData           *m_aboutData;
		KNewsTickerMenu      *m_contextMenu;
		bool                 m_newNews;
		NewsSourceBase::List m_newsSources;
		QStringList	         m_failedNewsUpdates;
		QStringList          m_pendingNewsUpdates;
};

class KNewsTickerMenu : public KPopupMenu
{
	Q_OBJECT

	public:
		KNewsTickerMenu(KNewsTicker *, const char * = 0);
		void setFullMenu(bool full) { m_fullMenu = full; populateMenu(); }

	protected slots:
		void populateMenu();

	private slots:
		void slotShowHelp();
		void slotShowAbout();
		void slotConfigure();
		void slotToggleOfflineMode();
		void slotCheckNews(int idx);
		void slotOpenArticle(int idx);

	private:
		KNewsTicker *m_parent;
		bool m_fullMenu;
};

#endif // KNEWSTICKER_H
