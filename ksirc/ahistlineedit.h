#ifndef AHISTLINEEDIT_H
#define AHISTLINEEDIT_H

#include <qtextedit.h>
#include <qstringlist.h>
#include <qsize.h>

class aHistLineEdit : public QTextEdit
{
Q_OBJECT
public:
    aHistLineEdit(QWidget *parent = 0, const char *name = 0);

    int cursorPosition () const;
    void setCursorPosition(int);

    QString text() const;

signals:
    // Don't use returnPressed from the textEdit
    void gotReturnPressed();
    void gotFocus();
    void lostFocus();
    void pasteText(const QString&);
    void notTab();
    void resized();

public slots:
    virtual void paste();
    virtual void slotMaybeResize();

protected:
    bool processKeyEvent( QKeyEvent * );
    virtual void keyPressEvent ( QKeyEvent * );
    virtual void focusInEvent ( QFocusEvent * );
    virtual void focusOutEvent ( QFocusEvent * );
//    virtual void mousePressEvent ( QMouseEvent * );

    virtual bool eventFilter( QObject *o, QEvent *e );

    virtual void paintEvent ( QPaintEvent * );

private:

    void doEnterKey();

    QStringList hist;
    QStringList::Iterator current;
    void ColourPickerPopUp();
    int m_height;

    bool m_drawrect;

};

#endif
