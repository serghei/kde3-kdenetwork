/*
 * newsscroller.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 * Copyright (c) 2001 Malte Starostik <malte@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include <dcopclient.h>

#include <qpainter.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kcursor.h>
#include <kstandarddirs.h>
#include <kurldrag.h>
#include <kmessagebox.h>

#include "configaccess.h"
#include "newsscroller.h"
#include "newsengine.h"
#include <kapplication.h>

class Headline
{
public:
	Headline(NewsScroller *scroller, const Article::Ptr &article)
		: m_scroller(scroller),
		 m_article(article),
		 m_normal(0),
		 m_highlighted(0)
	{
	};
	
	virtual ~Headline()
	{
		reset();
	}

	Article::Ptr article() const { return m_article; }	
	
	int width() { return pixmap()->width(); }
	int height() { return pixmap()->height(); }
	
	QPixmap *pixmap(bool highlighted = false, bool underlineHighlighted = true)
	{
		QPixmap *result = highlighted ? m_highlighted : m_normal;
		if (!result) {
			const QFontMetrics &metrics = m_scroller->fontMetrics();

			int w, h;
			if (m_scroller->m_cfg->showIcons()) {
				w = m_article->newsSource()->icon().width() + 4 + metrics.width(m_article->headline());
				h = QMAX(metrics.height(), m_article->newsSource()->icon().height());
			} else {
				w = metrics.width(m_article->headline());
				h = metrics.height();
			}

			if (ConfigAccess::rotated(static_cast<ConfigAccess::Direction>(m_scroller->m_cfg->scrollingDirection())))
				result = new QPixmap(h, w);
			else
				result = new QPixmap(w, h);
			
			result->fill(m_scroller->m_cfg->backgroundColor());
			QPainter p(result);
			QFont f = m_scroller->font();
			
			if (highlighted)
				f.setUnderline(underlineHighlighted);
			
			p.setFont(f);
			p.setPen(highlighted ? m_scroller->m_cfg->highlightedColor() : m_scroller->m_cfg->foregroundColor());

			if (ConfigAccess::rotated(static_cast<ConfigAccess::Direction>(m_scroller->m_cfg->scrollingDirection()))) {
				if (m_scroller->m_cfg->scrollingDirection() == ConfigAccess::UpRotated) {
					// Note that rotation also
					// changes the coordinate space
					//
					p.rotate(90.0);
					if (m_scroller->m_cfg->showIcons()) {
						p.drawPixmap(0, -m_article->newsSource()->icon().height(), m_article->newsSource()->icon());
						p.drawText(m_article->newsSource()->icon().width() + 4, -metrics.descent(), m_article->headline());
					} else
						p.drawText(0, -metrics.descent(), m_article->headline());
				} else {
					p.rotate(-90.0);
					if (m_scroller->m_cfg->showIcons()) {
						p.drawPixmap(-w, h - m_article->newsSource()->icon().height(), m_article->newsSource()->icon());
						p.drawText(-w + m_article->newsSource()->icon().width() + 4, h - metrics.descent(), m_article->headline());
					} else
						p.drawText(-w, h - metrics.descent(), m_article->headline());
				}
			} else {
				if (m_scroller->m_cfg->showIcons()) {
					p.drawPixmap(0,
						(result->height() - m_article->newsSource()->icon().height()) / 2,
						m_article->newsSource()->icon());
					p.drawText(m_article->newsSource()->icon().width() + 4, result->height() - metrics.descent(), m_article->headline());
				} else
					p.drawText(0, result->height() - metrics.descent(), m_article->headline());
			}
			
			if (highlighted)
				m_highlighted = result;
			else
				m_normal = result;
		}
		return result;
	}

	void reset()
	{
		delete m_normal;
		m_normal = 0;
		delete m_highlighted;
		m_highlighted = 0;
	}

private:
	NewsScroller *m_scroller;
	Article::Ptr m_article;
	QPixmap      *m_normal;
	QPixmap      *m_highlighted;
};

NewsScroller::NewsScroller(QWidget *parent, ConfigAccess *cfg, const char *name)
	: QFrame(parent, name, WNoAutoErase),
	m_cfg(cfg),
	m_scrollTimer(new QTimer(this)),
	m_activeHeadline(0),
	m_mouseDrag(false),
	m_totalStepping(0.0)
{
	if (!kapp->dcopClient()->isAttached())
		kapp->dcopClient()->attach();
	
	setFrameStyle(StyledPanel | Sunken);
	
	m_headlines.setAutoDelete(true);

	connect(m_scrollTimer, SIGNAL(timeout()), SLOT(slotTimeout()));

	setAcceptDrops(true);
	
	reset();
}

QSize NewsScroller::sizeHint() const
{
	return QSize(fontMetrics().width(QString::fromLatin1("X")) * 20, fontMetrics().height() * 2);
}

QSizePolicy NewsScroller::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void NewsScroller::clear()
{
	m_headlines.clear();
	reset();
}

void NewsScroller::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept(QTextDrag::canDecode(event));
}

void NewsScroller::dropEvent(QDropEvent* event)
{
	QString newSourceUrl;
	if ( QTextDrag::decode(event, newSourceUrl) ) {
		// <HACK>
		// This is just for http://www.webreference.com/services/news/
		newSourceUrl = newSourceUrl.replace(QRegExp(
				QString::fromLatin1("^view-source:http%3A//")),
				QString::fromLatin1("http://"));
		// </HACK>
		newSourceUrl = newSourceUrl.stripWhiteSpace();
		if (!isHeadline(newSourceUrl) && KMessageBox::questionYesNo(this, i18n("<p>Do you really want to add '%1' to"
				" the list of news sources?</p>")
					.arg(newSourceUrl), QString::null, i18n("Add"), KStdGuiItem::cancel()) == KMessageBox::Yes) {
			KConfig cfg(QString::fromLatin1("knewsticker_panelappletrc"), false, false);
			ConfigAccess configFrontend(&cfg);
			QStringList newsSources = configFrontend.newsSources();

			QString name = i18n("Unknown");
			if (newsSources.contains(name))
				for (unsigned int i = 0; ; i++)
					if (!newsSources.contains(i18n("Unknown %1").arg(i))) {
						name = i18n("Unknown %1").arg(i);
						break;
					}

			newsSources += name;
			configFrontend.setNewsSource(NewsSourceBase::Data(name, newSourceUrl));
			configFrontend.setNewsSources(newsSources);

			QByteArray data;
			kapp->dcopClient()->send("knewsticker", "KNewsTicker", "reparseConfig()", data);
		}
	}
}

bool NewsScroller::isHeadline(const QString &location) const
{
	for (Headline *h = m_headlines.first(); h; h = m_headlines.next())
		if (h->article()->address() == location)
			return true;

	return false;
}

void NewsScroller::addHeadline(Article::Ptr article)
{
	for (unsigned int i = 0; i < m_cfg->filters().count(); i++)
		if (m_cfg->filter(i).matches(article))
			return;
			
	m_headlines.append(new Headline(this, article));
}

void NewsScroller::scroll(int distance, bool interpret_directions)
{
  unsigned int t_dir;
  if ( interpret_directions ) t_dir = m_cfg->scrollingDirection();
  else t_dir = m_cfg->horizontalScrolling() ? ConfigAccess::Left : ConfigAccess::Up;
	switch (t_dir) {
		case ConfigAccess::Left:
			m_offset -= distance;
			if (m_offset <= - scrollWidth())
				m_offset = m_offset + scrollWidth() - m_separator.width();
			break;
		case ConfigAccess::Right:
			m_offset += distance;
			if (m_offset >= contentsRect().width())
				m_offset = m_offset + m_separator.width() - scrollWidth();
			break;
		case ConfigAccess::Up:
		case ConfigAccess::UpRotated:
			m_offset -= distance;
			if (m_offset <= - scrollHeight())
				m_offset = m_offset + scrollHeight() - m_separator.height();
			break;
		case ConfigAccess::Down:
		case ConfigAccess::DownRotated:
			m_offset += distance;
			if (m_offset >= contentsRect().height())
				m_offset = m_offset + m_separator.height() - scrollHeight();
	}
	
	QPoint pt = mapFromGlobal(QCursor::pos());
	
	if (contentsRect().contains(pt))
		updateActive(pt);

	update();
}

void NewsScroller::enterEvent(QEvent *)
{
	if (m_cfg->slowedScrolling() && m_cfg->scrollingSpeed() > 1)
		m_scrollTimer->changeInterval(speedAsInterval(m_cfg->scrollingSpeed() / 2));
}

void NewsScroller::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::LeftButton || e->button() == QMouseEvent::MidButton) {
		m_dragPos = e->pos();
		
		if (m_activeHeadline)
			m_tempHeadline = m_activeHeadline->article()->headline();
	}
}

void NewsScroller::mouseReleaseEvent(QMouseEvent *e)
{
	if ((e->button() == QMouseEvent::LeftButton || e->button() == QMouseEvent::MidButton)
			&& m_activeHeadline && m_activeHeadline->article()->headline() == m_tempHeadline
			&& !m_mouseDrag) {
		m_activeHeadline->article()->open();
		m_tempHeadline = QString::null;
	}
	
	if (e->button() == QMouseEvent::RightButton)
		emit(contextMenu());
	
	if (m_mouseDrag) {
		m_mouseDrag = false;
		if (m_cfg->scrollingSpeed())
			m_scrollTimer->start(speedAsInterval(m_cfg->scrollingSpeed()));
	}
}

void NewsScroller::mouseMoveEvent(QMouseEvent *e)
{
	// Are we in a drag phase?
	if (!m_mouseDrag) {
		// If not, check whether we need to start a drag.
		int dragDistance = 0;
		if (m_cfg->horizontalScrolling())
			dragDistance = QABS(e->x() - m_dragPos.x());
		else
			dragDistance = QABS(e->y() - m_dragPos.y());
		m_mouseDrag = (e->state() & QMouseEvent::LeftButton != 0) &&
			dragDistance >= KGlobal::config()->readNumEntry("StartDragDist", KApplication::startDragDistance());
		if (m_mouseDrag) // Stop the scroller if we just started a drag.
			m_scrollTimer->stop();
	} else	{
		// If yes, move the scroller accordingly.
		bool createDrag;
		if (m_cfg->horizontalScrolling()) {
			scroll(m_dragPos.x() - e->x(), false);
			m_dragPos = e->pos();
			createDrag = e->y() < 0 || e->y() > height();
		} else {
			scroll(m_dragPos.y() - e->y(), false);
			m_dragPos = e->pos();
			createDrag = e->x() < 0 || e->x() > width();
		}
		m_dragPos = e->pos();
		if (createDrag && m_activeHeadline) {
			KURL::List url;
			url.append(m_activeHeadline->article()->address());
			QDragObject *drag = new KURLDrag(url, this);
			drag->setPixmap(m_activeHeadline->article()->newsSource()->icon());
			drag->drag();
			m_mouseDrag = false;
			if (m_cfg->scrollingSpeed())
				m_scrollTimer->start(speedAsInterval(m_cfg->scrollingSpeed()));
		}
	}

	if (updateActive(e->pos()))
		update();
}

void NewsScroller::wheelEvent(QWheelEvent *e)
{
	// ### This 11 - m_cfg->mouseWheelSpeed() could be eliminated by swapping
	// the labels of the QSlider. :]
	int distance = qRound(QABS(e->delta()) / (11 - m_cfg->mouseWheelSpeed()));
	int direction = e->delta() > 0 ? -1 : 1;

	for (int i = 0; i < distance; i++)
		scroll(direction);

	QFrame::wheelEvent(e);
}

void NewsScroller::leaveEvent(QEvent *)
{
	if (m_cfg->slowedScrolling() && m_cfg->scrollingSpeed() > 1)
		m_scrollTimer->changeInterval(speedAsInterval(m_cfg->scrollingSpeed()));

	if (m_activeHeadline) {
		m_activeHeadline = 0;
		update();
	}
}

void NewsScroller::drawContents(QPainter *p)
{
	if (!scrollWidth() || // No news and no "No News Available": uninitialized
	    m_headlines.isEmpty()) // Happens when we're currently fetching new news
		return;

	QPixmap buffer(contentsRect().width(), contentsRect().height());
	buffer.fill(m_cfg->backgroundColor());
	int pos = m_offset;

	// Paste in all the separator bitmaps (" +++ ")
	if (horizontal()) {
		while (pos > 0)
			pos -= scrollWidth() - (m_headlines.isEmpty() ? m_separator.width() : 0);
		do {
			bitBlt(&buffer, pos, (contentsRect().height() - m_separator.height()) / 2, &m_separator);
			pos += m_separator.width();
		} while (m_headlines.isEmpty() && pos < contentsRect().width());
	} else {
		while (pos > 0)
			pos -= scrollHeight() - (m_headlines.isEmpty() ? 0 : m_separator.height());
		do {
			bitBlt(&buffer, (contentsRect().width() - m_separator.width()) / 2, pos, &m_separator);
			pos += m_separator.height();
		} while (m_headlines.isEmpty() && pos < contentsRect().height());
	}
	
	// Now do the headlines themselves
	do {
		QPtrListIterator<Headline> it(m_headlines);
		for(; *it; ++it) {
			if (horizontal()) {
				if (pos + (*it)->width() >= 0)
					bitBlt(&buffer, pos, (contentsRect().height() - (*it)->height()) / 2, (*it)->pixmap(*it == m_activeHeadline, m_cfg->underlineHighlighted()));
				pos += (*it)->width();
				
				if (pos + m_separator.width() >= 0)
					bitBlt(&buffer, pos, (contentsRect().height() - m_separator.height()) / 2, &m_separator);
				pos += m_separator.width();
				
				if (pos >= contentsRect().width())
					break;
			} else {
				if (pos + (*it)->height() >= 0)
					bitBlt(&buffer, (contentsRect().width() - (*it)->width()) / 2, pos, (*it)->pixmap(*it == m_activeHeadline, m_cfg->underlineHighlighted()));
				pos += (*it)->height();
				
				if (pos + m_separator.height() >= 0)
					bitBlt(&buffer, (contentsRect().width() - m_separator.width()) / 2, pos, &m_separator);
				pos += m_separator.height();
				
				if (pos > contentsRect().height())
					break;
			}
		}

		/*
		 * Break out if we reached the bottom of the window before the end of the
		 * list of headlines.
		 */
		if (*it)
			break;
	}
	while ((m_cfg->horizontalScrolling() && pos < contentsRect().width()) || pos < contentsRect().height());

	p->drawPixmap(0, 0, buffer);
}

