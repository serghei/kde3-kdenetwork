/* ascii table for ksirc - Robbie Ward <linuxphreak@gmx.co.uk>*/

#ifndef CHARSELECTOR_H
#define CHARSELECTOR_H

#include <kdialog.h>
#include <kdebug.h>
#include <kcharselect.h>

class QVBoxLayout;
class QPushButton;

class charSelector : public KDialog
{
  Q_OBJECT
  public:
    charSelector(QWidget *parent=0, const char *name=0);
    ~charSelector();

  virtual bool eventFilter ( QObject * watched, QEvent * e );

  public slots:
    void insertText();
    QString currentText() { return charSelect->chr(); }

    void setFont( const QString &font );

  protected slots:
    virtual void reject();

  signals:
    void clicked();

  protected:
    virtual void keyPressEvent(QKeyEvent *e);

  private:
    KCharSelect *charSelect;
    QVBoxLayout* testLayout;
    QPushButton *insertButton;
};
#endif
