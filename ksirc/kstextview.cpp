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

#include "kstextview.h"
#include "stringparserstate.h"

#include <qpainter.h>
#include <qvaluestack.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <qclipboard.h>
#include <qdict.h>
#include <kcharsets.h>
#include <kapplication.h>
#include <kmimesourcefactory.h>
#include <kcursor.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <assert.h>

using namespace KSirc;

typedef StringParserState<QChar> ParsingState;

static const int PaintBufferExtend = 128;

// temporary(!)
static QDict<QPixmap> *ksTextViewPixmapDict = 0;
static void cleanupKSTextViewPixmapDict()
{
    delete ksTextViewPixmapDict;
    ksTextViewPixmapDict = 0;
}

QPixmap ksTextViewLoadPixmap( const QString &icon )
{
    if ( !ksTextViewPixmapDict )
    {
        ksTextViewPixmapDict = new QDict<QPixmap>;
        ksTextViewPixmapDict->setAutoDelete( true );
        qAddPostRoutine( cleanupKSTextViewPixmapDict );
    }

    QPixmap *pix = ksTextViewPixmapDict->find( icon );
    if ( !pix )
    {
        QImage img;

        const QMimeSource *src = kapp->mimeSourceFactory()->data( icon, QString::null );
        if ( !src || !QImageDrag::decode( src, img ) || img.isNull() )
            return QPixmap();

        pix = new QPixmap( img );
        ksTextViewPixmapDict->insert( icon, pix );
    }
    return *pix;
}

Item::Item( TextParag *parag, const ItemProperties &props )
    : m_extendsDirty( true ), m_minWidth( -1 ), m_width( - 1 ),
      m_height( - 1 ), m_selection( NoSelection ), m_line( 0 ),
      m_parag( parag ), m_props( props )
{
}

Item::~Item()
{
}

int Item::width() const
{
    if ( m_extendsDirty )
    {
        calcExtends();
        m_extendsDirty = false;
    }
    return m_width;
}

int Item::minWidth() const
{
    if ( m_extendsDirty )
    {
        calcExtends();
        m_extendsDirty = false;
    }
    return m_minWidth;
}

int Item::height() const
{
    if ( m_extendsDirty )
    {
        calcExtends();
        m_extendsDirty = false;
    }
    return m_height;
}

Item *Item::breakLine( int )
{
    return 0; // can't break by default...
}

int Item::calcSelectionOffset( int x )
{
    return x; // default ...
}

void Item::selectionOffsets( int &startOffset, int &endOffset )
{
    m_parag->textView()->selectionOffsets( startOffset, endOffset );
}

int Item::maxSelectionOffset() const
{
    return text().len - 1;
}

StringPtr Item::text() const
{
    return StringPtr();
}

void Item::setProps( const ItemProperties &props )
{
    m_props = props;
    m_extendsDirty = true;
}

Item *Item::create( TextParag *parag, const Token &tok, const ItemProperties &props )
{
    assert( tok.id != Token::TagClose );

    if ( tok.id == Token::Text )
        return new TextChunk( parag, tok.value, props );

    if ( tok.value == "img" )
    {
        QString url = CONSTSTRING( tok.attributes[ "src" ] );
        if ( url.isEmpty() )
            return 0;

        QPixmap pixmap = ksTextViewLoadPixmap( url );
        if ( pixmap.isNull() )
            return 0;

        return new ImageItem( parag, pixmap );
    }

    return 0;
}

void Item::setLine(TextLine *line)
{
  m_line = line;
}

TextChunk::TextChunk( TextParag *parag, const StringPtr &text, const ItemProperties &props )
    : Item( parag, props ), m_text( text ), m_originalTextLength( text.len ),
      m_metrics( props.font ), m_parent(0)
{
}

void TextChunk::paint( QPainter &p )
{
    p.setFont( m_props.font );

    if ( m_selection == NoSelection )
        paintText( p, 0, m_text );
    else
        paintSelection( p );
}

Item *TextChunk::breakLine( int width )
{
    ParsingState state( m_text.ptr, m_text.len );

    const int requestedWidth = width;
    const int spaceWidth = m_metrics.width( ' ' );

    state.skip( ' ' );

    if ( state.atEnd() ) // eh?
        return 0;

    StringPtr firstWord; firstWord.ptr = state.current();
    firstWord.len = state.advanceTo( ' ' );

    const int firstWordWidth = m_metrics.width( CONSTSTRING( firstWord ) );

    if ( !state.atBegin() ) // some leading spaces?
        width -= spaceWidth;

    width -= firstWordWidth;

    if ( width < 0 ) {
        StringPtr rightHandSide = breakInTheMiddle( requestedWidth );
        if ( rightHandSide.isNull() )
	    return 0;
        return hardBreak( rightHandSide );
    }

    while ( !state.atEnd() )
    {
        bool spaceSeen = state.skip( ' ' ) > 0;

        if ( state.atEnd() )
            break;

        StringPtr word; word.ptr = state.current();
        word.len = state.advanceTo( ' ' );

        const int wordWidth = m_metrics.width( CONSTSTRING( word ) );

        if ( spaceSeen )
        {
            width -= spaceWidth;
            spaceSeen = false;
        }

        width -= wordWidth;
        if ( width > 0 )
            continue;

	StringPtr split( word.ptr, state.end() - word.ptr );
        return hardBreak( split );
    }
    return 0;
}

Item::LayoutResetStatus TextChunk::resetLayout()
{

    if ( m_originalTextLength == 0 )
    {
        if ( m_parent )
        {
            if ( m_selection == SelectionStart )
                m_parent->mergeSelection( this, m_parag->textView()->selectionStart() );
            else if ( m_selection == SelectionEnd )
                m_parent->mergeSelection( this, m_parag->textView()->selectionEnd() );
            else if ( m_selection == SelectionBoth )
            {
                m_parent->mergeSelection( this, m_parag->textView()->selectionStart() );
                m_parent->mergeSelection( this, m_parag->textView()->selectionEnd() );
            }
        }

        return DeleteItem;
    }

    m_extendsDirty |= ( m_text.len != m_originalTextLength );

    m_text.len = m_originalTextLength;
    return KeepItem;
}

int TextChunk::calcSelectionOffset( int x )
{
    // ### how to optimize?

    QConstString tmp( m_text.ptr, m_text.len );
    const QString &s = tmp.string();

    uint i = 0;
    int px = 0;
    for (; i < m_text.len; ++i )
    {
        const int partialWidth = m_metrics.width( s, i + 1 );
        if ( px <= x && x <= partialWidth )
            return i;
        px = partialWidth;
    }

    kdDebug(5008) << "calcSelectionOffset bug width:" << width() <<  " partialWidth: " << m_metrics.width( s, i + 1 )<< endl;
    //assert( false );
    return m_text.len-1;
}

StringPtr TextChunk::text() const
{
    return m_text;
}

void TextChunk::setProps( const ItemProperties &props )
{
    Item::setProps( props );
    m_metrics = QFontMetrics( props.font );
}

