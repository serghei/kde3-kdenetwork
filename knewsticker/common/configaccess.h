/*
 * configaccess.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef CONFIGACCESS_H
#define CONFIGACCESS_H

#include "configiface.h"
#include "newsengine.h"

#include <kconfig.h>
#include <kio/job.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qvaluelist.h>

#define DEFAULT_NEWSSOURCES 63
#define DEFAULT_SUBJECTS 13

class QColor;
class QFont;
class KURL;

class ArticleFilter {
	public:
		typedef QValueList<int> List;

		ArticleFilter(const QString & = I18N_NOOP(QString::fromLatin1("Show")),
				const QString & = I18N_NOOP(QString::fromLatin1("all newssources")),
				const QString & = I18N_NOOP(QString::fromLatin1("contain")),
				const QString & = QString::null,
				bool = true);

		QString action() const { return m_action; }
		void setAction(const QString &action) { m_action = action; }

		QString newsSource() const { return m_newsSource; }
		void setNewsSource(const QString &newsSource) { m_newsSource = newsSource; }
		
		QString condition() const { return m_condition; }
		void setCondition(const QString &condition) { m_condition = condition; }

		QString expression() const { return m_expression; }
		void setExpression(const QString &expression) { m_expression = expression; }

		bool enabled() const { return m_enabled; }
		void setEnabled(bool enabled) { m_enabled = enabled; }

		unsigned int id() const { return m_id; }
		void setId(const unsigned int id) { m_id = id; }

		bool matches(Article::Ptr) const;
	
	private:
		QString m_action;
		QString m_newsSource;
		QString m_condition;
		QString m_expression;
		bool    m_enabled;
		unsigned int    m_id;
};

class ConfigAccess : public ConfigIface
{
	public:
		ConfigAccess();
		ConfigAccess(KConfig *);
		virtual ~ConfigAccess();

		virtual unsigned int interval() const;
		virtual unsigned int scrollingSpeed() const;
		virtual unsigned int mouseWheelSpeed() const;
		virtual unsigned int scrollingDirection() const;
		virtual bool customNames() const;
		virtual bool scrollMostRecentOnly() const;
		virtual bool offlineMode() const;
		virtual bool underlineHighlighted() const;
		virtual bool showIcons() const;
		virtual bool slowedScrolling() const;
		virtual QColor foregroundColor() const;
		virtual QColor backgroundColor() const;
		virtual QColor highlightedColor() const;
		QFont font() const;
		virtual QStringList newsSources() const;
		NewsSourceBase *newsSource(const QString &);
		ArticleFilter::List filters() const;
		ArticleFilter filter(const unsigned int) const;
		
		static bool horizontal(Direction d) { return d == Left || d == Right; }
		static bool vertical(Direction d) { return d == Up || d == Down; }
		static bool rotated(Direction d) { return d == UpRotated || d == DownRotated; }

		inline bool horizontalScrolling() const
		{
			return horizontal((Direction) scrollingDirection());
		};

		inline bool verticalScrolling() const
		{
			return vertical((Direction)scrollingDirection());
		};

		virtual void setInterval(const unsigned int);
		virtual void setScrollingSpeed(const unsigned int);
		virtual void setMouseWheelSpeed(const unsigned int);
		virtual void setScrollingDirection(const unsigned int);
		virtual void setCustomNames(bool);
		virtual void setScrollMostRecentOnly(bool);
		virtual void setOfflineMode(bool);
		virtual void setUnderlineHighlighted(bool);
		virtual void setShowIcons(bool);
		virtual void setSlowedScrolling(bool);
		virtual void setForegroundColor(const QColor &);
		virtual void setBackgroundColor(const QColor &);
		virtual void setHighlightedColor(const QColor &);
		void setFont(const QFont &);
		virtual void setNewsSources(const QStringList &);
		void setNewsSource(const NewsSourceBase::Data &);
		void setFilters(const ArticleFilter::List &);
		void setFilter(const ArticleFilter &);
		void reparseConfiguration() { m_cfg->reparseConfiguration(); }
	
	private:
		KConfig *m_cfg;
		KConfig *m_defaultCfg;
};

#endif // CONFIGACCESS_H
