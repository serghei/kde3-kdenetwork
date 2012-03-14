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

// $Id: ksview.cpp 393645 2005-02-27 22:05:39Z staikos $

#include <qclipboard.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qvaluestack.h>
#include <qstylesheet.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <krun.h>
#include <kpopupmenu.h>
#include <kstringhandler.h>
#include <knotifyclient.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <kurldrag.h>

#include "ksopts.h"
#include "ksview.moc"
#include "ksparser.h"
#include "nickColourMaker.h"
#include "ksircprocess.h"

KSircView::KSircView(KSircProcess *proc, QWidget *parent, const char *name)
    : KSirc::TextView(parent, name), m_proc(proc)
{
    m_acceptFiles = false;
    m_timestamps = false;
    viewport()->setAcceptDrops(true);
    clear();
    connect( this, SIGNAL( linkClicked( const QMouseEvent *, const QString & ) ),
             this, SLOT( anchorClicked( const QMouseEvent *, const QString & ) ) );

    QPixmap background = ksopts->backgroundPixmap();
    if ( !background.isNull() )
        viewport()->setBackgroundPixmap( background );

    setLinkColor( ksopts->linkColor );
}

KSircView::~KSircView()
{
}

void KSircView::clear()
{
    m_lines = 0;
    m_timeStamps.clear();
    KSirc::TextView::clear();
}

QString KSircView::makeTimeStamp()
{
    QTime now = QTime::currentTime();
    QString timeStamp = QString::fromLatin1( "[%1:%2:%3] " )
        .arg( QString::number( now.hour() ).rightJustify( 2, '0' ) )
        .arg( QString::number( now.minute() ).rightJustify( 2, '0' ) )
        .arg( QString::number( now.second() ).rightJustify( 2, '0' ) );
    return timeStamp;
}

void KSircView::saveURL( const QString &url )
{
	KURL kurl( url );

	KFileDialog *dlg = new KFileDialog( QString::null, QString::null /*filter*/, this /*parent*/, "filedialog" /*name*/, true /*modal*/ );

	dlg->setKeepLocation( true );

	dlg->setCaption( i18n( "Save As" ) );

	if ( !kurl.fileName().isEmpty() )
		dlg->setSelection( kurl.fileName() );

	if ( dlg->exec() ) {
		KURL destURL( dlg->selectedURL() );
		if ( destURL.isValid() ) {
			KIO::Job *job = KIO::copy( kurl, destURL );
			job->setAutoErrorHandlingEnabled( true );
		}
	}

	delete dlg;
}

QString KSircView::addLine(const QString &pixmap, const QColor &color, const QString &_text)
{
    //kdDebug(5008) << "Start Text:      " << _text << endl;

    QString richText( "<font color=\"%1\">" );
    richText = richText.arg( color.name() );

    if ( !pixmap.isEmpty() )
        richText.prepend( QString::fromLatin1( "<img src=\"%1\"></img>" ).arg( pixmap ) );

    QString timeStamp = QString::fromLatin1( "<font color=\"%1\">%2</font>" )
        .arg( ksopts->textColor.name() )
        .arg( makeTimeStamp() );
    m_timeStamps.append(timeStamp);
    if ( m_timestamps )
        richText.prepend( timeStamp );

    QString text = QStyleSheet::escape(_text);

    // ### a bit of a hack: turn '&lt;nick&gt; message' into
    // <span>&lt;nick&gt;<span> message' . span itself isn't supported but it
    // enforces the creation of separate item objects and hence separate
    // drawing of '<nick>' and 'message' , which is needed for BiDi users,
    // according to UV Kochavi <uv1st@yahoo.com> , to prevent output like
    // '<nick message<' , which is supposedly a bug in Qt's reordering.  The
    // same is done for [nick] and >nick< to catch queries.
    QRegExp bidiRe( "^(&lt;\\S+&gt;)(.+)$" );
    text.replace( bidiRe, QString::fromLatin1( "<span>\\1</span>\\2" ) );
    QRegExp bidiRe2( "^(\\[\\S+\\])(.+)$" );
    text.replace( bidiRe2, QString::fromLatin1( "<span>\\1</span>\\2" ) );
    QRegExp bidiRe3( "^(&gt;\\S+&lt;)(.+)$" );
    text.replace( bidiRe3, QString::fromLatin1( "<span>\\1</span>\\2" ) );

    QRegExp nickCol( "~n(.+)~n" );
    nickCol.setMinimal(true);
    int pos;

    while( (pos = nickCol.search(text)) >= 0 ){
        //kdDebug(5008) << "Found nick: " << nickCol.cap(1) << endl;
        QString newText = nickCol.cap(1);
        if( nickCol.cap(1) != m_proc->getNick()){
            QColor col = nickColourMaker::colourMaker()->findFg(nickCol.cap(1));

            newText.prepend(QString("<font color=\"%1\">").arg(col.name()));
            newText.append("</font>");
        }  else {
            QColor col = ksopts->ownNickColor.name();
            if( ! col.isValid())
                nickColourMaker::colourMaker()->findFg(nickCol.cap(1));
            newText.prepend(QString("<font color=\"%1\">").arg(col.name()));
            newText.append("</font>");
            if(ksopts->ownNickBold){
                newText.prepend("<b>");
                newText.append("</b>");
            }
            if(ksopts->ownNickUl){
                newText.prepend("<u>");
                newText.append("</u>");
            }
            if(ksopts->ownNickRev){
                newText.prepend("<r>");
                newText.append("</r>");
            }
        }
        text.replace(pos, nickCol.matchedLength(), newText);
    }

    //kdDebug(5008) << "After Text:      " << text << endl;

    KSParser parser;
    richText += parser.parse( text );

    richText += "</font>";


    //kdDebug(5008) << "Text:      " << _text << endl;


    richText = KStringHandler::tagURLs( richText );
    //kdDebug(5008) << "Rich text: " << richText << endl;

    KSirc::TextParagIterator parag = appendParag( richText );

    m_lines++;
    if ( ksopts->windowLength && m_lines > ksopts->windowLength )
    {
        while ( m_lines > ksopts->windowLength )
        {
            removeParag( firstParag() );
            m_timeStamps.remove( m_timeStamps.begin() );
            m_lines--;
        }
    }

    if (parser.beeped()) {
        KNotifyClient::event(winId(), QString::fromLatin1("BeepReceived"),
                        i18n("Beep Received"));
    }

    QString logText = parag.plainText();
    // append timestamp if it's not already there
    if ( ! m_timestamps )
        logText.prepend( makeTimeStamp() );

    return logText + '\n';
}