void TextChunk::calcExtends() const
{
    QConstString tmp( m_text.ptr, m_text.len );
    const QString &text = tmp.string();

    m_width = m_metrics.width( text );
    m_height = m_metrics.lineSpacing();

    //m_minWidth = 0;

    m_minWidth = m_metrics.charWidth( text, 1 );

    /*
    ParsingState state( m_text.ptr, m_text.len );

    state.skip( ' ' );

    if ( state.atEnd() ) // eh?
        return;

    StringPtr firstWord; firstWord.ptr = state.current();
    firstWord.len = state.advanceTo( ' ' );

    m_minWidth = m_metrics.width( CONSTSTRING( firstWord ) );
    */
}

StringPtr TextChunk::breakInTheMiddle( int width )
{
    QConstString tmp( m_text.ptr, m_text.len );
    const QString &s = tmp.string();

    uint i = 0;
    for (; i < m_text.len; ++i )
    {
        const int partialWidth = m_metrics.width( s, i + 1 );
        if ( partialWidth >= width ) {
            if ( i == 0 )
                return StringPtr();
            return StringPtr( m_text.ptr + i, m_text.len - i );
        }
    }

    return StringPtr();
}

Item *TextChunk::hardBreak( const StringPtr &rightHandSide )
{
    TextChunk *chunk = new TextChunk( m_parag, rightHandSide, m_props );
    chunk->m_originalTextLength = 0; // ### hack... You make the last line dynamic so if it's 1 word it doesn't chop itself up
    if(m_parent == 0x0)
	chunk->m_parent = this;
    else
        chunk->m_parent = m_parent;

    m_text.len = rightHandSide.ptr - m_text.ptr;
    m_extendsDirty = true;

    SelectionPoint *selection = 0;
    if ( m_selection == SelectionStart )
        selection = m_parag->textView()->selectionStart();
    else if ( m_selection == SelectionEnd )
        selection = m_parag->textView()->selectionEnd();
    else if ( m_selection == SelectionBoth ) {
        SelectionPoint *selStart = m_parag->textView()->selectionStart();
        SelectionPoint *selEnd = m_parag->textView()->selectionEnd();

        if ( selStart->offset >= m_text.len ) {
            selStart->offset -= m_text.len;
            selEnd->offset -= m_text.len;
            selStart->item = selEnd->item = chunk;
            chunk->setSelectionStatus( m_selection );
            m_selection = NoSelection;
        } else if ( selEnd->offset >= m_text.len ) {
            selEnd->offset -= m_text.len;
            selEnd->item = chunk;
            chunk->setSelectionStatus( SelectionEnd );
            m_selection = SelectionStart;
        }
    }

    if ( selection && selection->offset >= m_text.len ) {
        selection->offset -= m_text.len;
        selection->item = chunk;
        chunk->setSelectionStatus( m_selection );
        m_selection = NoSelection;
    }

    return chunk;
}

void TextChunk::paintSelection( QPainter &p )
{
    int selectionStart = 0;
    int selectionEnd = 0;
    selectionOffsets( selectionStart, selectionEnd );

    if ( m_selection == SelectionStart )
    {
        const int width = paintText( p, 0, StringPtr( m_text.ptr, selectionStart ) );
        paintSelection( p, width, StringPtr( m_text.ptr + selectionStart,
                                             m_text.len - selectionStart ) );
    }
    else if ( m_selection == InSelection )
        paintSelection( p, 0, m_text );
    else if ( m_selection == SelectionEnd )
    {
        const int width = paintSelection( p, 0, StringPtr( m_text.ptr, selectionEnd + 1 ) );
        paintText( p, width, StringPtr( m_text.ptr + selectionEnd + 1,
                                        m_text.len - selectionEnd - 1 ) );
    }
    else if ( m_selection == SelectionBoth )
    {
        int width = paintText( p, 0, StringPtr( m_text.ptr, selectionStart ) );
        width += paintSelection( p, width, StringPtr( m_text.ptr + selectionStart,
                                                      selectionEnd - selectionStart + 1 ) );
        paintText( p, width, StringPtr( m_text.ptr + selectionEnd + 1,
                                        m_text.len - selectionEnd - 1 ) );
    }
}

int TextChunk::paintSelection( QPainter &p, int x, const StringPtr &text ) 
{
    QConstString constString( text.ptr, text.len ); 
    const QString &str = constString.string();

    const int width = m_metrics.width( str );

    const QColorGroup &cg = m_parag->textView()->colorGroup();

    if (m_props.bgSelColor.isValid())
        p.fillRect( x, 0, width, height(), m_props.bgSelColor );
    else
        p.fillRect( x, 0, width, height(), cg.highlight() );
    
    if (m_props.selColor.isValid())
	p.setPen( m_props.selColor );
    else
	p.setPen( cg.highlightedText() );

    p.drawText( x, m_metrics.ascent(), str );

    return width;
}

int TextChunk::paintText( QPainter &p, int x, const StringPtr &text )
{
    QConstString constString( text.ptr, text.len );
    const QString &str = constString.string();

    const int width = m_metrics.width( str );

    if ( m_props.bgColor.isValid() )
        p.fillRect( x, 0, width, height(), m_props.bgColor );

    if ( m_props.color.isValid() )
        p.setPen( m_props.color );
    else
	p.setPen( m_parag->textView()->foregroundColor() );

    p.drawText( x, m_metrics.ascent(), str );

    return width;
}

void TextChunk::mergeSelection( TextChunk *child, SelectionPoint *selection )
{
    selection->offset += child->m_text.ptr - m_text.ptr;

    if(selection->offset > m_originalTextLength){
	kdDebug(5008) << "Child: " << child->m_text.toQString() << " Parent: " << m_text.toQString() << endl;
        kdDebug(5008) << "Length all wrong!" << endl;
	//assert(0);
    }

    selection->item = this;

    if ( ( m_selection == SelectionStart && child->selectionStatus() == SelectionEnd ) ||
         ( m_selection == SelectionEnd && child->selectionStatus() == SelectionStart ) )
        m_selection = SelectionBoth;
    else
        m_selection = child->selectionStatus();
}

ImageItem::ImageItem( TextParag *parag, const QPixmap &pixmap )
    : Item( parag ), m_pixmap( pixmap )
{
}

void ImageItem::paint( QPainter &painter )
{
    int y = 0;
    if(m_line) {
        y = (m_line->maxHeight() - m_pixmap.height())/2;
    }

    if ( m_selection != NoSelection ) {
      int h;

      if(m_line)
	h = m_line->maxHeight();
      else
        h = height();

      if (m_props.bgSelColor.isValid())
	painter.fillRect( 0, 0, width(), h, m_props.bgSelColor );
      else {
	const QColorGroup &cg = m_parag->textView()->colorGroup();
	painter.fillRect( 0, 0, width(), h, cg.highlight() );
      }
    }

    painter.drawPixmap( 0, y, m_pixmap );
}

Item::LayoutResetStatus ImageItem::resetLayout()
{
    // nothin' to do
    return KeepItem;
}

void ImageItem::calcExtends() const
{
    m_width = m_minWidth = m_pixmap.width();
    m_height = m_pixmap.height();
}

