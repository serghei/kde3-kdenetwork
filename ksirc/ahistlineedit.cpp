/************************************************************************

 AHistLineEdit

 $$Id: ahistlineedit.cpp 278329 2004-01-10 02:43:13Z asj $$

 An extended KLineEdit with history scroll back and focus controls.

 Function:

   Each time a line is saved, to a max of 256, when the user hits enter.
   The arrow keys are used to scroll through the history list, rolling
   around at the end.

   When focus is gained or lost it emits the appropriate signals. This
   is so the toplevel can track who has focus.

 signals:
   gotFocus: duh!

   lostFocus: no shit sherlock

 Implementation:

   protected:

     keyPressEvent: Filter key presses looking for up arrow, down
       arrow or enter.  UpArrow saves the current line at the end then
       scroll. No more processing.  DownArrow does the opposite.  Enter
       saves the line, but then passes on the event for normal
       processing.

     focusInEvent: emits needed signal
     focusOutEvent: ditto

   Variables:
     QStrList: current list of history items.
     current: what I think is the current list item.

*************************************************************************/


#include "ahistlineedit.h"
#include "colorpicker.h"
#include "ksopts.h"
#include <qtextedit.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpen.h>
#include <qpainter.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kshortcut.h>


aHistLineEdit::aHistLineEdit(QWidget *parent, const char *name)
  : QTextEdit(parent, name)
{

    m_drawrect = false;
    m_height = 0;
    current = hist.append(QString::null); // Set the current as blank
    setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);
    setVScrollBarMode( AlwaysOff );
    setHScrollBarMode( AlwaysOff );

//    connect( this, SIGNAL( returnPressed () ), this, SLOT( slotReturn() ) );
    connect( this, SIGNAL( textChanged () ), this, SLOT( slotMaybeResize() ) );

    setTabChangesFocus(true);
    setTextFormat(PlainText);

    slotMaybeResize(); // setup initial size.

    setLineWidth(2);

}

void aHistLineEdit::setCursorPosition ( int index )
{
    QTextEdit::setCursorPosition(0, index);
}

int aHistLineEdit::cursorPosition() const
{
    int par, index;
    QTextEdit::getCursorPosition(&par, &index);
    return index;
}

void aHistLineEdit::slotMaybeResize()
{
/*
    if(QTextEdit::text().contains("\n")){
        setText(text());
        setCursorPosition(text().length());
        }
*/

    if(text().length() > IRC_SAFE_MAX_LINE){
        if(m_drawrect == false){
            m_drawrect = true;
            repaint();
        }
    }
    else {
        if(m_drawrect == true){
            m_drawrect = false;
            repaint();
        }
    }

    QFontMetrics metrics( currentFont() );
    //    int h = metrics.height() * lines();
    int h = metrics.lineSpacing() * lines()+8;
    // only grow if we are less than 1/4 the size of the toplevel
    if(h > (topLevelWidget()->height() >> 2)) {
        if(this != topLevelWidget()) {
            h = topLevelWidget()->height() >> 2;
            setVScrollBarMode( Auto );
        }
    }
    else {
        setVScrollBarMode( AlwaysOff );
    }
    if(h != m_height){
        m_height = h;
        QSize s = size();
        s.setHeight(h);
        resize(s);
        setFixedHeight( h );
        QLayout *l = topLevelWidget()->layout();
        if(l){
            l->invalidate();
            l->activate();
        }
        emit resized();
    }
}

void aHistLineEdit::keyPressEvent( QKeyEvent *e )
{
    bool accept = true;
    bool ignore = false;
    bool callKeyPressEvent = false;

//    kdDebug(5008) << "Got key: " << e->text() << "/" << e->key() << "-" << e->state() << endl;

    if ( ! (e->key() == Key_Tab || e->key() == Key_Shift || e->state() == ShiftButton || e->key() == 0)) {
//        kdDebug(5008) << "Sending notTab() " << e->text() << endl;
        emit notTab();
    }

    // those keycodes correspond to the ones in ksirc.pl in sub hook_fixcolours
    if ( e->state() == ControlButton ) {
        QString s = text();
        int curPos = cursorPosition();
        switch ( e->key() ) {
        case Key_K:
            if ( ksopts->colorPicker )
                ColourPickerPopUp();
            else
            {
                s.insert(  cursorPosition(), 0x03 );
                setText(s);
                setCursorPosition(curPos + 1);
            }
            break;
        case Key_I:
            s.insert( cursorPosition(), "~i");
            setText(s);
            setCursorPosition(curPos + 2);
	    break;
	case Key_Return:
	case Key_Enter:
	    doEnterKey();
	    callKeyPressEvent = false;
            break;
        default:
            accept = false;
            ignore = false;
            callKeyPressEvent = true;
        }
    }
    else if(e->state() == 0){
        switch(e->key()){
        case Key_Return:
        case Key_Enter:
            doEnterKey();
            callKeyPressEvent = false;
            break;
        case Key_Up:
            if(ksopts->oneLineEntry){
                if ((*current) != text()) { // change in hist <-> text() then save text
                    *current = text();
                }
                if (current == hist.begin()) { // if at begin of list then wrap around
                    current = hist.fromLast();
                } else --current; // previous entry
                setText(*current);
                setCursorPosition((*current).length());
                break;
            }
        case Key_Down:
            if(ksopts->oneLineEntry){
                if ((*current) != text()) { // change in hist <-> text() then save text
                    *current = text();
                }
                if (current == hist.fromLast()) { // if at end of list then wrap around
                    current = hist.begin();
                } else ++current; // next entry
                setText(*current);
                setCursorPosition((*current).length());
                break;
            }
        default:
            accept = false;
            ignore = false;
            callKeyPressEvent = true;
        }
    }
    // QLineEdit falsely converts Alt+C to the character 'c', so to work around
    //  this, we just don't pass any Alt+? key on to the keyPressEvent() method.
    else if (e->state() == AltButton) {
        switch(e->key()){
        case Key_Return:
        case Key_Enter:
            doEnterKey();
            callKeyPressEvent = false;
            break;
        case Key_Up:
            if ((*current) != text()) { // change in hist <-> text() then save text
                *current = text();
            }
            if (current == hist.begin()) { // if at begin of list then wrap around
                current = hist.fromLast();
            } else --current; // previous entry
            setText(*current);
            setCursorPosition((*current).length());
            break;
        case Key_Down:
            if ((*current) != text()) { // change in hist <-> text() then save text
                *current = text();
            }
            if (current == hist.fromLast()) { // if at end of list then wrap around
                current = hist.begin();
            } else ++current; // next entry
            setText(*current);
            setCursorPosition((*current).length());
            break;
        default:
            accept = false;
            callKeyPressEvent = true;
        }
    }
    else if (e->state() == ShiftButton){
        switch(e->key()){
        case Key_Return:
        case Key_Enter:
            doEnterKey();
            callKeyPressEvent = false;
            break;
        default:
            accept = false;
            ignore = false;
            callKeyPressEvent = true;
        }
    }
    else {
        switch(e->key()){
        case Key_Return:
        case Key_Enter:
            doEnterKey();
            callKeyPressEvent = false;
            break;
        default:
            accept = false;
            ignore = false;
            callKeyPressEvent = true;

        }
    }

//    kdDebug(5008) << "Accept: " << accept << " Ignore: " << ignore << " callKPE: " << callKeyPressEvent <<  endl;
    if ( accept ) {
        e->accept();
    }
    else {
        if ( ignore ) {
            e->ignore();
        }
        else {
            if ( callKeyPressEvent ) {
                QTextEdit::keyPressEvent(e);
            }
        }
    }
}