void NewsScroller::reset(bool bSeparatorOnly)
{
	setFont(m_cfg->font());
	
	m_scrollTimer->stop();
	if (m_cfg->scrollingSpeed())
		m_scrollTimer->start(speedAsInterval(m_cfg->scrollingSpeed()));
	
	QString sep = m_headlines.isEmpty() ? i18n(" +++ No News Available +++") : QString::fromLatin1(" +++ ");

	int w = fontMetrics().width(sep);
	int h = fontMetrics().height();

	if (rotated())
		m_separator.resize(h, w);
	else
		m_separator.resize(w, h);

	m_separator.fill(m_cfg->backgroundColor());
	
	QPainter p(&m_separator);
	p.setFont(font());
	p.setPen(m_cfg->foregroundColor());

	if(rotated()) {
		if (m_cfg->scrollingDirection() == ConfigAccess::UpRotated) {
			p.rotate(90.0);
			p.drawText(0, -fontMetrics().descent(),sep);
		} else {
			p.rotate(-90.0);
			p.drawText(-w, h-fontMetrics().descent(),sep);
		}
	} else
		p.drawText(0, m_separator.height() - fontMetrics().descent(), sep);
	p.end();
	
	if (!bSeparatorOnly)	
		for (QPtrListIterator<Headline> it(m_headlines); *it; ++it)
			(*it)->reset();

	switch (m_cfg->scrollingDirection()) {
		case ConfigAccess::Left:
			m_offset = contentsRect().width();
			break;
		case ConfigAccess::Right:
			m_offset = - scrollWidth();
			break;
		case ConfigAccess::Up:
		case ConfigAccess::UpRotated:
			m_offset = contentsRect().height();
			break;
		case ConfigAccess::Down:
		case ConfigAccess::DownRotated:
			m_offset = - scrollHeight();
	}

	update();
}