Tokenizer::Tokenizer( PString &text )
    : m_text( text.data ), m_tags( text.tags ),
      m_textBeforeFirstTagProcessed( false ), m_done( false )
{
    //qDebug( "Tokenizer::Tokenizer( %s )", m_text.ascii() );

    m_lastTag = m_tags.begin();

    if ( !m_tags.isEmpty() ) {
        if ( ( *m_tags.begin() ).type != TagIndex::Open ) {
            qDebug( "something went awfully wrong! bailing out with an assertion" );
            qDebug( "text input was: %s", text.data.ascii() );
        }
        assert( ( *m_tags.begin() ).type == TagIndex::Open );
    }
}

Tokenizer::PString Tokenizer::preprocess( const QString &richText )
{
    PString result;
    result.data = richText;
    result.tags = scanTagIndices( result.data );
    resolveEntities( result.data, result.tags );
    return result;
}

QString Tokenizer::convertToRichText( const PString &ptext )
{
    if ( ptext.tags.isEmpty() )
        return ptext.data;

    QString result = ptext.data;
    uint i = 0;
    TagIndexList tags = ptext.tags;
    TagIndexList::Iterator it = tags.begin();
    TagIndexList::Iterator end = tags.end();

    for (; i < result.length(); ++i )
    {
        if ( it != end && i == (*it).index )
        {
            ++it;
            continue;
        }

        unsigned short indexAdjustment = 0;
        const QChar ch = result[ i ];
        // ### incomplete!
        /* this doesn't work quite right (when resolving back
         * KCharSet's fromEntity breaks)
        if ( ch == '<' || ch == '>' || ch == '&' )
        {
            QString entity = KGlobal::charsets()->toEntity( ch );
            indexAdjustment = entity.length() - 1;
            result.replace( i, 1, entity );
        }
        */
        if ( ch == '<' )
        {
            result.replace( i, 1, "&lt;" );
            indexAdjustment = 3;
        }
        else if ( ch == '>' )
        {
            result.replace( i, 1, "&gt;" );
            indexAdjustment = 3;
        }
        else if ( ch == '&' )
        {
            result.replace( i, 1, "&amp;" );
            indexAdjustment = 4;
        }

        if ( indexAdjustment > 0 )
        {
            TagIndexList::Iterator tmpIt = it;
            for (; tmpIt != end; ++tmpIt )
                (*tmpIt).index += indexAdjustment;
        }
    }

    return result;
}

bool Tokenizer::parseNextToken( Token &tok )
{
    if ( m_done )
    {
        //qDebug( "Tokenizer: premature end" );
        return false;
    }

    if ( m_tags.isEmpty() )
    {
        tok.id = Token::Text;
        tok.attributes.clear();
        tok.value = StringPtr( m_text );
        m_done = true;
        return true;
    }

    TagIndexList::ConstIterator it = m_lastTag;
    ++it;

    if ( it == m_tags.end() )
    {
        m_done = true;

        const uint idx = (*m_lastTag).index + 1;
        if ( idx >= m_text.length() )
            return false;

        tok.id = Token::Text;
        tok.value = StringPtr( m_text.unicode() + idx,
                               m_text.length() - idx );
        tok.attributes.clear();
        return true;
    }

    // text before first tag opening?
    if ( m_lastTag == m_tags.begin() &&
         (*m_lastTag).index > 0 &&
         !m_textBeforeFirstTagProcessed )
    {
        tok.id = Token::Text;
        tok.attributes.clear();
        tok.value = StringPtr( m_text.unicode(),
                               (*m_lastTag).index );

        m_textBeforeFirstTagProcessed = true;
        return true;
    }

    const uint index = (*it).index;
    const int type = (*it).type;
    const uint lastIndex = (*m_lastTag).index;
    const uint lastType = (*m_lastTag).type;

    assert( lastIndex < index );

    // a tag
    if ( lastType == TagIndex::Open &&
         type == TagIndex::Close )
    {
        const QChar *tagStart = m_text.unicode() + lastIndex + 1;
        uint tagLen = ( index - 1 ) - ( lastIndex + 1 ) + 1;
        // </bleh> ?
        if ( *tagStart == '/' )
        {
            ++tagStart;
            --tagLen;
            tok.id = Token::TagClose;
        }
        else
            tok.id = Token::TagOpen;

        parseTag( StringPtr( tagStart, tagLen ), tok.value, tok.attributes );

        m_lastTag = it;
        return true;
    }
    // text
    else if ( lastType == TagIndex::Close &&
              type == TagIndex::Open )
    {
        tok.id = Token::Text;
        tok.attributes.clear();
        tok.value = StringPtr( m_text.unicode() + lastIndex + 1,
                              ( index - 1 ) - lastIndex );

        m_lastTag = it;
        return true;
    }
    else {
        qDebug( "EEK, this should never happen. input text was: %s", m_text.ascii() );
        assert( false );
    }

    return false;
}

Tokenizer::TagIndexList Tokenizer::scanTagIndices( const QString &text )
{
    const QChar *start = text.unicode();
    const QChar *p = start;
    const QChar *endP = p + text.length();
    bool quoted = false;
    bool inTag = false;

    TagIndexList tags;

    for (; p < endP; ++p )
    {
	const QChar ch = *p;
	if ( ch == '"' && inTag )
	{
	    quoted = quoted ? false : true;
            continue;
	}

	if( quoted )
            continue;

	if ( ch == '<' )
	{
            inTag = true;
	    tags.append( TagIndex( p - start, TagIndex::Open ) );
	    continue;
        }
        else if ( ch == '>' )
	{
            inTag = false;
	    tags.append( TagIndex( p - start, TagIndex::Close ) );
	    continue;
        }
    }

    return tags;
}

void Tokenizer::resolveEntities( QString &text, TagIndexList &tags )
{
    const QChar *p = text.unicode();
    const QChar *endP = p + text.length();
    uint i = 0;
    bool scanForSemicolon = false;
    const QChar *ampersand = 0;
    TagIndexList::Iterator tagInfoIt = tags.begin();
    TagIndexList::Iterator tagsEnd = tags.end();

    for (; p < endP; ++p, ++i )
    {
        if ( tagInfoIt != tagsEnd &&
              i > (*tagInfoIt).index )
            ++tagInfoIt;

        const QChar ch = *p;

        if ( ch == '&' ) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if ( ch != ';' || !scanForSemicolon )
            continue;

        assert( ampersand );

        scanForSemicolon = false;

        const QChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if ( entityLength == 0 )
            continue;

        const QChar entityValue = KCharsets::fromEntity( QConstString( entityBegin, entityLength ).string() );
        if ( entityValue.isNull() )
            continue;

        const uint ampersandPos = ampersand - text.unicode();

        text[ ampersandPos ] = entityValue;
        text.remove( ampersandPos + 1, entityLength + 1 );
        i = ampersandPos;
        p = text.unicode() + i;
        endP = text.unicode() + text.length();
        ampersand = 0;

        uint adjustment = entityLength + 1;
        TagIndexList::Iterator it = tagInfoIt;
        for (; it != tags.end(); ++it )
            (*it).index -= adjustment;
    }
}