bool aHistLineEdit::processKeyEvent( QKeyEvent *e )
{
    /*
     * Only put key sequences in here you
     * want us to ignore and to pass upto
     * parent widgets for handling
     */

    bool eat = false;

//    kdDebug(5008) << "Key: " << KShortcut( KKey( e ) ).toString() << " StdAccel: " << KStdAccel::paste().toString() << endl;

    if ( KStdAccel::paste().contains(KKey( e ))) {
	e->ignore();
	eat = true;
    }

    return eat;
}

void aHistLineEdit::ColourPickerPopUp()
{
  ColorPicker picker( this );
  if ( picker.exec() == QDialog::Accepted )
  {
      QString s = text();
      //      int curPos = cursorPosition();
      int curPos, p;
      getCursorPosition(&p, &curPos);
      QString colString = picker.colorString();
      colString.prepend( 0x03 );
      s.insert( curPos, colString );
      setText( s );
      setCursorPosition( curPos + colString.length() );
  }
}

void aHistLineEdit::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);
    emit gotFocus();
}

void aHistLineEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);
    emit lostFocus();
}
#if 0
void aHistLineEdit::mousePressEvent ( QMouseEvent *e )
{
    if(e->button() == MidButton){
        /*
         *  emit pasteText(QApplication::clipboard()->text(QClipboard::Selection));
         */
  }
  else{
    QTextEdit::mousePressEvent(e);
  }
}
#endif
bool aHistLineEdit::eventFilter( QObject *o, QEvent *e )
{
    if ( o == this && e->type() == QEvent::AccelOverride )
        if(processKeyEvent( static_cast<QKeyEvent*>( e ) ) == true)
            return true;

    return QTextEdit::eventFilter( o, e );
}

QString aHistLineEdit::text() const
{
    QString s = QTextEdit::text();
    s.remove("\n");
    return s;
}

// Called upon MMB on the lineedit
void aHistLineEdit::paste()
{
    /* we let the top level take it */
    if(ksopts->oneLineEntry) {
	emit pasteText(QApplication::clipboard()->text(QClipboard::Selection));
    }
    else {
	QString paste = QApplication::clipboard()->text(QClipboard::Selection);
	paste.replace("\n", " ~ ");
	insert(paste);
    }
}

void aHistLineEdit::paintEvent ( QPaintEvent *p )
{
    QTextEdit::paintEvent(p);

    if(m_drawrect == true){
        QPainter paint(this);
        QPen pen = paint.pen();
        pen.setWidth(5);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(palette().active().highlight());
        paint.setPen(pen);
        QRect r = frameRect();
        paint.drawRect(r);
    }

}

void aHistLineEdit::doEnterKey()
{
    // strategy: always append an empty line to the end
    if ((*current).isEmpty()) { // current is empty
        if (!text().isEmpty()) {
            // text() has something -> store it in current and add a new empty one
            *current = text();
            hist.append(QString::null); // always add empty line at end
            if (hist.count() >= 256) { // if appended check if it will go beyond the max number of entries
                hist.remove(hist.begin()); // if so then delete the first entry
            }
        }
    } else { // current has content
        if (!text().isEmpty()) {
            // both !isEmpty() and != -> append at end
            current = hist.fromLast();     // goto last entry which is empty
            *current = text();             // change content to text()
            hist.append(QString::null);    // always add empty line at end
            if (hist.count() >= 256) {     // if appended check if it will go beyond the max number of entries
                hist.remove(hist.begin()); // if so then delete the first entry
            }
        }
    }
    current = hist.fromLast(); // set current history item back to the last item in the list
    emit gotReturnPressed();
}

#include "ahistlineedit.moc"
