#ifndef ENTER_COMBO_H
#define ENTER_COMBO_H

#include <qcombobox.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qlineedit.h>

#undef KeyPress // X headers...

class EnterCombo : public QComboBox {
  Q_OBJECT
public:
  EnterCombo ( QWidget * parent=0, const char * name=0 )
    : QComboBox(TRUE, parent, name)
    {
    }
  EnterCombo ( bool rw, QWidget * parent=0, const char * name=0 )
      : QComboBox(rw, parent, name)
      {
        QKeyEvent ke(QEvent::KeyPress, SHIFT|Key_Home, 0, 0);
        keyPressEvent(&ke);
      }

  virtual void show(){
    QComboBox::show();
    lineEdit()->selectAll();    
  }

signals:
  void enterPressed();
  
protected:
  virtual void keyPressEvent( QKeyEvent *e );
};

#endif
