#include <stdlib.h>
#include <qpainter.h>
#include <qframe.h>
#include <qpaintdevice.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kfontdialog.h>
#include <klocale.h>

#include "ksticker.h"
#include "speeddialog.h"

#include "kspainter.h"
#include "../ksopts.h"
#include "../nickColourMaker.h"

#include <kconfig.h>
#include <kwin.h>
#include <kdebug.h>

KSTicker::KSTicker(QWidget * parent, const char * name, WFlags f)
: QFrame(parent, name, f)
{

  pHeight = 1;

  pic = new QPixmap(); // create pic map here, resize it later though.
  //  pic->setBackgroundMode(TransparentMode);

  KConfig *conf = kapp->config();
  conf->setGroup("KSTicker");
  bScrollConstantly = conf->readNumEntry("ScollConst", FALSE);

  bAtEnd = FALSE;
  setFont(conf->readFontEntry("Font", &ksopts->defaultFont));
  ourFont = font(); //QFont("Courier");
  setFont(ourFont);
  setMinimumSize(100, 10);
  setFixedHeight((fontMetrics().height()+fontMetrics().descent()*2)*pHeight);
  descent =  fontMetrics().descent();
  onechar = fontMetrics().width("X");

  pic->resize(width() + onechar, height());
  pic->fill(backgroundColor());

  tickStep = 2;
  cOffset = 0;

  tickRate = 30;

  currentChar = 0;
  chars = this->width() / onechar;

  popup = new QPopupMenu();
  popup->insertItem(i18n( "Font..." ), this, SLOT(fontSelector()));
  popup->insertItem(i18n( "Scroll Rate..." ), this, SLOT(scrollRate()));
  iScrollItem = popup->insertItem(i18n( "Scroll Constantly" ), this, SLOT(scrollConstantly()));
  popup->setItemChecked(iScrollItem, bScrollConstantly);
  popup->insertSeparator();
  popup->insertItem(i18n( "Return to Normal Mode" ), this, SIGNAL(doubleClick()));

  currentStr = "";

  /*
   * Tell KWM to use only minimum decurations
   */
  // ### FIXME: port to kwin
  //KWM::setDecoration(winId(), KWM::tinyDecoration);
  //  KWin::setType(winId(), NET::Override);

  /*
   * Setup basic colours and background status info for ticker.
   */

  KSPainter::initOptColours();

  bold = FALSE;
  underline = FALSE;
  italics = FALSE;
  defbg = ksopts->backgroundColor;
  deffg = ksopts->textColor;
  setBackgroundColor(defbg);
  bg = ksopts->backgroundColor;
  fg = ksopts->textColor;

}

KSTicker::~KSTicker()
{
  killTimers();
  delete pic;
}

void KSTicker::show()
{
  /*
   * Tell KWM to use only minimum decurations
   */
    // ### FIXME: port to kwin
    //KWM::setDecoration(winId(), KWM::tinyDecoration);
  QSize size = this->size();
  size.setHeight(QFontMetrics(font()).height()+10);
  setFixedHeight(QFontMetrics(font()).height()+10);
  resize(size);

  QFrame::show();
  if(currentStr.length() != 0)
    startTicker();
  currentChar = 0;
  repaint(TRUE);
}

void KSTicker::hide()
{
  killTimers();
  QFrame::hide();
}

void KSTicker::iconify()
{
  QFrame::showMinimized();
  killTimers();
}

void KSTicker::setString(QString str)
{
    strlist.clear();
    strlist.append(str);
    repaint(TRUE);
    startTicker();
}

void KSTicker::mergeString(QString str, QColor c)
{
  int num = KSPainter::colour2num(c);

  if(num != -1){
      str.prepend(QString("~%1").arg(num));
  }
  mergeString(str);

}

void KSTicker::mergeString(QString str)
{
    QRegExp rx("~n(\\S+)~n");
    if(rx.search(str) >= 0){
	int value = nickColourMaker::colourMaker()->findIdx(rx.cap(1));
	if(value >= 0){
	    QString newText = QString("~%1\\1~c").arg(value);
	    str.replace(rx, newText);
	}
    }

    str.append("~C ");
    strlist.append(str);

    if(strlist.count() > 5){
        int found = 0;
        QStringList::Iterator it = strlist.begin();
        for(; it != strlist.end(); it++){
            if(((*it).find(ksopts->server["global"].nick, 0, FALSE) == -1) &&
               ((*it).find(ksopts->server["global"].altNick, 0, FALSE) == -1)){
                strlist.remove(it);
                found = 1;
                break;
            }
            else {
            }
        }
        if(found == 0){
            strlist.pop_front();
        }
    }

    if(bScrollConstantly == FALSE)
        startTicker();

    QStringList sep = QStringList::split(" ", stripCols(str));
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
    if(brok.endsWith("\n"))
        brok.truncate(brok.length()-1);

    tipbuffer.append(brok);
    while(tipbuffer.count() > 10)
        tipbuffer.pop_front();
    QString tip = tipbuffer.join("\n");

//    QToolTip::remove(this);
    QToolTip::add(this, tip);


}

