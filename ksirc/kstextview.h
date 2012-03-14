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
#ifndef __kstextview_h__
#define __kstextview_h__

#include <qscrollview.h>
#include <qpen.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qpixmap.h>

class QTimer;
class QDragObject;

namespace KSirc
{

class TextView;

struct StringPtr
{
    StringPtr() : ptr( 0 ), len( 0 ) {}
    StringPtr( const QChar *_ptr, uint _len )
        : ptr( _ptr ), len( _len ) {}
    explicit StringPtr( const QString &s ) // use with care!
        : ptr( s.unicode() ), len( s.length() ) {}

    inline bool isNull() const { return ptr == 0; }

    // makes deep copy
    inline QString toQString() const 
    { return ( ptr && len > 0 ) ? QString( ptr, len ) : QString::null; }

    const QChar *ptr;
    uint len;
};

#define CONSTSTRING( substr ) QConstString( substr.ptr, substr.len ).string()

inline bool operator<( const StringPtr &s1, const StringPtr &s2 )
{
    return CONSTSTRING( s1 ) < CONSTSTRING( s2 );
}

inline bool operator==( const StringPtr &s1, const StringPtr &s2 )
{
    return CONSTSTRING( s1 ) == CONSTSTRING( s2 );
}

inline bool operator==( const StringPtr &s1, const char *s2 )
{
    return CONSTSTRING( s1 ) == s2;
}

class AttributeMap : public QMap<StringPtr, StringPtr>
{
public:
    AttributeMap() {}
    AttributeMap( const AttributeMap &rhs ) : QMap<StringPtr, StringPtr>( rhs ) {}
    AttributeMap &operator=( const AttributeMap &rhs )
    { QMap<StringPtr, StringPtr>::operator=( rhs ); return *this; }

    // helper for 'const char *' key...
    ConstIterator findAttribute( const char *key ) const
    {
        QString qkey( key );
        return find( StringPtr( qkey ) );
    }
    Iterator findAttribute( const char *key ) 
    {
        QString qkey( key );
        return find( StringPtr( qkey ) );
    }

    StringPtr operator[]( const char *key ) const
    {
        ConstIterator it = findAttribute( key );
        if ( it == end() )
            return StringPtr();
        return it.data();
    }
    StringPtr &operator[]( const StringPtr &key )
    {
        return QMap<StringPtr, StringPtr>::operator[]( key );
    }
};

struct Token
{
    Token() : id( -1 ) {}

    enum Id { TagOpen, Text, TagClose };
    int id;
    StringPtr value;
    AttributeMap attributes;
};

struct ItemProperties
{
    ItemProperties();
    ItemProperties( const QFont &defaultFont );
    ItemProperties( const ItemProperties &other,
                    const Token &token,
		    TextView *textView );
    ItemProperties( const ItemProperties &rhs );
    ItemProperties &operator=( const ItemProperties &rhs );

    void updateFont( const QFont &newFont );

    // these three are inherited/merged
    QFont font;
    QColor color;
    QColor selColor;
    QColor bgColor;
    QColor bgSelColor;
    bool reversed;
    // ### todo: inherit these, too
    AttributeMap attributes;
};

class TextParag;
class TextLine;
class SelectionPoint;

class Item
{
public:
    enum LayoutResetStatus { DeleteItem, KeepItem };
    enum SelectionStatus { SelectionStart = 0, InSelection, SelectionEnd, SelectionBoth, 
                           NoSelection };
    enum SelectionAccuracy { SelectExact, SelectFuzzy };

    Item( TextParag *parag, const ItemProperties &props = ItemProperties() );

    virtual ~Item();

    virtual const char *type() { return "Item"; }

    virtual void paint( QPainter &painter ) = 0;

    int width() const;

    int minWidth() const;

    int height() const;

    virtual Item *breakLine( int width );

    virtual LayoutResetStatus resetLayout() = 0;

    virtual int calcSelectionOffset( int x );

    void setSelectionStatus( SelectionStatus status ) { m_selection = status; }

    SelectionStatus selectionStatus() const { return m_selection; }

    void selectionOffsets( int &startOffset, int &endOffset );

    int maxSelectionOffset() const;

    void setLine(TextLine *line);

    // ###
    virtual StringPtr text() const;

    virtual void setProps( const ItemProperties &props );
    ItemProperties &props() { return m_props; }

    static Item *create( TextParag *parag, const Token &tok, 
                         const ItemProperties &props = ItemProperties() );

protected:
    mutable bool m_extendsDirty;
    mutable int m_minWidth;
    mutable int m_width;
    mutable int m_height;

    virtual void calcExtends() const = 0;

    SelectionStatus m_selection;
    TextLine  *m_line;
    TextParag *m_parag;
    ItemProperties m_props;
};

class TextChunk : public Item
{
public:
    TextChunk( TextParag *parag, const StringPtr &text, const ItemProperties &props );