void Tokenizer::parseTag( const StringPtr &text,
                          StringPtr &tag,
                          AttributeMap &attributes )
{
    assert( text.len > 0 );

    attributes.clear();
    tag = StringPtr();

    const QChar *p = text.ptr;
    const QChar *endP = p + text.len;
    const QChar *start = p;

    int state = ScanForName;

    StringPtr key;

    while ( p < endP )
    {
        const QChar ch = *p;

        if ( ch == ' ' )
        {
            start = ++p;
            continue;
        }

       if ( state == ScanForEqual )
       {
           if ( ch == '=' )
           {
               state = ScanForValue;
               ++p;
               continue;
           }
           state = ScanForName;
       }

       if ( state == ScanForValue )
       {
           if ( ch == '=' ) // eh?
           {
               qDebug( "EH?" );
               ++p;
               continue;
           }

           if ( key.isNull() )
           {
               qDebug( "Tokenizer: Error, attribute value without key." );
               // reset
               state = ScanForName;
               ++p;
               continue;
           }

           start = 0x0;
	   if ( *p == '"' )
	   {
	       ++p;
	       start = p;
	       while ( p < endP && *p != '"' ) {
		   ++p;
	       }
	   }
	   else {
	       while ( p < endP && *p != ' ' && *p != '>') {
		   if(!start)
		       start = p;
		   ++p;
	       }
	   }

	   if(start == 0x0) {
	       state = ScanForName;
	       qDebug( "Never found start \" in tag." );
               ++p;
	       continue;
	   }

	   const QChar *valueEnd = p;

           StringPtr value = StringPtr( start, valueEnd - start );
           attributes[ key ] = value;

           if(*p == '"')
	       ++p; // move p beyond the last "
           state = ScanForName;
           continue;
       }

       if ( state == ScanForName )
       {
	   while ( p < endP && *p != ' ' && *p != '=' ){
	       ++p;
	   }

           key = StringPtr( start, p - start );

           if ( tag.isNull() )
               tag = key;
           else
               attributes[ key ] = StringPtr();

           state = ScanForEqual;
           continue;
       }

       assert( false ); // never reached.
    }

/*
    kdDebug(5008) << "tagName: " << tag.toQString() << endl;
    AttributeMap::ConstIterator it = attributes.begin();
    for (; it != attributes.end(); ++it )
	kdDebug(5008) << "attribute: " << it.key().toQString() <<
	    " -> " << it.data().toQString() << endl;
*/
}

ItemProperties::ItemProperties()
    : reversed( false )
{
}

ItemProperties::ItemProperties( const QFont &defaultFont )
    : font( defaultFont ), reversed( false )
{
}

ItemProperties::ItemProperties( const ItemProperties &other,
                                const Token &token,
                                TextView *textView )
    : attributes( token.attributes )
{
    // inherit
    font = other.font;
    color = other.color;
    bgColor = other.bgColor;
    bgSelColor = other.bgSelColor;
    selColor = other.selColor;
    reversed = other.reversed;

    if ( token.value == "b" )
        font.setBold( true );
    else if ( token.value == "i" )
        font.setItalic( true );
    else if ( token.value == "u" )
	font.setUnderline( true );
    else if ( token.value == "r" ) {
	reversed = true;
	if(other.bgColor.isValid())
	    color = other.bgColor;
	else
	    color = textView->paletteBackgroundColor();

	if(other.color.isValid())
	    bgColor = other.color;
	else
	    bgColor = textView->foregroundColor();
    }
    else if ( token.value == "font" )
    {
        StringPtr colAttr = attributes[ "color" ];
        if ( !colAttr.isNull() )
        {
            QColor col( CONSTSTRING( colAttr ) );
	    if ( col.isValid() ){
                if(!reversed)
		    color = col;
		else
                    bgColor = col;
	    }
        }
        colAttr = attributes[ "bgcolor" ];
        if ( !colAttr.isNull() )
        {
            QColor col( CONSTSTRING( colAttr ) );
	    if ( col.isValid() ) {
                if(!reversed)
		    bgColor = col;
		else
                    color = col;
	    }
        }
    }
    else if ( token.value == "a" )
    {
        color = textView->linkColor();
        font.setUnderline( true );
    }
}

ItemProperties::ItemProperties( const ItemProperties &rhs )
{
    ( *this ) = rhs;
}

ItemProperties &ItemProperties::operator=( const ItemProperties &rhs )
{
    font = rhs.font;
    color = rhs.color;
    bgColor = rhs.bgColor;
    bgSelColor = rhs.bgSelColor;
    selColor = rhs.selColor;
    reversed = rhs.reversed;
    attributes = rhs.attributes;
    return *this;
}

void ItemProperties::updateFont( const QFont &newFont )
{
    QFont f = newFont;
    f.setUnderline( font.underline() );
    f.setBold( font.bold() );
    f.setItalic( font.italic() );
    font = f;
}

TextLine::TextLine()
    : m_maxHeight( 0 )
{
    m_items.setAutoDelete( true );
}

TextLine::TextLine( const QPtrList<Item> &items )
    : m_maxHeight( 0 )
{
    m_items.setAutoDelete( true );

    assert( !items.autoDelete() );

    QPtrListIterator<Item> it( items );
    for (; it.current(); ++it )
        appendItem( it.current(), UpdateMaxHeight );
}

QString TextLine::updateSelection( const SelectionPoint &start, const SelectionPoint &end )
{
    QString selectedText;

    // fixes a crash where because of an empty list i becomes null
    if ( m_items.isEmpty() )
        return QString::null;

    if ( start.line == this )
    {
        const int idx = m_items.findRef( start.item );
        assert( idx != -1 );
    }
    else
        m_items.first();

    Item *i = m_items.current();

    Item *lastItem = 0;

    if ( end.line == this )
    {
        const int oldCurrent = m_items.at();

        const int idx = m_items.findRef( end.item );
        assert( idx != -1 );
        lastItem = m_items.next();

        m_items.at( oldCurrent );
    }

    for (; i != lastItem && i!=0L; i = m_items.next() )
    {
        if ( i == start.item )
        {
            i->setSelectionStatus( Item::SelectionStart );

            StringPtr txt = i->text();
            if ( !txt.isNull() )
                selectedText += QString( txt.ptr + start.offset,
                                         txt.len - start.offset );
        }
        else if ( i == end.item )
        {
            i->setSelectionStatus( Item::SelectionEnd );

            StringPtr txt = i->text();
            if ( !txt.isNull() )
                selectedText += QString( txt.ptr, end.offset + 1 );
        }
        else
        {
	    i->setSelectionStatus( Item::InSelection );

            selectedText += i->text().toQString();
        }
    }

    return selectedText;
}

void TextLine::clearSelection()
{
    Item *i = m_items.first();
    for (; i; i = m_items.next() )
        i->setSelectionStatus( Item::NoSelection );
}

void TextLine::appendItem( Item *i, int layoutUpdatePolicy )
{
    m_items.append( i );
    i->setLine(this);

    if ( layoutUpdatePolicy == UpdateMaxHeight )
        m_maxHeight = kMax( m_maxHeight, i->height() );
}

Item *TextLine::resetLayout( QPtrList<Item> &remainingItems)
{
    Item *lastLineItem = m_items.getLast();
    Item *it = m_items.first();
    // We have to use the tobeDel structure or we call resetLayout on the Item
    // twice for each item we want to delete
    QPtrList<Item> tobeDel;
    while ( it )
    {
        if ( it->resetLayout() == Item::KeepItem )
            remainingItems.append( m_items.take() );
        else 
            tobeDel.append( m_items.take() );

        it = m_items.current();
    }
    m_items = tobeDel;
    return lastLineItem;
}

