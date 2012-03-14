/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/


#include "topic.h"

#include <qapplication.h>
#include <qtimer.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <krun.h>
#include <kpopupmenu.h>
#include <kstringhandler.h>
#include <kfiledialog.h>
#include <kio/job.h>

#include "ksopts.h"
#include "ksparser.h"


KSircTopic::KSircTopic( QWidget *parent, const char *name )
    : KActiveLabel( parent, name )
{
    m_editor = 0;
    m_doEdit = false;
    m_height = 0;
//    setBackgroundColor( colorGroup().light() );
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    setTextFormat( RichText );
    setWordWrap(QTextEdit::NoWrap);
    doResize();
//    setAlignment( alignment() | WordBreak );
}

void KSircTopic::setText( const QString &_text)
{
    m_text = _text; /* save a raw copy for us */
    QString text = _text;

    QString richText( "<font color=\"%1\">" );
    richText = richText.arg( ksopts->textColor.name() );

    text.replace('&', "&amp;");
    text.replace('<', "&lt;");
    text.replace('>', "&gt;");

    text.replace('~', "~~");

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

    KSParser parser;
    richText += parser.parse( text );
    richText += "</font>";

    richText = KStringHandler::tagURLs( richText );
    KActiveLabel::setText(richText);

    doResize();

}

void KSircTopic::contentsMouseDoubleClickEvent( QMouseEvent * )
{
    m_doEdit = true;
}

void KSircTopic::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( m_doEdit ) {
        m_doEdit = false;

        if ( m_editor )
            return;

        doResize();

        m_editor = new KSircTopicEditor( this );

        connect( m_editor, SIGNAL( returnPressed() ),
                 this, SLOT( setNewTopic() ) );
        connect( m_editor, SIGNAL( resized() ),
                 this, SLOT( slotEditResized() ) );
        connect( m_editor, SIGNAL( destroyed() ),
                 this, SLOT( doResize() ) );

        /*
         * If you don't do this order
         * the size really breaks and you get
         * a huge widget
         */
        m_editor->setGeometry( geometry() );
        m_editor->setFocus();
        m_editor->show();

        m_editor->setText( m_text );
        QToolTip::remove(this);

    }
    KActiveLabel::contentsMouseReleaseEvent(e);
}

void KSircTopic::setNewTopic()
{
    QString topic = m_editor->text().stripWhiteSpace();

    /*
     * don't change the channel display
     * test since if it is set we'll get it
     * from the server.  If we can't set the topic
     * don't make it look like it was set
     */
    QTimer::singleShot( 0, m_editor, SLOT( close() ) );
    disconnect( m_editor, SIGNAL( resized() ),
             this, SLOT( slotEditResized() ) );
    doResize();
    emit topicChange( topic );
}

void KSircTopic::slotEditResized( )
{
    setFixedHeight( m_editor->height() );
}


void KSircTopic::doResize()
{

    int h;
    QFontMetrics metrics( currentFont() );

    h = metrics.lineSpacing()+8;
    m_height = h;
    setFixedHeight( h );


    QToolTip::remove(this);
    QStringList sep = QStringList::split(" ", m_text);
    int len = 0;
    QString brok;
    QStringList::Iterator it = sep.begin();
    for(; it != sep.end(); ++it) {
        brok += *it + " ";
        len += (*it).length();
        if(len >= 50){
            brok += "\n";
            len = 0;
        }
    }

    QToolTip::add(this, brok);

}

void KSircTopic::fontChange(QFont &f)
{
    KActiveLabel::fontChange(f);
    doResize();
}

KSircTopicEditor::KSircTopicEditor( QWidget *parent, const char *name )
    : QTextEdit( parent, name )
{
    setWFlags( WDestructiveClose );
    setFocusPolicy( QWidget::ClickFocus );
    connect( this, SIGNAL( textChanged () ), this, SLOT( slotMaybeResize() ) );
}

void KSircTopicEditor::keyPressEvent( QKeyEvent *ev )
{
    if ( ev->key() == Key_Escape )
    {
        ev->accept();
        QTimer::singleShot( 0, this, SLOT( close() ) );
        return;
    }
    else if ( ev->key() == Key_Return )
    {
        ev->accept();
        emit returnPressed();
        return;
    }
    QTextEdit::keyPressEvent( ev );
}

void KSircTopicEditor::focusOutEvent( QFocusEvent * )
{
    // we don't want to quit editing when someone brings up QLE's popup
    // menu
    if ( QFocusEvent::reason() == QFocusEvent::Popup  )
    {
        QWidget *focusw = qApp->focusWidget();
        if ( focusw && m_popup && focusw == m_popup )
            return;
    }

    QTimer::singleShot( 0, this, SLOT( close() ) );
}

QPopupMenu *KSircTopicEditor::createPopupMenu( const QPoint &pos )
{
    QPopupMenu *popup = QTextEdit::createPopupMenu( pos );
    m_popup = popup;
    return popup;
}

void KSircTopicEditor::slotMaybeResize()
{
    if(text().contains("\n")){
        QString s = text();
        s.replace('\n', " ");
        setText(s);
        setCursorPosition(0, s.length());
    }

    QFontMetrics metrics( currentFont() );

    int h = metrics.lineSpacing() * lines()+8;
    setFixedHeight( h );
    emit resized();
}

#include "topic.moc"