    virtual const char *type() { return "TextChunk"; }

    virtual void paint( QPainter &painter );

    virtual Item *breakLine( int width );

    virtual LayoutResetStatus resetLayout();

    virtual int calcSelectionOffset( int x );

    virtual StringPtr text() const;

    virtual void setProps( const ItemProperties &props );

protected:
    virtual void calcExtends() const;

private:
    StringPtr breakInTheMiddle( int width );
    Item *hardBreak( const StringPtr &rightHandSide );

    void paintSelection( QPainter &p );
    int paintSelection( QPainter &p, int x, const StringPtr &text );
    int paintText( QPainter &p, int x, const StringPtr &text );

    void mergeSelection( TextChunk *child, SelectionPoint *selection );

    StringPtr m_text;
    uint m_originalTextLength;
    QFontMetrics m_metrics;
    class TextChunk *m_parent;
};

class ImageItem : public Item
{
public:
    ImageItem( TextParag *parag, const QPixmap &pixmap );

    virtual const char *type() { return "Image"; }

    virtual void paint( QPainter &painter );

    virtual LayoutResetStatus resetLayout();

protected:
    virtual void calcExtends() const;

private:
    QPixmap m_pixmap;
};

class Tokenizer
{
public:
    struct TagIndex
    {
        enum Type { Open, Close };
        TagIndex() : index( 0 ), type( -1 ) {}
        TagIndex( int _index, int _type )
            : index( _index ), type( _type ) {}
        uint index;
        int type;
    };
    typedef QValueList<TagIndex> TagIndexList;
 
    // preprocessed string
    struct PString
    {
        QString data;
        TagIndexList tags;
    };
    
    Tokenizer( PString &text );

    static PString preprocess( const QString &richText );

    static QString convertToRichText( const PString &ptext );

    bool parseNextToken( Token &tok );

private:
    void parseTag( const StringPtr &text,
                   StringPtr &tag,
                   AttributeMap &attributes );

    static TagIndexList scanTagIndices( const QString &text );
    static void resolveEntities( QString &text, TagIndexList &tags );

    enum TagParsingState { ScanForName, ScanForEqual, ScanForValue };
    
    QString &m_text;
    TagIndexList m_tags;
    TagIndexList::ConstIterator m_lastTag;
    bool m_textBeforeFirstTagProcessed;
    bool m_done;

    Tokenizer( const Tokenizer & );
    Tokenizer &operator=( const Tokenizer & );
};

class SelectionPoint;

class TextLine
{
public:
    enum LayoutPolicy { NoUpdate, UpdateMaxHeight };

    TextLine();
    // tranfers ownership of items! make sure that 'items' does not
    // have autodeletion enabled!
    TextLine( const QPtrList<Item> &items );

    int maxHeight() const { return m_maxHeight; }

    QString updateSelection( const SelectionPoint &start, const SelectionPoint &end );
    void clearSelection();

    // transfers ownership
    void appendItem( Item *i, int layoutUpdatePolicy = NoUpdate );

    bool isEmpty() const { return m_items.isEmpty(); }

    Item *resetLayout( QPtrList<Item> &remainingItems);

    void paint( QPainter &p, int y );

    Item *itemAt( int px, SelectionPoint *selectionInfo, 
                  Item::SelectionAccuracy accuracy = Item::SelectExact );

    QPtrListIterator<Item> iterator() const { return QPtrListIterator<Item>( m_items ); }

    QString plainText() const;

    void fontChange( const QFont &newFont );

private:
    QPtrList<Item> m_items;
    int m_maxHeight;
};

class SelectionPoint;

class TextParag
{
public:
    TextParag( TextView *textView, const QString &richText );
    
    ~TextParag();

    void layout( int width );

    void paint( QPainter &p, int y, int maxY );

    inline void setLayouted( bool l ) { m_layouted = l; }
    inline bool isLayouted() const { return m_layouted; }

    inline int minWidth() const { return m_minWidth; }
    inline int height() const { return m_height; }

    Item *itemAt( int px, int py, SelectionPoint *selectionInfo,
                  Item::SelectionAccuracy accuracy = Item::SelectExact );

    TextView *textView() const { return m_textView; }

    QString updateSelection( const SelectionPoint &start, const SelectionPoint &end );

    void clearSelection();

    void setRichText( const QString &richText );

    Tokenizer::PString processedRichText() const { return m_processedRichText; }

    QString plainText() const;

    void fontChange( const QFont &newFont );

private:
    Tokenizer::PString m_processedRichText;
    QPtrList<TextLine> m_lines;
    bool m_layouted;
    int m_height;
    int m_minWidth;
    TextView *m_textView;

    struct Tag
    {
        Tag() {}
        Tag( const StringPtr &_name, const ItemProperties &_props )
            : name( _name ), props( _props ) {}

        StringPtr name;
        ItemProperties props;
    };

