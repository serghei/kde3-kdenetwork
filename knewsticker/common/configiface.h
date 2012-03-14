/*
 * configface.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef CONFIGIFACE_H
#define CONFIGIFACE_H

class QColor;
class QStringList;
class QFont;

class KURL;

class ConfigIface
{
	public:
		enum Direction {Left = 0, Right, Up, Down, UpRotated, DownRotated};

		virtual unsigned int interval() const = 0;
		virtual unsigned int scrollingSpeed() const = 0;
		virtual unsigned int mouseWheelSpeed() const = 0;
		virtual unsigned int scrollingDirection() const = 0;
		virtual bool customNames() const = 0;
		virtual bool scrollMostRecentOnly() const = 0;
		virtual bool offlineMode() const = 0;
		virtual bool underlineHighlighted() const = 0;
		virtual bool showIcons() const = 0;
		virtual bool slowedScrolling() const = 0;
		virtual QColor foregroundColor() const = 0;
		virtual QColor backgroundColor() const = 0;
		virtual QColor highlightedColor() const = 0;
		virtual QStringList newsSources() const = 0;
		
		virtual void setInterval(const unsigned int) = 0;
		virtual void setScrollingSpeed(const unsigned int) = 0;
		virtual void setMouseWheelSpeed(const unsigned int) = 0;
		virtual void setScrollingDirection(const unsigned int) = 0;
		virtual void setCustomNames(bool) = 0;
		virtual void setScrollMostRecentOnly(bool) = 0;
		virtual void setOfflineMode(bool) = 0;
		virtual void setUnderlineHighlighted(bool) = 0;
		virtual void setShowIcons(bool) = 0;
		virtual void setSlowedScrolling(bool) = 0;
		virtual void setForegroundColor(const QColor &) = 0;
		virtual void setBackgroundColor(const QColor &) = 0;
		virtual void setHighlightedColor(const QColor &) = 0;
		virtual void setNewsSources(const QStringList &) = 0;
};

#endif // CONFIGIFACE_H
