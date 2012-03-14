/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "colorpicker.h"
#include "ksopts.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qvbox.h>
#include <qstyle.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>

ColorPicker::ColorPicker( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true /*modal*/, i18n( "Pick Color" ),
                   KDialogBase::Ok | KDialogBase::Cancel,
                   KDialogBase::Cancel ),
      m_foregroundColor( -1 ), m_backgroundColor( -1 )
{
    QVBox *mainWidget = makeVBoxMainWidget();

    QWidget *sampleBox = new QWidget( mainWidget );
    QHBoxLayout *sampleLayout = new QHBoxLayout( sampleBox );

    QLabel *preview = new QLabel( i18n( "Preview:" ), sampleBox );
    sampleLayout->addWidget( preview );

    m_sample = new QLineEdit( i18n( "Sample Text" ), sampleBox );
    m_sample->setFocusPolicy( NoFocus );
    m_sample->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                             m_sample->sizePolicy().verData() ) );
    sampleLayout->addWidget( m_sample );
    sampleLayout->addStretch();

    QHBox *box = new QHBox( mainWidget );
    QLabel *description = new QLabel( i18n( "&Foreground:" ), box );
    ColorBar *foregroundColor = new ColorBar( ksopts->ircColors.toValueVector(), box );
    description->setBuddy( foregroundColor );

    box = new QHBox( mainWidget );
    description = new QLabel( i18n( "&Background:" ), box );
    ColorBar *backgroundColor = new ColorBar( ksopts->ircColors.toValueVector(), box );
    description->setBuddy( backgroundColor );

    QPushButton *ok = actionButton( KDialogBase::Ok );
    QPushButton *cancel = actionButton( KDialogBase::Cancel );

    setTabOrder( foregroundColor, backgroundColor );
    setTabOrder( backgroundColor, ok );
    setTabOrder( ok, cancel );

    ok->setAutoDefault( false );
    cancel->setAutoDefault( false );

    connect( foregroundColor, SIGNAL( colorPicked( int ) ),
             this, SLOT( setForegroundColor( int ) ) );
    connect( backgroundColor, SIGNAL( colorPicked( int ) ),
             this, SLOT( setBackgroundColor( int ) ) );

    ok->setEnabled( false );

    updateSample();
}

QString ColorPicker::colorString() const
{
    assert( m_foregroundColor != -1 );
    QString res( QString::number( m_foregroundColor ) );
    if ( m_backgroundColor != -1 )
    {
        res += ',';
        res += QString::number( m_backgroundColor );
    }
    return res;
}

void ColorPicker::setForegroundColor( int col )
{
    QPushButton * ok =actionButton( KDialogBase::Ok );
    assert( ok );
    ok->setEnabled( true );

    m_foregroundColor = col;
    updateSample();
}

void ColorPicker::setBackgroundColor( int col )
{
    m_backgroundColor = col;
    updateSample();
}

void ColorPicker::updateSample()
{
    QColorGroup cg( colorGroup() );

    QColor col = ksopts->textColor;
    if ( m_foregroundColor != -1 )
        col = ksopts->ircColors[ m_foregroundColor ];

    cg.setColor( QColorGroup::Foreground, col );
    cg.setColor( QColorGroup::Text, col );

    if ( m_backgroundColor != -1 )
    {
        col = ksopts->ircColors[ m_backgroundColor ];
        cg.setColor( QColorGroup::Background, col );
        cg.setColor( QColorGroup::Base, col );
    }

    m_sample->setPalette( QPalette( cg, cg, cg ) );
}

ColorBar::ColorBar( const QValueVector<QColor> &colors, QWidget *parent, 
                      const char *name )
    : QFrame( parent, name, WStaticContents | WRepaintNoErase ),
      m_currentCell( -1 ), m_focusedCell( - 1 ), m_colors( colors ), 
      m_cellSize( 0 )
{
    setFrameStyle( StyledPanel | Sunken );

    updateCellSize();

    setFocusPolicy( StrongFocus );
}

void ColorBar::drawContents( QPainter *p )
{
    int x = contentsRect().x();
    int y = contentsRect().y();
    for ( unsigned int i = 0; i < m_colors.size(); ++i, x += m_cellSize )
    {
        bool isCurrentCell = ( m_currentCell != -1 && 
                               i == static_cast<uint>( m_currentCell ) );
        bool isFocusedCell = ( m_focusedCell != -1 && 
                               i == static_cast<uint>( m_focusedCell ) );
        drawCell( p, x, y, m_colors[ i ], QString::number( i ), 
                  isFocusedCell, isCurrentCell );
    }
}

void ColorBar::keyPressEvent( QKeyEvent *ev )
{
    if ( m_focusedCell == -1 ) {
        QFrame::keyPressEvent( ev );
        return;
    }

    switch ( ev->key() )
    {
        case Key_Left:
            if ( m_focusedCell > 1 )
                m_focusedCell--;
            update();
            ev->accept();
            return;
        case Key_Right:
            if ( static_cast<uint>( m_focusedCell ) < m_colors.size() - 1 )
                m_focusedCell++;
            update();
            ev->accept();
            return;
        case Key_Enter:
        case Key_Return:
        case Key_Space:
            setCurrentCell( m_focusedCell );
            update();
            ev->accept();
            return;
        default: break;
    }

    QFrame::keyPressEvent( ev );
}