int NewsScroller::scrollWidth() const
{
	int result = (m_headlines.count() + 1) * m_separator.width();
	
	for (QPtrListIterator<Headline> it(m_headlines); *it; ++it)
		result += (*it)->width();
	
	return result;
}

int NewsScroller::scrollHeight() const
{
	int result = (m_headlines.count() + 1) * m_separator.height();
	
	for (QPtrListIterator<Headline> it(m_headlines); *it; ++it)
		result += (*it)->height();
	
	return result;
}

bool NewsScroller::updateActive(const QPoint &pt)
{
	int pos = m_offset;
	
	Headline *headline = 0;
		
	if (!m_headlines.isEmpty()) {
		while (pos > 0)
			if (horizontal())
				pos -= scrollWidth() - m_separator.width();
			else
				pos -= scrollHeight() - m_separator.height();

		do {
			QPtrListIterator<Headline> it(m_headlines);
			for (; (headline = *it); ++it) {
				QRect rect;
				if (horizontal()) {
					pos += m_separator.width();
					rect.moveTopLeft(QPoint(pos, (contentsRect().height() - (*it)->height()) / 2));
					pos += (*it)->width();
				} else {
					pos += m_separator.height();
					rect.moveTopLeft(QPoint((contentsRect().width() - (*it)->width()) / 2, pos));
					pos += (*it)->height();
				}
				rect.setSize(QSize((*it)->width(), (*it)->height()));
				
				if (m_mouseDrag)
					if (horizontal()) {
						rect.setTop(0);
						rect.setHeight(height());
					} else {
						rect.setLeft(0);
						rect.setWidth(width());
					}

				if (rect.contains(pt))
					break;
			}
			if (*it)
				break;
		}
		while ((m_cfg->horizontalScrolling() && pos < contentsRect().width()) || pos < contentsRect().height());
	}
	
	if (m_activeHeadline == headline)
		return false;
	
	if ((m_activeHeadline = headline))
		setCursor(KCursor::handCursor());
	else
		unsetCursor();
	
	return true;
}

void NewsScroller::slotTimeout()
{
	m_totalStepping += m_stepping;
	if ( m_totalStepping >= 1.0 ) {
		const int distance = static_cast<int>( m_totalStepping );
		m_totalStepping -= distance;
		scroll( distance );
	}
}

int NewsScroller::speedAsInterval( int speed )
{
	Q_ASSERT( speed > 0 );

	static const int MaxScreenUpdates = 25;

	if ( speed <= MaxScreenUpdates ) {
		m_stepping = 1.0;
		return 1000 / speed;
	}

	m_stepping = speed / MaxScreenUpdates;
	return 1000 / MaxScreenUpdates;
}

#include "newsscroller.moc"