void KSircView::addRichText(const QString &_text)
{
    //kdDebug(5008) << "Start Text:      " << _text << endl;

    QString text = _text;

    QRegExp re("^(<font color=\"[^\"]+\">\\[[0-9:]+\\] </font>)");
    QString timeStamp;

    if(re.search(text) >= 0){
        timeStamp = re.cap(1);
    }
    else {
	timeStamp = QString::fromLatin1( "<font color=\"%1\">%2</font>" )
	    .arg( ksopts->textColor.name() )
	    .arg( makeTimeStamp() );
	if ( m_timestamps )
	    text.prepend( timeStamp );
    }
    m_timeStamps.append(timeStamp);

    KSirc::TextParagIterator parag = appendParag( text );

    m_lines++;
    if ( ksopts->windowLength && m_lines > ksopts->windowLength )
    {
        while ( m_lines > ksopts->windowLength )
        {
            removeParag( firstParag() );
            m_timeStamps.remove( m_timeStamps.begin() );
            m_lines--;
        }
    }

}

void KSircView::enableTimeStamps(bool enable)
{
    if(enable == m_timestamps)
        return;
    setUpdatesEnabled( false );
    m_timestamps = enable;
    KSirc::TextParagIterator paragIt = firstParag();
    QStringList::ConstIterator timeStampIt = m_timeStamps.begin();
    for (; !paragIt.atEnd(); ++paragIt, ++timeStampIt )
    {
        QString text = paragIt.richText();
        if ( enable )
            text.prepend( *timeStampIt );
        else
            text.remove( 0, (*timeStampIt).length() );
        paragIt.setRichText( text );
    }
    setUpdatesEnabled( true );
    updateContents();
}

void KSircView::anchorClicked(const QMouseEvent *ev, const QString &url)
{
    if ( (ev->button() & LeftButton) && (ev->state() & ShiftButton ) )
		saveURL( url );
	else if ( (ev->button() & LeftButton) || (ev->button() & MidButton) )
    {
        openBrowser( url );
    }
    else if ( ev->button() & RightButton )
    {
        static const int openURLID = 0;
        static const int copyLinkLocationID = 1;

        // Adding a nice contextmenu
        KPopupMenu* menu = new KPopupMenu( this );
        menu->insertTitle( i18n( "URL" ) );
        menu->insertItem( i18n("Open URL"), openURLID );
        menu->insertItem( i18n("Copy Link Address"), copyLinkLocationID );
        switch( menu->exec( ( ev->globalPos() ) ) )
        {
        case openURLID :
            openBrowser( url );
            break;
        case copyLinkLocationID :
            copyLinkToClipboard( url );
            break;
        default:
            break;
        }
        delete menu;
    }
}

void KSircView::openBrowser(const QString &url )
{
    (void) new KRun( KURL( url.startsWith("www") ? QString::fromLatin1("http://") + url : url));
}

void KSircView::copyLinkToClipboard( const QString &url )
{
    QApplication::clipboard()->setText( url, QClipboard::Clipboard );
}

QColor KSircView::ircColor(int code)
{
    if (code >= 0 && code < 16)
        return ksopts->ircColors[code];
    return QColor();
}

void KSircView::contentsDragEnterEvent(QDragEnterEvent* event)
{
    event->accept((QTextDrag::canDecode(event) ||
                   (m_acceptFiles && KURLDrag::canDecode(event))) &&
                  (!event->source() || event->source() != viewport()));
}

void KSircView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    event->accept(!event->source() || event->source() != viewport());
}

void KSircView::contentsDropEvent(QDropEvent* event)
{
    QStringList urls;
    QString text;

    if (m_acceptFiles && KURLDrag::decodeLocalFiles(event, urls))
        emit urlsDropped(urls);
    else if (QTextDrag::decode(event, text))
        emit textDropped(text);
}

// vim: ts=4 sw=4 noet
