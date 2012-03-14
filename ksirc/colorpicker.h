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
#ifndef __colorpicker_h__
#define __colorpicker_h__

#include <qvaluevector.h>
#include <qcolor.h>

#include <kdialogbase.h>

class QLineEdit;

class ColorPicker : public KDialogBase
{
    Q_OBJECT
public:
    ColorPicker( QWidget *parent, const char *name = 0 );

    QString colorString() const;

private slots:
    void setForegroundColor( int col );
    void setBackgroundColor( int col );

private:
    void updateSample();

    QLineEdit *m_sample;
    int m_foregroundColor;
    int m_backgroundColor;
};

class ColorBar : public QFrame
{
    Q_OBJECT
public:
    ColorBar( const QValueVector<QColor> &colors, QWidget *parent, const char *name = 0 );

signals:
    void colorPicked( int col );

protected:
    virtual void drawContents( QPainter *p );
    virtual void keyPressEvent( QKeyEvent *ev );
    virtual void focusInEvent( QFocusEvent *ev );
    virtual void focusOutEvent( QFocusEvent *ev );
    virtual void fontChange( const QFont &oldFont );
    virtual void styleChange( QStyle &oldStyle );
    virtual bool focusNextPrevChild( bool next );
    virtual void mousePressEvent( QMouseEvent *ev );
    virtual void mouseReleaseEvent( QMouseEvent *ev );

private:
    void updateCellSize();
    void setCurrentCell( int cell );

    void drawCell( QPainter *p, int x, int y, const QColor &color, 
                   const QString &text, bool isFocusedCell, 
                   bool isCurrentCell );

    int m_currentCell;
    int m_focusedCell;
    const QValueVector<QColor> m_colors;
    int m_cellSize;

    // ### style me
    static const int s_indicatorSize = 2;
    static const int s_focusSize = 1;
    static const int s_innerMargin = 1;
};

#endif

/* vim: et sw=4
 */