    TextParag( const TextParag & );
    TextParag &operator=( const TextParag & );
};

struct SelectionPoint
{
    SelectionPoint() : item( 0 ), line( 0 ), parag( 0 ), offset( 0 ) {}
    Item *item;
    TextLine *line;
    TextParag *parag;
    uint offset;
    QPoint pos;
};

class TextParagIterator
{
    friend class TextView;
public:
    TextParagIterator( const TextParagIterator &rhs )
        : m_paragIt( rhs.m_paragIt ) {}
    TextParagIterator &operator=( const TextParagIterator &rhs )
    { m_paragIt = rhs.m_paragIt; return *this; }

    QString richText() const;
    void setRichText( const QString &richText );

    QString plainText() const;

    bool atEnd() const { return m_paragIt.current() == 0; }

    TextParagIterator &operator++() { ++m_paragIt; return *this; }
    TextParagIterator &operator++( int steps ) { m_paragIt += steps; return *this; }
    TextParagIterator &operator--() { --m_paragIt; return *this; }
    TextParagIterator &operator--( int steps ) { m_paragIt -= steps; return *this; }

protected:
    TextParagIterator( const QPtrListIterator<TextParag> &paragIt )
        : m_paragIt( paragIt ) {}

private:
    QPtrListIterator<TextParag> m_paragIt;
};

class ContentsPaintAlgorithm
{
public:
    ContentsPaintAlgorithm( const QPtrListIterator<TextParag> &paragIt,
                            QWidget *viewport, QPixmap &paintBuffer,
                            QPainter &painter, int clipX, int clipY, int clipHeight );

    void paint();

private:
    int goToFirstVisibleParagraph();
    int paint( QPainter &bufferedPainter, int currentY );
    int adjustYAndIterator( int startY, int currentY, int nextY );

    QPtrListIterator<TextParag> m_paragIt;
    QWidget *m_viewport;
    QPixmap &m_paintBuffer;
    QPainter &m_painter;
    int m_clipX, m_clipY, m_clipHeight;
    int m_overshoot;
};

class TextView : public QScrollView
{
    Q_OBJECT
    friend class Item;
    friend class TextChunk;
    friend class TextParag;
    friend class TextParagIterator;
public:
    TextView( QWidget *parent, const char *name  = 0 );
    virtual ~TextView();

    virtual void clear();

    TextParagIterator appendParag( const QString &richText );
    
    bool removeParag( const TextParagIterator &parag );

    void clearSelection( bool repaint = false ); // ### re-consider the repaint arg...

    QString selectedText() const { return m_selectedText; }

    TextParagIterator firstParag() const;

    QString plainText() const;

    QColor linkColor() const;
    void setLinkColor( const QColor &linkColor );

    void scrollToBottom( bool force = false );

signals:
    void selectionChanged();
    void pasteReq(const QString&);
    void linkClicked( const QMouseEvent *ev, const QString &url );

public slots:
    void copy();

protected slots:
    void scrolling(int value);

protected:
    virtual void viewportResizeEvent( QResizeEvent *ev );
    virtual void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    virtual void contentsMousePressEvent( QMouseEvent *ev );
    virtual void contentsMouseMoveEvent( QMouseEvent *ev );
    virtual void contentsMouseReleaseEvent( QMouseEvent *ev );
    virtual void fontChange( const QFont & );

    virtual void startDrag();

    virtual QDragObject *dragObject( const QString &dragURL );

private slots:
    void autoScroll();

private:
    void emitLinkClickedForMouseEvent( QMouseEvent *ev );

    void startAutoScroll();

    void stopAutoScroll();

    void selectionOffsets( int &startOffset, int &endOffset );

    void updateSelectionOrder();

    QString updateSelection( const SelectionPoint &start, const SelectionPoint &end );

    SelectionPoint *selectionStart();
    SelectionPoint *selectionEnd();

    void layout( bool force = true );

    Item *itemAt( const QPoint &pos, SelectionPoint *selectionInfo = 0,
                  Item::SelectionAccuracy accuracy = Item::SelectExact );

    void clearSelectionInternal();

    void contentsChange(int heightChange, bool force = false);

    QPtrList<TextParag> m_parags;
    QPixmap m_paintBuffer;

    SelectionPoint m_selectionMaybeStart;
    SelectionPoint m_selectionStart;
    SelectionPoint m_selectionEnd;
    bool m_selectionEndBeforeStart;

    QTimer *m_autoScrollTimer;

    QString m_selectedText;

    QPoint m_dragStartPos;
    QString m_dragURL;
    bool m_mousePressed : 1;
    bool m_mmbPressed : 1;
    QColor m_linkColor;
    QColor m_selectionBackgroundColor;

    int m_height;
    bool m_inScroll;
    int m_lastScroll;
};

} // namespace KSirc

#endif
/*
 * vim: et sw=4
 */