void ColorBar::focusInEvent( QFocusEvent *ev )
{
    if ( ev->reason() == QFocusEvent::Tab ||
         ev->reason() == QFocusEvent::Backtab )
        m_focusedCell = 0;
    QFrame::focusInEvent( ev );
}

void ColorBar::focusOutEvent( QFocusEvent *ev )
{
    if ( ev->reason() == QFocusEvent::Tab ||
         ev->reason() == QFocusEvent::Backtab ||
         ev->reason() == QFocusEvent::Mouse )
        m_focusedCell = -1;
    QFrame::focusOutEvent( ev );
}

void ColorBar::fontChange( const QFont &oldFont )
{
    updateCellSize();
    QFrame::fontChange( oldFont );
}

void ColorBar::styleChange( QStyle &oldStyle )
{
    updateCellSize();
    QFrame::styleChange( oldStyle );
}

bool ColorBar::focusNextPrevChild( bool next )
{
    if ( next )
    {
        assert( m_focusedCell != -1 );

        if ( static_cast<uint>( m_focusedCell ) < m_colors.size() - 1 )
        {
            m_focusedCell++;
            update();
            return true;
        }
        return QFrame::focusNextPrevChild( next );
    }

    if ( m_focusedCell > 1 )
    {
        m_focusedCell--;
        update();
        return true;
    }

    return QFrame::focusNextPrevChild( next );
}

void ColorBar::mousePressEvent( QMouseEvent *ev )
{
    const QPoint &p = ev->pos();
    if ( contentsRect().contains( p ) )
    {
        m_focusedCell = p.x() / m_cellSize;
        update();
    }

    QFrame::mousePressEvent( ev );
}

void ColorBar::mouseReleaseEvent( QMouseEvent *ev )
{
    if ( m_focusedCell != -1 )
    {
        setCurrentCell( m_focusedCell );
        update();
    }
    QFrame::mouseReleaseEvent( ev );
}

void ColorBar::updateCellSize()
{
    setLineWidth( style().pixelMetric( QStyle::PM_DefaultFrameWidth, this ) );

    QFontMetrics metrics( font() );

    m_cellSize = metrics.width( QString::number( m_colors.size() ) ) +
                 ( s_indicatorSize * 2 ) +
                 ( s_focusSize * 2 ) +
                 ( s_innerMargin * 2 );

    setFixedSize( QSize( ( m_colors.size() * m_cellSize ) + ( frameWidth() * 2 ), 
                         m_cellSize + ( frameWidth() * 2 ) ) );
}

void ColorBar::setCurrentCell( int cell )
{
    m_currentCell = cell;
    emit colorPicked( cell );
}

void ColorBar::drawCell( QPainter *p, int x, int y, const QColor &color,
                         const QString &text, bool isFocusedCell, 
                         bool isCurrentCell )
{
    p->fillRect( x, y, m_cellSize, m_cellSize, color );

    QColor penColor = black;
    // ### hack
    if ( color.red() < 127 && color.green() < 127 && color.blue() < 127 )
        penColor = white;
        
    p->setPen( penColor );

    if ( isCurrentCell )
    {
        p->fillRect( x, y, m_cellSize, s_indicatorSize, penColor );
        p->fillRect( x, y + s_indicatorSize, 
                     s_indicatorSize, m_cellSize - ( 2 * s_indicatorSize ), penColor );
        p->fillRect( x, y + m_cellSize - s_indicatorSize, 
                     m_cellSize, s_indicatorSize, penColor );
        p->fillRect( x + m_cellSize - s_indicatorSize, y + s_indicatorSize,
                     s_indicatorSize, m_cellSize - ( 2 * s_indicatorSize ), penColor );
    }

    if ( isFocusedCell )
    {
        int focusRectSize = m_cellSize - ( 2 * s_indicatorSize );

        p->fillRect( x + s_indicatorSize, y + s_indicatorSize, 
                     focusRectSize, s_focusSize, penColor );
        p->fillRect( x + s_indicatorSize, y + s_indicatorSize + s_focusSize, 
                     s_focusSize, focusRectSize - ( 2 * s_focusSize ), penColor );
        p->fillRect( x + s_indicatorSize, 
                     y + m_cellSize - s_indicatorSize - s_focusSize,
                     focusRectSize, s_focusSize, penColor );
        p->fillRect( x + m_cellSize - s_indicatorSize - s_focusSize, 
                     y + s_indicatorSize + s_focusSize, 
                     s_focusSize, focusRectSize - ( 2 * s_focusSize ), penColor );
    }

    QFontMetrics metrics( p->font() );

    int offset = ( m_cellSize / 2 ) - ( metrics.width( text ) / 2 );
    p->drawText( x + offset, y + s_focusSize + s_indicatorSize + 
                 + metrics.ascent(), text );
}
 
#include "colorpicker.moc"

/* vim: et sw=4
 */
