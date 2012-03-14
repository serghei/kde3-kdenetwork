/* mode buttons for ksirc - Robbie Ward <linuxphreak@gmx.co.uk>*/

#include "chanButtons.h"
#include "chanButtons.moc"

#include <klocale.h>
#include <kpopupmenu.h>
#include <knuminput.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <qlayout.h>
#include <qtooltip.h>
#include <qlineedit.h>

#include "ksircprocess.h"

static void makeSquare(QWidget *w)
{
   QSize sh = w->sizeHint();
   int s = sh.height();
   w->setFixedSize(s,s);
}

chanButtons::chanButtons(KSircProcess *proc, QWidget *parent, const char *name)
: QWidget(parent, name),
  m_proc(proc)
{
  Popupmenu = new KPopupMenu( this );
  Popupmenu->insertTitle(i18n("Channel Modes"));
  toggleMenu[0] = Popupmenu->insertItem(i18n("i (invite-only)"), this, SLOT(invite()));
  toggleMenu[1] = Popupmenu->insertItem(i18n("l (limited users)"), this, SLOT(limited()));
  toggleMenu[2] = Popupmenu->insertItem(i18n("k (key to join)"), this, SLOT(key()));
  toggleMenu[3] = Popupmenu->insertItem(i18n("s (secret)"), this, SLOT(secret()));
  Popupmenu->insertSeparator();
  Popupmenu->insertTitle(i18n("User Modes"));
  toggleMenu[4] = Popupmenu->insertItem(i18n("i (be invisible)"), this, SLOT(invisible()));
  toggleMenu[5] = Popupmenu->insertItem(i18n("w (receive wallops)"), this, SLOT(wallops()));
  toggleMenu[6] = Popupmenu->insertItem(i18n("s (get server notices)"), this, SLOT(serverNotices()));

  layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setMargin(0);

  protectButton = new QPushButton(i18n("T"), this);
  protectButton->setToggleButton(true);
  makeSquare(protectButton);
  QToolTip::add(protectButton, i18n("Only op'ed users can change the topic"));
  connect(protectButton, SIGNAL(clicked()), this, SLOT(protectMode()));
  layout->addWidget(protectButton);

  outsideButton = new QPushButton(i18n("N"), this);
  outsideButton->setToggleButton(true);
  makeSquare(outsideButton);
  QToolTip::add(outsideButton, i18n("No outside messages"));
  connect(outsideButton, SIGNAL(clicked()), this, SLOT(outsideMode()));
  layout->addWidget(outsideButton);

  moderateButton = new QPushButton(i18n("M"), this);
  moderateButton->setToggleButton(true);
  makeSquare(moderateButton);
  QToolTip::add(moderateButton, i18n("Only op'ed users and voiced users (+v) can speak"));
  connect(moderateButton, SIGNAL(clicked()), this, SLOT(moderateMode()));
  layout->addWidget(moderateButton);

  menuButton = new QPushButton(i18n("..."), this);
  makeSquare(menuButton);
  menuButton->setFixedWidth(menuButton->width()*2);
  menuButton->setPopup(Popupmenu);
  QToolTip::add(menuButton, i18n("More mode commands"));
  layout->addWidget(menuButton);
}

void chanButtons::invite()
{
  if (Popupmenu->isItemChecked(toggleMenu[0])) {
    Popupmenu->setItemChecked(toggleMenu[0], false);
    emit mode(QString("-i"), 0);
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[0], true);
    emit mode(QString("+i"), 0);
  }
}

void chanButtons::limited()
{
  chanDialog = new chanbuttonsDialog(chanbuttonsDialog::limited);
  chanDialog->exec();
  if (chanDialog->sendLimitedUsers() == 0) {
    Popupmenu->setItemChecked(toggleMenu[1], false);
    emit mode(QString("-l"), 0);
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[1], true);
    emit mode(QString("+l %1").arg(chanDialog->sendLimitedUsers()), 0);
  }
  delete chanDialog;
}

void chanButtons::key()
{
  chanDialog = new chanbuttonsDialog(chanbuttonsDialog::key);
  if (Popupmenu->isItemChecked(toggleMenu[2])) {
    chanDialog->exec();
    Popupmenu->setItemChecked(toggleMenu[2], false);
    emit mode(QString("-k %1").arg(chanDialog->sendKey()), 0);
  }
  else {
    chanDialog->exec();
    Popupmenu->setItemChecked(toggleMenu[2], true);
    emit mode(QString("+k %1").arg(chanDialog->sendKey()), 0);
  }
  delete chanDialog;
}