void KSTicker::timerEvent(QTimerEvent *)
{
    if((uint)currentChar >= currentStr.length()){
        if(strlist.isEmpty()){
            if(bScrollConstantly == TRUE){
                currentStr = strbuffer.first();
                strbuffer.append(strbuffer.first());
                strbuffer.pop_front();
                currentChar = 0;
            }
            else{
                stopTicker();
                return;
            }
        }
        else {
            currentStr = strlist.first();
            strlist.pop_front();
            strbuffer.append(currentStr);
            while(strbuffer.count() > 10)
                strbuffer.pop_front();
            currentChar = 0;
        }
    }

    bAtEnd = FALSE;
    static BGMode bgmode = TransparentMode;

    bitBlt(pic, -tickStep, 0, pic);
    QPainter p(pic);

    cOffset += tickStep;
    if(cOffset >= onechar){
        int step = 1; // Used to check if we did anything, and hence
        // catch ~c~c type things. Set to 1 to start loop
        while(((currentStr[currentChar] == '~') || (currentStr[currentChar] == 0x03))
              && (step > 0)){
            step = 1; // reset in case it's our second, or more loop.
            QString text = currentStr.mid(currentChar);
            QString buf = "";

            if((text[step] >= '0') &&
               (text[step] <= '9')) {
                buf += text[step];
                step++;
                if((text[step] >= '0') &&
                   (text[step] <= '9')) {
                    buf += text[step];
                    step++;
                }
                int col = buf.toInt();
                if((col >= 0) || (col <= KSPainter::maxcolour)){
                    fg = KSPainter::num2colour[col];
                }
                bg = defbg;
                buf = "";
                if(text[step] == ','){
                    step++;
                    if((text[step] >= '0') &&
                       (text[step] <= '9')) {
                        buf += text[step];
                        step++;
                        if((text[step] >= '0') &&
                           (text[step] <= '9')) {
                            buf += text[step];
                            step++;
                        }
                        int col = buf.toInt();
                        if((col >= 0) || (col <= KSPainter::maxcolour)){
                            bg = KSPainter::num2colour[col];
                            bgmode = OpaqueMode;
                        }
                    }
                }
                else{
                    bgmode = TransparentMode;
                }
            }
            else {
                switch(text[step].latin1()){
                case 'c':
                    fg = deffg;
                    bg = defbg;
                    step++;
                    break;
                case 'C':
                    fg = deffg;
                    bg = defbg;
                    bold = FALSE;
                    underline = FALSE;
                    italics = FALSE;
                    step++;
		    break;
		case '#':
		    fg.setNamedColor(text.mid(step, 7));
		    step += 7;
                    break;
                case 'b':
                    bold == TRUE ? bold = FALSE : bold = TRUE;
                    step++;
                    break;
                case 'u':
                    underline == TRUE ? underline = FALSE : underline = TRUE;
                    step++;
                    break;
                case 'i':
                    italics == TRUE ? italics = FALSE : italics = TRUE;
                    step++;
                    break;
                case 'n':
                    fg = ksopts->nickForeground;
                    bg = ksopts->nickBackground;
                    step++;
                    break;
                case 'o':
                    fg = ksopts->msgContainNick;
                    step++;
                    break;
                case '~':
                    currentChar++; // Move ahead 1, but...
                    step = 0;      // Don't loop on next ~.
                    break;
                default:
                    if(currentStr[currentChar] == 0x03){
                        fg = deffg;
                        bg = defbg;
                    }
                    else
                        step = 0;
                }
            }
            currentChar += step;
        }
        if((uint)currentChar >= currentStr.length()){ // Bail out if we're
            // at the end of the string.
            return;
        }


        QFont fnt = font();
        fnt.setBold(bold);
        fnt.setUnderline(underline);
        fnt.setItalic(italics);
        p.setFont(fnt);

        p.setPen(fg);
        p.setBackgroundColor(bg);
        p.setBackgroundMode(OpaqueMode);
        p.drawText(this->width() - cOffset + onechar, // remove -onechar.
                   this->height() / 4 + p.fontMetrics().height() / 2,
                   currentStr.mid(currentChar, 1),
                   1);
        currentChar++; // Move ahead to next character.
        cOffset -= onechar; // Set offset back one.
    }
    p.end();
    bitBlt(this, 0, descent, pic);
}

