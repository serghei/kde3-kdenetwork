#ifndef ksview_h
#define ksview_h

/* This file is part of ksirc
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id: ksview.h 239587 2003-07-27 22:37:32Z asj $
//
#include "kstextview.h"

class KSircProcess;

class KSircView : public KSirc::TextView
{
	Q_OBJECT
public:
	KSircView(KSircProcess *proc, QWidget *parent, const char *name);
	virtual ~KSircView();

	virtual void clear();
	// returns the plain-text string added (for logging)
	QString addLine(const QString &pixmap, const QColor &color, const QString &text);
        void addRichText(const QString &_text);
        void enableTimeStamps(bool enable);

	void setAcceptFiles(bool _acceptFiles) { m_acceptFiles = _acceptFiles; }
signals:
        void urlsDropped(const QStringList&);
        void textDropped(const QString&);

protected:
        virtual void contentsDragEnterEvent(QDragEnterEvent *);
        virtual void contentsDragMoveEvent(QDragMoveEvent *);
        virtual void contentsDropEvent(QDropEvent *);

private:
	virtual bool linksEnabled() const { return true; }
    void openBrowser( const QString &url );
    void copyLinkToClipboard( const QString &url );
	static QString makeTimeStamp();

	void saveURL( const QString &url );

private slots:
	void anchorClicked(const QMouseEvent *ev, const QString &url);

	QColor ircColor(int);

private:
	int m_lines;
	QStringList m_timeStamps;
        bool m_acceptFiles;
	bool m_timestamps;
        KSircProcess *m_proc;
};

// vim: ts=4 sw=4 noet

#endif