void chanButtons::secret()
{
  if (Popupmenu->isItemChecked(toggleMenu[3])) {
    Popupmenu->setItemChecked(toggleMenu[3], false);
    emit mode(QString("-s"), 0);
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[3], true);
    emit mode(QString("+s"), 0);
  }
}

void chanButtons::invisible()
{
  if (Popupmenu->isItemChecked(toggleMenu[4])) {
    Popupmenu->setItemChecked(toggleMenu[4], false);
    emit mode(QString("-i"), 1, m_proc->getNick());
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[4], true);
    emit mode(QString("+i"), 1, m_proc->getNick());
  }
}

void chanButtons::wallops()
{
  if (Popupmenu->isItemChecked(toggleMenu[5])) {
    Popupmenu->setItemChecked(toggleMenu[5], false);
    emit mode(QString("-w"), 1, m_proc->getNick());
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[5], true);
    emit mode(QString("+w"), 1, m_proc->getNick());
  }
}

void chanButtons::serverNotices()
{
  if (Popupmenu->isItemChecked(toggleMenu[6])) {
    Popupmenu->setItemChecked(toggleMenu[6], false);
    emit mode(QString("-s"), 1, m_proc->getNick());
  }
  else {
    Popupmenu->setItemChecked(toggleMenu[6], true);
    emit mode(QString("+s"), 1, m_proc->getNick());
  }
}

void chanButtons::protectMode()
{
  if (protectButton->isOn())
    emit mode(QString("+t"), 0);
  else
    emit mode(QString("-t"), 0);
}

void chanButtons::outsideMode()
{
  if (outsideButton->isOn())
    emit mode(QString("+n"), 0);
  else
    emit mode(QString("-n"), 0);
}

void chanButtons::moderateMode()
{
  if (moderateButton->isOn())
    emit mode(QString("+m"), 0);
  else
    emit mode(QString("-m"), 0);
}

void chanButtons::setButtonsEnabled(bool enabled)
{
  // Set all the channel mode entries disabled if non-op else enable them
  protectButton->setEnabled(enabled);
  outsideButton->setEnabled(enabled);
  moderateButton->setEnabled(enabled);
  Popupmenu->setItemEnabled(toggleMenu[0], enabled);
  Popupmenu->setItemEnabled(toggleMenu[1], enabled);
  Popupmenu->setItemEnabled(toggleMenu[2], enabled);
  Popupmenu->setItemEnabled(toggleMenu[3], enabled);
}

void chanButtons::setMenuItemMode(int id, bool value)
{
  Popupmenu->setItemChecked(toggleMenu[id], value);
}

chanbuttonsDialog::chanbuttonsDialog(const type &modeType, QWidget *parent,
                          const char *name, bool modal) : KDialog(parent, name, modal)
{
  m_sendKey = "";
  m_sendLimitedUsers = 0;
  resize(190, 82);
  setCaption(i18n("Limit Number of Users"));
  LimitedLayout = new QVBoxLayout(this);
  LimitedLayout->setSpacing(9);
  LimitedLayout->setMargin(11);

  switch (modeType) {
    case limited:
      SpinBox = new KIntSpinBox(this);
      LimitedLayout->addWidget(SpinBox);
      break;
    case key:
      EditBox = new QLineEdit(this);
      LimitedLayout->addWidget(EditBox);
      break;
  }

  Layout = new QHBoxLayout;
  Layout->setSpacing(6);
  Layout->setMargin(0);

  okButton = new KPushButton(KStdGuiItem::ok(), this);
  okButton->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1,
                                      okButton->sizePolicy().hasHeightForWidth()));
  Layout->addWidget(okButton);
  switch (modeType) {
    case limited:
      connect(okButton, SIGNAL(clicked()), SLOT(limitedUsers()));
      break;
    case key:
      connect(okButton, SIGNAL(clicked()), SLOT(keyString()));
      break;
  }

  Layout->addWidget(okButton);

  cancelButton = new KPushButton(KStdGuiItem::cancel(), this);
  cancelButton->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1,
                                      cancelButton->sizePolicy().hasHeightForWidth()));
  Layout->addWidget(cancelButton);
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  LimitedLayout->addLayout(Layout);

  QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  LimitedLayout->addItem(spacer);
}

void chanbuttonsDialog::keyString()
{
  m_sendKey = EditBox->text();
  accept();
}

void chanbuttonsDialog::limitedUsers()
{
  m_sendLimitedUsers = SpinBox->value();
  accept();
}