void TextLine::paint( QPainter &p, int y )
{
    QPtrListIterator<Item> it( m_items );
    int x = 0;
    for (; it.current(); ++it )
    {
        p.translate( x, y );
        it.current()->paint( p );
        p.translate( -x, -y );
        x += it.current()->width();
    }
}

Item *TextLine::itemAt( int px, SelectionPoint *selectionInfo,
                        Item::SelectionAccuracy accuracy )
{
    QPtrListIterator<Item> it( m_items );
    int x = 0;
    int width = 0;
    for (; it.current(); ++it )
    {
        width = it.current()->width();
        if ( x < px && px < ( x + width ) )
        {
            Item *i = it.current();
            if ( selectionInfo )
            {
                selectionInfo->pos.setX( x );
                selectionInfo->offset = i->calcSelectionOffset( px - x );
                selectionInfo->item = i;
                selectionInfo->line = this;
            }
            return i;
        }
        x += width;
    }

    if ( accuracy == Item::SelectFuzzy && selectionInfo &&
         !m_items.isEmpty() && width > 0 )
    {
        Item *i = m_items.getLast();
        selectionInfo->pos.setX( x - width );
        selectionInfo->offset = i->maxSelectionOffset();
        selectionInfo->item = i;
        selectionInfo->line = this;
    }

    return 0;
}

QString TextLine::plainText() const
{
    QString res;
    QPtrListIterator<Item> it( m_items );
    for (; it.current(); ++it )
        res += it.current()->text().toQString();
    return res;
}

void TextLine::fontChange( const QFont &newFont )
{
    QPtrListIterator<Item> it( m_items );
    for (; it.current(); ++it )
    {
        ItemProperties props = it.current()->props();
        props.updateFont( newFont );
        it.current()->setProps( props );
    }
}

TextParag::TextParag( TextView *textView, const QString &richText )
    : m_layouted( false ), m_height( 0 ), m_minWidth( 0 ),
      m_textView( textView )
{
    setRichText( richText );
    m_lines.setAutoDelete(true);
}

TextParag::~TextParag()
{
}

void TextParag::layout( int width )
{
    QPtrList<Item> items;

    TextLine *row = m_lines.first();

    for(;row;row = m_lines.next()){
        row->resetLayout( items );
    }

    m_lines.clear();

    // all text chunks are now in a flat list. break them into
    // pieces of lists of chunks, so they fit with the given width

    m_height = 0;
    m_minWidth = 0;

    int remainingWidth = width;

    SelectionPoint *selStart = m_textView->selectionStart();
    SelectionPoint *selEnd = m_textView->selectionEnd();
    assert( selStart && selEnd );

    QPtrListIterator<Item> it( items );
    while ( it.current() )
    {
        m_minWidth = kMax( m_minWidth, it.current()->minWidth() );

        Item *item = it.current();
        int itemWidth = item->width();

        if ( remainingWidth >= itemWidth )
        {
            remainingWidth -= itemWidth;
            ++it;
            continue;
        }

        Item *newChunk = 0;

        if ( itemWidth > item->minWidth() )
            newChunk = item->breakLine( remainingWidth );

        if ( newChunk || it.atFirst() )
            ++it;

        TextLine *line = new TextLine;

        Item *next = it.current();

        items.first();

        while ( items.current() != next )
        {
            Item *i = items.take();

            if ( selStart->item == i )
                selStart->line = line;
            else if ( selEnd->item == i )
                selEnd->line = line;

            line->appendItem( i, TextLine::UpdateMaxHeight );
        }

        assert( !line->isEmpty() );

        m_height += line->maxHeight();
        m_lines.append( line );

        if ( newChunk )
            items.prepend( newChunk );

        it.toFirst();
        remainingWidth = width;
    }

    // append what's left
    if ( items.count() > 0 )
    {
        TextLine *line = new TextLine( items );
        m_height += line->maxHeight();
        m_lines.append( line );

        if ( selStart->parag == this ||
             selEnd->parag == this )
        {
            // ### move to TextLine?
            QPtrListIterator<Item> it( line->iterator() );
            for (; it.current(); ++it )
            {
                if ( selStart->item == it.current() )
                    selStart->line = line;
                if ( selEnd->item == it.current() )
                    selEnd->line = line;
            }
        }
    }

    m_layouted = true;
}

void TextParag::paint( QPainter &p, int y, int maxY )
{
    TextLine *row = m_lines.first();
    for (; row; row = m_lines.next() )
    {
        if( (y + row->maxHeight()) >= 0 )
	    row->paint( p, y );
	y += row->maxHeight();
	if( y > maxY )
            break;
    }
}

Item *TextParag::itemAt( int px, int py, SelectionPoint *selectionInfo,
                         Item::SelectionAccuracy accuracy )
{
    int y = 0;
    int height = 0;
    TextLine *row = m_lines.first();
    for (; row; row = m_lines.next() )
    {
        height = row->maxHeight();
        if ( y <= py && py <= ( y + height ) )
        {
            Item *i = row->itemAt( px, selectionInfo, accuracy );
            if ( selectionInfo )
            {
                selectionInfo->pos.setY( y );
                selectionInfo->parag = this;
            }
            return i;
        }
        y += height;
    }

    if ( accuracy == Item::SelectFuzzy && selectionInfo && !m_lines.isEmpty() )
    {
        TextLine *row = m_lines.getLast();
        row->itemAt( px, selectionInfo, accuracy );

        selectionInfo->pos.setY( y - height );
        selectionInfo->parag = this;
    }

    return 0;
}

QString TextParag::updateSelection( const SelectionPoint &start, const SelectionPoint &end )
{
    QString selectedText;

    // sanity check
    // (don't put it lower because it changes the current list item)
    if ( end.parag == this )
        assert( m_lines.findRef( end.line ) != -1 );

    if ( start.parag == this )
    {
        int idx = m_lines.findRef( start.line );
        assert( idx != -1 );
    }
    else
        m_lines.first();

    TextLine *line = m_lines.current();

    TextLine *lastLine = m_lines.getLast();

    if ( end.parag == this )
        lastLine = end.line;

    for (; line != lastLine; line = m_lines.next() )
        selectedText += line->updateSelection( start, end );

    if ( lastLine )
        selectedText += lastLine->updateSelection( start, end );

    return selectedText;
}

void TextParag::clearSelection()
{
    // ### optimize, add 'selectionDirty' flag to TextLine and TextParag!
    TextLine *line = m_lines.first();
    for (; line; line = m_lines.next() )
        line->clearSelection();
}

