/*
 * knewstickerconfig.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef KCMNEWSTICKER_H
#define KCMNEWSTICKER_H

#include "knewstickerconfigwidget.h"
#include "configaccess.h"
#include "newsengine.h"

#include <kdialogbase.h>

#include <qevent.h>
#include <qfont.h>
#include <qlistview.h>

class KNewsTickerConfig;
class KNewsTickerConfigWidget;
class KConfig;
class NewsIconMgr;
class ConfigAccess;

class CategoryItem : public QListViewItem
{
	public:
		CategoryItem(QListView *, const QString &);

		void setOpen(bool);
};

class NewsSourceItem : public QCheckListItem
{
	public:
		NewsSourceItem(KNewsTickerConfig *, CategoryItem *, const NewsSourceBase::Data &);

		NewsSourceBase::Data data() const;
		void setData(const NewsSourceBase::Data &);
		void setIcon(const QPixmap &);

		QListViewItem *parentItem() { return QCheckListItem::parent(); }
	
	private:
		QString      m_icon;
		bool         m_isProgram;
		NewsSourceBase::Subject m_subject;
		CategoryItem *m_parent;
		KNewsTickerConfig *m_kcm;
};

class KNewsTickerConfig : public KDialogBase
{
	Q_OBJECT
	friend class NewsSourceItem;
	public:
		KNewsTickerConfig(ConfigAccess *, QWidget * = 0, const char * = 0);

		void load();
		void save();
		void defaults();

	protected:
		void addNewsSource(const NewsSourceBase::Data &, bool = false);
		void modifyNewsSource(QListViewItem *);
		void removeNewsSource();
		void addFilter(const ArticleFilter &);
		void removeFilter(QListViewItem *);
		void resizeEvent(QResizeEvent *);
		void openModifyDialog();
		bool eventFilter(QObject *o, QEvent *e);
		void getNewsIcon(NewsSourceItem *, const KURL &);

	protected slots:
		void slotNewsSourceContextMenu(KListView *, QListViewItem *, const QPoint &);
		void slotChooseFont();
		void slotAddNewsSource();
		void slotAddFilter();
		void slotAddNewsSource(const NewsSourceBase::Data &);
		void slotRemoveNewsSource();
		void slotRemoveFilter();
		void slotModifyNewsSource();
		void slotModifyNewsSource(const NewsSourceBase::Data &);
		void slotModifyNewsSource(QListViewItem *, const QPoint &, int);
		void slotNewsSourceSelectionChanged();
		void slotFilterSelectionChanged(QListViewItem *);
		void slotFilterActionChanged(const QString &);
		void slotFilterNewsSourceChanged(const QString &);
		void slotFilterConditionChanged(const QString &);
		void slotFilterExpressionChanged(const QString &);
		virtual void slotOk();

	private slots:
		void slotGotNewsIcon(const KURL &, const QPixmap &);

	private:
		ConfigAccess        *m_cfg;
		KNewsTickerConfigWidget *m_child;
		QFont               m_font;
		NewsSourceItem      *m_modifyItem;
		NewsIconMgr         *m_newsIconMgr;
		QMap<QString, NewsSourceItem *> m_itemIconMap;
};

#endif // KCMNEWSTICKER_H