void KSTicker::paintEvent( QPaintEvent *)
{
  if(isVisible() == FALSE)
    return;
  bitBlt(this, 0, descent, pic);
}

void KSTicker::resizeEvent( QResizeEvent *e)
{
  QFrame::resizeEvent(e);
  onechar = fontMetrics().width("X");
  chars = this->width() / onechar;
  killTimers();
  QPixmap *new_pic = new QPixmap(width() + onechar, height());
  new_pic->fill(backgroundColor());
  bitBlt(new_pic,
         new_pic->width() - pic->width(), 0,
         pic, 0, 0,
         pic->width(), pic->height(),
         CopyROP, TRUE);
  delete pic;
  pic = new_pic;
  //  if(ring.length() > (uint) chars)
    startTicker();
}

void KSTicker::closeEvent( QCloseEvent *)
{
  emit closing();
  killTimers();
  //  delete this;
}

void KSTicker::startTicker()
{
  killTimers();
  startTimer(tickRate);
}

void KSTicker::stopTicker()
{
  killTimers();
}

void KSTicker::mouseDoubleClickEvent( QMouseEvent * )
{
  emit doubleClick();
}

void KSTicker::mousePressEvent( QMouseEvent *e)
{
  if(e->button() == RightButton){
    popup->popup(this->cursor().pos());
  }
  else{
    QFrame::mousePressEvent(e);
  }
}
void KSTicker::fontSelector()
{
  int result = KFontDialog::getFont( ourFont, true );
  if ( result == KFontDialog::Accepted ) {
      updateFont(ourFont);
  }
}

void KSTicker::scrollRate()
{
  SpeedDialog *sd = new SpeedDialog(tickRate, tickStep);
  sd->setLimit(5, 200, 1, onechar);
  connect(sd, SIGNAL(stateChange(int, int)),
          this, SLOT(setSpeed(int, int)));
  sd->show();
}

void KSTicker::scrollConstantly()
{
  bScrollConstantly = !bScrollConstantly;
  popup->setItemChecked(iScrollItem, bScrollConstantly);
  if(bScrollConstantly == TRUE)
    startTicker();
  KConfig *conf = kapp->config();
  conf->setGroup("KSTicker");
  conf->writeEntry("ScollConst", bScrollConstantly);
  conf->sync();
}

void KSTicker::updateFont(const QFont &font){
  setFont(font);
  setFixedHeight((fontMetrics().height()+fontMetrics().descent()*2)*pHeight);
  resize(fontMetrics().width("X")*chars,
         (fontMetrics().height()+fontMetrics().descent())*pHeight);
  KConfig *conf = kapp->config();
  conf->setGroup("KSTicker");
  conf->writeEntry("Font", font);
  conf->sync();

}

void KSTicker::setSpeed(int _tickRate, int _tickStep){
  tickRate = _tickRate;
  tickStep = _tickStep;
  startTicker();
}

void KSTicker::speed(int *_tickRate, int *_tickStep){
  *_tickRate = tickRate;
  *_tickStep = tickStep;
}

void KSTicker::setBackgroundColor ( const QColor &c )
{
  QFrame::setBackgroundColor(c);
  pic->fill(c);
  bitBlt(this, 0,0, pic);
  defbg = backgroundColor();
  bg = backgroundColor();
}

void KSTicker::setPalette ( const QPalette & p )
{
  QFrame::setPalette(p);

  pic->fill(backgroundColor());
  bitBlt(this, 0,0, pic);
  defbg = backgroundColor();
  bg = backgroundColor();
  deffg = backgroundColor();
  fg = foregroundColor();
}

QString KSTicker::stripCols( QString str )
{
    uint i = 0;
    QString ret;
    for(i = 0; i < str.length(); i++){
        if(((str[i] == '~') || (str[i] == 0x03))){
            if((str[i+1] >= '0') &&
               (str[i+1] <= '9')) {
                i+=1;
                if((str[i+1] >= '0') &&
                   (str[i+1] <= '9')) {
                    i+=1;
                }
                if(str[i+1] == ','){
                    i+=1;
                    if((str[i+1] >= '0') &&
                       (str[i+1] <= '9')) {
                        i+=1;
                        if((str[i+1] >= '0') &&
                           (str[i+1] <= '9')) {
                            i+=1;
                        }
                    }
                }
            }
            else {
                switch(str[i+1].latin1()){
                case 'c':
                case 'C':
                case 'b':
                case 'u':
                case 'i':
                case 'n':
                case 'o':
                    i+= 1;
                    break;
                case '~':
                    i+= 0;
                    break;
                default:
                    ret.append(str[i]);
                }
            }
        }
        else {
            ret.append(str[i]);
        }
    }
    return ret;
}
#include "ksticker.moc"