void TextParag::setRichText( const QString &richText )
{
    m_layouted = false;
    m_height = 0;
    m_minWidth = 0;

    // ### FIXME SELECTION!!!!!!!!!
    if ( m_textView->selectionStart()->parag == this ||
         m_textView->selectionEnd()->parag == this )
        m_textView->clearSelection();

    m_lines.clear();

    m_processedRichText = Tokenizer::preprocess( richText );

    Tokenizer tokenizer( m_processedRichText );
    Token tok;
    Token lastTextToken;

    QValueStack<Tag> tagStack;

    TextLine *line = new TextLine;

    while ( tokenizer.parseNextToken( tok ) )
    {
        if ( tok.id == Token::TagOpen )
        {
            ItemProperties oldProps( m_textView->font() );
            if ( !tagStack.isEmpty() )
                oldProps = tagStack.top().props;

            // ...bleh<foo>... -> finish off 'bleh' first
            if ( lastTextToken.id != -1 )
            {
                Item *item = Item::create( this, lastTextToken, oldProps );
                if ( item )
                    line->appendItem( item );
                lastTextToken = Token();
            }

            ItemProperties props( oldProps, tok, m_textView );
            tagStack.push( Tag( tok.value, props ) );

            Item *item = Item::create( this, tok, props );
            if ( item )
                line->appendItem( item );

            continue;
        }
        else if ( tok.id == Token::TagClose )
	{
            assert( !tagStack.isEmpty() );

            Tag tag = tagStack.pop();

            if( !( tok.value == tag.name ) ) {
                kdDebug(5008) << "ASSERT failed! tok.value=" << tok.value.toQString() << " tag.name=" << tag.name.toQString() << endl;
                kdDebug(5008) << "while parsing " << richText << endl;
            }

            // ...foo</bleh>... -> finish off 'foo'
            if ( !lastTextToken.value.isNull() )
            {
                Item *item = Item::create( this, lastTextToken, tag.props );
                if ( item )
                    line->appendItem( item );
            }

            lastTextToken = Token();
        }
        else
            lastTextToken = tok;
    }

    // some plain text at the very end, outside of any tag?
    if ( !lastTextToken.value.isNull() )
    {
        Item *item = Item::create( this, lastTextToken );
        if ( item )
            line->appendItem( item );
    }

    m_lines.append( line );
}

QString TextParag::plainText() const
{
    QString result;
    QPtrListIterator<TextLine> it( m_lines );
    for (; it.current(); ++it )
        result += it.current()->plainText();
    return result;
}

void TextParag::fontChange( const QFont &newFont )
{
    QPtrListIterator<TextLine> it( m_lines );
    for (; it.current(); ++it )
        it.current()->fontChange( newFont );
}

ContentsPaintAlgorithm::ContentsPaintAlgorithm( const QPtrListIterator<TextParag> &paragIt,
                                                QWidget *viewport, QPixmap &paintBuffer,
                                                QPainter &painter, int clipX, int clipY,
                                                int clipHeight )
        : m_paragIt( paragIt ), m_viewport( viewport ), m_paintBuffer( paintBuffer ),
          m_painter( painter ), m_clipX( clipX ), m_clipY( clipY ), m_clipHeight( clipHeight ),
          m_overshoot( 0 )
{
}

int ContentsPaintAlgorithm::goToFirstVisibleParagraph()
{
    int y = 0;

    while ( y < m_clipY && m_paragIt.current() ) {
        y += m_paragIt.current()->height();
        ++m_paragIt;
    }

    y = adjustYAndIterator( y, y, m_clipY );
    return y;
}

int ContentsPaintAlgorithm::paint( QPainter &bufferedPainter, int currentY )
{
    const int startY = currentY;

    int nextY = startY + PaintBufferExtend;

    if ( !m_paragIt.current() )
        return nextY;

    while ( currentY < nextY && m_paragIt.current() ) {

	TextParag *parag = m_paragIt.current();

	//kdDebug(5008) << "Painting[" << currentY << "/" << parag->height() << "]: " << parag->plainText() << endl;

	int drawPos = currentY;
        int newY = parag->height();

	if(m_overshoot != 0) {
            drawPos = currentY - parag->height() + m_overshoot;
	    newY = m_overshoot;
	    m_overshoot = 0;
	}

	parag->paint( bufferedPainter, drawPos, nextY );
	currentY += newY;

        ++m_paragIt;
    }

    nextY = adjustYAndIterator( startY, currentY, nextY );
    return nextY;
}

int ContentsPaintAlgorithm::adjustYAndIterator( int , int currentY, int nextY )
{
    // nothing to adjust?
    if ( currentY <= nextY || m_paragIt.atFirst() )
        return currentY;

    if ( m_paragIt.current() )
	--m_paragIt;
    else
	m_paragIt.toLast();

    m_overshoot = currentY - nextY;
    if(m_overshoot < 0)
	m_overshoot = 0;

    return nextY;
}

void ContentsPaintAlgorithm::paint()
{
    int y = goToFirstVisibleParagraph();

    int yEnd = m_clipY + m_clipHeight;

    while ( y < yEnd )
    {
        m_paintBuffer.fill( m_viewport, 0, y );
        QPainter bufferedPainter( &m_paintBuffer );
        bufferedPainter.translate( -m_clipX, -y );
	int nextY = paint( bufferedPainter, y );


        bufferedPainter.end();

        m_painter.drawPixmap( m_clipX, y, m_paintBuffer );

	y = nextY;
    }
}

TextView::TextView( QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase ),
      m_paintBuffer( PaintBufferExtend, PaintBufferExtend ),
      m_selectionEndBeforeStart( false ), m_mousePressed( false ),
      m_mmbPressed( false ), 
      m_linkColor( Qt::blue ), m_height(-1), m_inScroll(false),
      m_lastScroll(0)
{
    m_parags.setAutoDelete( true );
    viewport()->setBackgroundMode( PaletteBase );
    viewport()->setMouseTracking( true );
    m_autoScrollTimer = new QTimer( this );
    connect(verticalScrollBar(), SIGNAL(valueChanged( int ) ),
            this, SLOT(scrolling( int )));

    setDragAutoScroll( false );

//    setStaticBackground( true );
}

TextView::~TextView()
{
}

void TextView::drawContents( QPainter *painter, int clipX, int clipY, int , int clipHeight )
{
    if ( m_parags.isEmpty() )
        return;

    if ( m_paintBuffer.width() != visibleWidth() )
        m_paintBuffer.resize( visibleWidth(), PaintBufferExtend );

    QPtrListIterator<TextParag> paragIt( m_parags );

    ContentsPaintAlgorithm( paragIt,
                            viewport(), m_paintBuffer, *painter, clipX, clipY,
                            clipHeight )
        .paint();
}

void TextView::viewportResizeEvent( QResizeEvent *ev )
{
    QScrollView::viewportResizeEvent(ev);

    if ( ev->size().width() != ev->oldSize().width() )
	layout();

    int newdiff = ev->size().height() - ev->oldSize().height();
    setContentsPos( 0, contentsY()-newdiff );

    if(m_lastScroll == newdiff){
	m_inScroll = false;
        m_lastScroll = 0;
    }

    scrollToBottom();
}

void TextView::scrolling( int value )
{
    int tl = m_height-visibleHeight();
    int offset = 25;

    TextParag *parag =  m_parags.last();
    if(parag){
	if(parag->height() > offset)
            offset = parag->height();
    }

    if((tl - value) > offset)
	m_inScroll = true;
    else
	m_inScroll = false;

    m_lastScroll = tl-value;

}

