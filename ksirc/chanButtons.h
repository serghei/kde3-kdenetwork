/* mode buttons for ksirc - Robbie Ward <linuxphreak@gmx.co.uk>*/

#ifndef CHANBUTTONS_H
#define CHANBUTTONS_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <kdialog.h>

class QHBoxLayout;
class QVBoxLayout;
class KPopupMenu;
class chanbuttonsDialog;
class KIntSpinBox;
class QLineEdit;
class KSircProcess;
class KPushButton;

class chanButtons : public QWidget
{
  Q_OBJECT
  friend class KSircTopLevel;
  public:
    chanButtons(KSircProcess *proc, QWidget* parent=0, const char* name=0);
    ~chanButtons() {};

    void setProtectMode(bool value) { protectButton->setOn(value); }
    void setModerateMode(bool value) { moderateButton->setOn(value); }
    void setNooutsideMode(bool value) { outsideButton->setOn(value); }
    void setButtonsEnabled(bool);
    void setMenuItemMode(int, bool);

  signals:
    void mode(QString, int, QString nick = QString::null);  /// int == 0 channel mode, int == 1 user nick mode

  private slots:
    void protectMode();
    void outsideMode();
    void moderateMode();
    void invite();
    void limited();
    void key();
    void secret();
    void invisible();
    void wallops();
    void serverNotices();

  private:
    int toggleMenu[7];
    QHBoxLayout *layout;
    QPushButton *protectButton;
    QPushButton *moderateButton;
    QPushButton *outsideButton;
    QPushButton *menuButton;
    KPopupMenu *Popupmenu;
    chanbuttonsDialog *chanDialog;
    KSircProcess *m_proc;
};

class chanbuttonsDialog : public KDialog
{
  Q_OBJECT
  friend class chanButtons;
  public:
    enum type { limited, key };
    chanbuttonsDialog(const type &modeType, QWidget *parent=0, const char *name=0, bool modal=true);
    ~chanbuttonsDialog() {};

  private slots:
    void keyString();
    void limitedUsers();
    QString sendKey() { return m_sendKey; }
    int sendLimitedUsers() { return m_sendLimitedUsers; };

  private:
    QString m_sendKey;
    int m_sendLimitedUsers;
    QVBoxLayout *LimitedLayout;
    QHBoxLayout *Layout;
    KIntSpinBox *SpinBox;
    QLineEdit *EditBox;
    KPushButton *okButton;
    KPushButton *cancelButton;
};
#endif
