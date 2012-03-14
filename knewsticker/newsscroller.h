/*
 * newsscroller.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 * Copyright (c) 2001 Malte Starostik <malte@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSSCROLLER_H
#define NEWSSCROLLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configaccess.h"
#include "newsengine.h"

#include <qframe.h>
#include <qptrlist.h>
#include <qpixmap.h>

class QTimer;
class Headline;
template <class> class QPtrList;
typedef QPtrList<Headline> HeadlineList;

class NewsScroller : public QFrame
{
	Q_OBJECT

	public:
		NewsScroller(QWidget *, ConfigAccess *, const char * = 0);

		virtual QSize sizeHint() const;
		virtual QSizePolicy sizePolicy() const;
	
		// Convenience stuff. Somehow ugly, no?	
		inline bool horizontal() const
		{
			return m_cfg->horizontal(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

		inline bool vertical() const
		{
			return m_cfg->vertical(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

		inline bool rotated() const
		{
			return m_cfg->rotated(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

	public slots:
		void clear();
		void addHeadline(Article::Ptr);
		void reset(bool bSeparatorOnly = false);

	signals:
		void contextMenu();

	protected:
		virtual void enterEvent(QEvent *);
		virtual void mousePressEvent(QMouseEvent *);
		virtual void mouseReleaseEvent(QMouseEvent *);
		virtual void mouseMoveEvent(QMouseEvent *);
		virtual void wheelEvent(QWheelEvent *);
		virtual void leaveEvent(QEvent *);
		virtual void drawContents(QPainter *);
		virtual void dragEnterEvent( QDragEnterEvent *);
		virtual void dropEvent(QDropEvent *);

	protected slots:
		void scroll(int = 1, bool = true);
		void slotTimeout();

	private:
		int scrollWidth() const;
		int scrollHeight() const;
		bool updateActive(const QPoint &);
		bool isHeadline(const QString &) const;
		int speedAsInterval( int speed );

	private:
		friend class Headline;
		ConfigAccess *m_cfg;
		QTimer       *m_scrollTimer;
		mutable HeadlineList m_headlines;
		Headline     *m_activeHeadline;
		QPixmap      m_separator;
		int          m_offset;
		QPoint       m_dragPos;
		bool         m_mouseDrag;
		QString      m_tempHeadline;
		float        m_totalStepping;
		float        m_stepping;
};

#endif // NEWSSCROLLER_H