void TextView::scrollToBottom( bool force )
{
    bool scroll = true;
    if(force == true){
	scroll = true;
    }
    else {
	if(m_inScroll){
            scroll = false;
	}
	else {
	    if(m_mousePressed == true){
                scroll = false;
	    }
	    else {
		scroll = true;
	    }
	}
    }

    if(scroll == true)
	setContentsPos( 0, m_height-visibleHeight() );
}

void TextView::clear()
{
    stopAutoScroll();
    clearSelection();
    m_parags.clear();
    layout();
    viewport()->erase();
}

TextParagIterator TextView::appendParag( const QString &richText )
{
    TextParag *parag = new TextParag( this, richText );
    m_parags.append( parag );
    layout( false );
    scrollToBottom();
    QPtrListIterator<TextParag> it( m_parags );
    it.toLast();
    return TextParagIterator( it );
}

bool TextView::removeParag( const TextParagIterator &parag )
{

    if ( parag.atEnd() )
        return false;

    TextParag *paragPtr = parag.m_paragIt.current();
    const int idx = m_parags.findRef( paragPtr );
    if ( idx == -1 )
        return false;

    if ( m_selectionStart.parag == paragPtr ||
         m_selectionEnd.parag == paragPtr )
        clearSelection( false );

    int height = paragPtr->height();
    m_parags.removeRef( paragPtr );

    if(m_selectionStart.item != 0)
        m_selectionStart.pos.ry() -= height;
    if(m_selectionEnd.item != 0)
        m_selectionEnd.pos.ry() -= height;

    //layout( false );
    contentsChange(-height, true);


    if ( isUpdatesEnabled() )
        updateContents();

    return true;
}

void TextView::clearSelection( bool repaint )
{
    m_selectionStart = SelectionPoint();
    m_selectionEnd = SelectionPoint();
    m_selectionEndBeforeStart = false;
    m_selectedText = QString::null;
    clearSelectionInternal();
    if ( repaint )
        updateContents();
}

TextParagIterator TextView::firstParag() const
{
    return TextParagIterator( QPtrListIterator<TextParag>( m_parags ) );
}

QString TextView::plainText() const
{
    if ( m_parags.isEmpty() )
        return QString::null;

    QString result;
    QPtrListIterator<TextParag> paragIt( m_parags );
    while ( paragIt.current() )
    {
        result += paragIt.current()->plainText();
        ++paragIt;
        if ( paragIt.current() )
            result += '\n';
    }

    return result;
}

QColor TextView::linkColor() const
{
    return m_linkColor;
}

void TextView::setLinkColor( const QColor &linkColor )
{
    m_linkColor = linkColor;
}

void TextView::copy()
{
    QApplication::clipboard()->setText( m_selectedText );
}

void TextView::clearSelectionInternal()
{
    m_selectionEndBeforeStart = false;
    TextParag *p = m_parags.first();
    for (; p; p = m_parags.next() )
        p->clearSelection();
}

void TextView::contentsMousePressEvent( QMouseEvent *ev )
{
    if ( ev->button() & RightButton ) {
        emitLinkClickedForMouseEvent( ev );
        return;
    }

    if ( !( ev->button() & LeftButton ) && !(ev->button() & MidButton ) )
        return;

    clearSelection( true );
    SelectionPoint p;
    Item *itemUnderMouse = itemAt( ev->pos(), &p, Item::SelectFuzzy );
    if ( p.item && ( ev->button() & LeftButton ) ) {
        m_selectionMaybeStart = p;
        p.item->setSelectionStatus( Item::NoSelection );
    }
    if ( TextChunk *text = dynamic_cast<TextChunk *>( itemUnderMouse ) ) {
        StringPtr href = text->props().attributes[ "href" ];
        if ( !href.isNull() ) {
            m_dragStartPos = ev->pos();
            m_dragURL = href.toQString();
            if ( ev->button() & LeftButton )
               m_mousePressed = true;
            else 
               m_mmbPressed = true;
        }
    }
}

void TextView::contentsMouseMoveEvent( QMouseEvent *ev )
{
    if ( m_mousePressed && ev->state() == NoButton )
    {
        m_mousePressed = false;
        m_mmbPressed = false;
    }

    if ( m_mousePressed && !m_dragURL.isEmpty() &&
         ( m_dragStartPos - ev->pos() ).manhattanLength() > QApplication::startDragDistance() ) {

        m_mousePressed = false;
        m_dragStartPos = QPoint();

        startDrag();

        m_dragURL = QString::null;
        return;
    }


    SelectionPoint p;
    Item *i = itemAt( ev->pos(), &p, Item::SelectFuzzy );
    if ( !i && !p.item )
        return;

    if ( (ev->state() & LeftButton && m_selectionStart.item && p.item) ||
         (ev->state() & LeftButton && m_selectionMaybeStart.item && p.item))
    {

        if(m_selectionMaybeStart.item != 0){
            m_selectionStart = m_selectionMaybeStart;
            m_selectionMaybeStart = SelectionPoint();
        }

        m_selectionEnd = p;

        clearSelectionInternal();

        updateSelectionOrder();

        SelectionPoint start = m_selectionStart;
        SelectionPoint end = m_selectionEnd;

        if ( m_selectionEndBeforeStart )
        {
            if ( start.item == end.item )
            {
                if ( start.offset > end.offset )
                    qSwap( start.offset, end.offset );
            }
            else
                qSwap( start, end );
        }

        m_selectedText = updateSelection( start, end );

        emit selectionChanged();

        updateContents();

        startAutoScroll();

        return;
    }
    else if ( i )
    {
        TextChunk *text = dynamic_cast<TextChunk *>( i );
        if ( text )
        {
            StringPtr href = text->props().attributes[ "href" ];
            if ( !href.isNull() )
            {
                viewport()->setCursor( KCursor::handCursor() );
                return;
            }
        }
    }

    QCursor c = KCursor::arrowCursor();
    if ( viewport()->cursor().handle() != c.handle() )
        viewport()->setCursor( c );
}

void TextView::contentsMouseReleaseEvent( QMouseEvent *ev )
{
    stopAutoScroll();

    bool clicked = (m_mousePressed || m_mmbPressed) &&
                   (m_dragStartPos - ev->pos()).manhattanLength() < QApplication::startDragDistance();
    m_mousePressed = false;
    m_mmbPressed = false;
    m_dragStartPos = QPoint();
    m_dragURL = QString::null;

    m_selectionMaybeStart = SelectionPoint();

    if ( (ev->button() & Qt::LeftButton) && !m_selectedText.isEmpty() )
        QApplication::clipboard()->setText( m_selectedText, QClipboard::Selection );

    if ( clicked ) {
        emitLinkClickedForMouseEvent( ev );
        return;
    }

    if (ev->button() & Qt::MidButton)
    {
        emit pasteReq( KApplication::clipboard()->text( QClipboard::Selection ) );
        return;
    }
}

void TextView::fontChange( const QFont & )
{
    QPtrListIterator<TextParag> it( m_parags );
    for (; it.current(); ++it )
        it.current()->fontChange( font() );

    layout( true );
}

void TextView::startDrag()
{
    QDragObject *dragObj = dragObject( m_dragURL );
    if ( !dragObj )
        return;

    stopAutoScroll();

    dragObj->drag();
}

