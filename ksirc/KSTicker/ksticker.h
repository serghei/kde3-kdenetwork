#ifndef KSTICKER_H
#define KSTICKER_H

#include <qobject.h>
#include <qframe.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qpopupmenu.h>

class SInfo {
public:
  int length;
};

class KSTicker : public QFrame
{
  Q_OBJECT

public:
  KSTicker(QWidget * parent=0, const char * name=0, WFlags f=0);
  virtual ~KSTicker();

  void setString(QString);
  void mergeString(QString);
  void mergeString(QString, QColor);

  virtual void show();
  virtual void hide();

  void speed(int *, int *);

  virtual void setBackgroundColor ( const QColor & );
  virtual void setPalette ( const QPalette & p );

signals:
  void doubleClick();
  void closing();

public slots:
  virtual void setSpeed(int, int);

protected slots:
  virtual void fontSelector();
  virtual void scrollRate();
  virtual void updateFont(const QFont &font);
  virtual void scrollConstantly();

protected:
  virtual void timerEvent ( QTimerEvent * );
  virtual void paintEvent ( QPaintEvent * );
  virtual void resizeEvent( QResizeEvent * );
  virtual void closeEvent( QCloseEvent * );
  virtual void mouseDoubleClickEvent( QMouseEvent * );
  virtual void mousePressEvent ( QMouseEvent * );
  virtual void iconify();


private:

    QString stripCols(QString);

    QStringList strlist;   /* everything left to parse */
    QStringList strbuffer; /* fifo of the last 10 lines to scroll in scroll constantly mode */
    QStringList tipbuffer; /* 5 lines for the tooltip */
    QString currentStr;   /* the string we are currently parsing */

    QFont ourFont;

    int onechar;
    int chars;
    int descent;

    int tickStep;
    int cOffset;

    int tickRate;

    int pHeight;

    int currentChar;

    void startTicker();
    void stopTicker();

    bool bScrollConstantly;
    int iScrollItem;
    bool bAtEnd;

    QPixmap *pic;

    QPopupMenu *popup;

    /*
     * Drawing settings and variables
     */
    bool bold;
    bool underline;
    bool italics;
    QColor defbg;
    QColor deffg;
    QColor bg;
    QColor fg;
};

#endif // KSTICKER_H