QDragObject *TextView::dragObject( const QString &dragURL )
{
#if KDE_IS_VERSION(3,1,92)
    return new KURLDrag( KURL( dragURL ), viewport() );
#else
    return KURLDrag::newDrag( KURL( dragURL ), viewport() );
#endif
}

void TextView::autoScroll()
{
    QPoint cursor = viewport()->mapFromGlobal( QCursor::pos() );

    QPoint contentsPos = viewportToContents( cursor );

    cursor.rx() -= viewport()->x();
    cursor.ry() -= viewport()->y();

    if ( ( cursor.x() < 0 || cursor.x() > visibleWidth() ) ||
	 ( cursor.y() < 0 || cursor.y() > visibleHeight() ) ) {
        ensureVisible( contentsPos.x(), contentsPos.y(),
		       0, 5 );
    }
}

void TextView::emitLinkClickedForMouseEvent( QMouseEvent *ev )
{
    TextChunk *text = dynamic_cast<TextChunk *>( itemAt( ev->pos() ) );
    if ( !text )
        return;

    StringPtr href = text->props().attributes[ "href" ];
    if ( href.isNull() )
        return;

    emit linkClicked( ev, CONSTSTRING( href ) );
}

void TextView::startAutoScroll()
{
    if(m_autoScrollTimer->isActive() == false){
	connect( m_autoScrollTimer, SIGNAL( timeout() ),
		 this, SLOT( autoScroll() ) );
	m_autoScrollTimer->start( 75, false );
    }
}

void TextView::stopAutoScroll()
{
    disconnect( m_autoScrollTimer, SIGNAL( timeout() ),
                this, SLOT( autoScroll() ) );
    m_autoScrollTimer->stop();
}

void TextView::selectionOffsets( int &startOffset, int &endOffset )
{
    assert( m_selectionStart.item );

    if ( m_selectionEndBeforeStart )
    {
        startOffset = m_selectionEnd.offset;
        endOffset = m_selectionStart.offset;
    }
    else
    {
        startOffset = m_selectionStart.offset;
        endOffset = m_selectionEnd.offset;
    }

    if ( m_selectionStart.item == m_selectionEnd.item && startOffset > endOffset )
        qSwap( startOffset, endOffset );
}

void TextView::updateSelectionOrder()
{
    int start = m_selectionStart.pos.y();
    int end = m_selectionEnd.pos.y();

    if ( start == end )
    {
        start = m_selectionStart.pos.x();
        end = m_selectionEnd.pos.x();

        if ( start == end )
        {
            start = m_selectionStart.offset;
            end = m_selectionEnd.offset;
        }
    }

    m_selectionEndBeforeStart = end < start;
}

SelectionPoint *TextView::selectionStart()
{
    return m_selectionEndBeforeStart ? &m_selectionEnd : &m_selectionStart;
}

SelectionPoint *TextView::selectionEnd()
{
    return m_selectionEndBeforeStart ? &m_selectionStart : &m_selectionEnd;
}

QString TextView::updateSelection( const SelectionPoint &start, const SelectionPoint &end )
{
    QString selectedText;

    if ( start.item == end.item )
    {
        Item *i = start.item;


        if ( start.offset == end.offset )
        {
            if ( start.pos.x() == end.pos.x() )
            {
                i->setSelectionStatus( Item::NoSelection );
                return QString::null;
            }

            i->setSelectionStatus( Item::SelectionBoth );

            // ### ugly
            const TextChunk *t = dynamic_cast<TextChunk *>( i );
            if ( t )
            {
                StringPtr text = t->text();
                selectedText = QString( text.ptr + start.offset, 1 );
            }
        }
        else
        {
            i->setSelectionStatus( Item::SelectionBoth );

            // ### ugly
            TextChunk *t = dynamic_cast<TextChunk *>( i );
            if ( t )
            {
                StringPtr text = t->text();
                if (end.offset > start.offset)
                    selectedText = QString( text.ptr + start.offset,
                                            end.offset - start.offset + 1 );
                else
                    selectedText = QString( text.ptr + end.offset,
                                            start.offset - end.offset + 1 );

            }
        }
    }
    else
    {
        assert( m_parags.findRef( end.parag ) != -1 );
        const int idx = m_parags.findRef( start.parag );
        assert( idx != -1 );

        TextParag *p = m_parags.current();
        for (; p && p != end.parag; p = m_parags.next() )
        {
            selectedText += p->updateSelection( start, end );
            selectedText += '\n';
        }

        if ( p )
            selectedText += p->updateSelection( start, end );
    }

    return selectedText;
}

void TextView::layout( bool force )
{
    int height = 0;
    int contentsWidth = visibleWidth();
    int width = contentsWidth;

    QPtrListIterator<TextParag> it( m_parags );
    for (; it.current(); ++it )
    {
        if ( !it.current()->isLayouted() || force )
            it.current()->layout( width );

        height += it.current()->height();
        contentsWidth = kMax( contentsWidth, it.current()->minWidth() );
    }

    if ( m_selectionStart.item && m_selectionEnd.item )
        updateSelection( *selectionStart(), *selectionEnd() );

    m_height = height;
    resizeContents( contentsWidth, height );
}

void TextView::contentsChange(int heightChange, bool force)
{
    if(m_height == -1){
        layout(force);
    }
    else {
        m_height += heightChange;
        resizeContents( visibleWidth(), m_height );
    }

    if ( m_selectionStart.item && m_selectionEnd.item )
        updateSelection( *selectionStart(), *selectionEnd() );


}

Item *TextView::itemAt( const QPoint &pos, SelectionPoint *selectionInfo,
                          Item::SelectionAccuracy accuracy )
{
    int px = pos.x();
    int py = pos.y();

    int y = 0;
    int height = 0;
    QPtrListIterator<TextParag> it( m_parags );
    for (; it.current(); ++it )
    {
        height = it.current()->height();
        if ( y <= py && py <= ( y + height ) )
        {
            Item *res = it.current()->itemAt( px, py - y, selectionInfo, accuracy );
            if ( selectionInfo )
            {
                selectionInfo->pos.ry() += y;
                selectionInfo->pos.rx() = px;
            }
            return res;
        }
        y += height;
    }

    if ( accuracy == Item::SelectFuzzy && selectionInfo && !m_parags.isEmpty() )
    {
        TextParag *parag = m_parags.getLast();
        parag->itemAt( px, height - 1, selectionInfo, accuracy );

        selectionInfo->pos.ry() += y - height;
        selectionInfo->pos.rx() = px;
    }

    return 0;
}

QString TextParagIterator::richText() const
{
    if ( atEnd() )
        return QString::null;

    return Tokenizer::convertToRichText( m_paragIt.current()->processedRichText() );
}

void TextParagIterator::setRichText( const QString &richText )
{
    if ( atEnd() )
        return;

    m_paragIt.current()->setRichText( richText );

    TextView *textView = m_paragIt.current()->textView();
    textView->layout( false );

    if ( textView->isUpdatesEnabled() )
        textView->updateContents();
}

QString TextParagIterator::plainText() const
{
    if ( atEnd() )
        return QString::null;

    return m_paragIt.current()->plainText();
}

#include "kstextview.moc"

/*
 * vim: et sw=4
 */
