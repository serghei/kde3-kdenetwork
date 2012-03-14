#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <klineedit.h>
#include "NewWindowDialog.h"

NewWindowDialog::NewWindowDialog(const KSircChannel &channelInfo, QWidget * parent, const char * name)
    : KDialogBase(parent, name, true, i18n("New Window For"), Ok|Cancel, Ok, true),
      m_channelInfo(channelInfo)
{
  QHBox * w = makeHBoxMainWidget();

  QLabel * l = new QLabel(i18n("C&hannel/Nick:"), w);

  m_combo = new KHistoryCombo(w);
  m_combo->setFocus();

  // we don't need duplicated channel into the list
  m_combo->setDuplicatesEnabled( false );

  l->setBuddy(m_combo);

  QLabel * l2 = new QLabel(i18n("&Key:"), w);
  m_le = new KLineEdit(w);
  m_le->setEnabled(false);
  l2->setBuddy(m_le);

  connect(
      m_combo, SIGNAL(activated(const QString &)),
      m_combo, SLOT(addToHistory(const QString &)));
  connect( m_combo->lineEdit(), SIGNAL(textChanged ( const QString & )),
	   this, SLOT( slotTextChanged( const QString &)));

  KConfig *kConfig = kapp->config();
  KConfigGroupSaver saver(kConfig, "Recent");
  m_combo->setHistoryItems(kConfig->readListEntry("Channels"));
  slotTextChanged( m_combo->lineEdit()->text());
}

NewWindowDialog::~NewWindowDialog()
{
  KConfig *kConfig = kapp->config();
  KConfigGroupSaver saver(kConfig, "Recent");
  kConfig->writeEntry("Channels", m_combo->historyItems());
}

void NewWindowDialog::slotTextChanged( const QString &text)
{
  enableButtonOK( !text.isEmpty() );

  if(text[0] == "#" || text[0] == "&")
    m_le->setEnabled(true);
  else
    m_le->setEnabled(false);
}


  void
NewWindowDialog::slotOk()
{
  m_channelInfo.setChannel(m_combo->lineEdit()->text().lower());
  if(m_le->isEnabled())
    m_channelInfo.setKey(m_le->text());
  emit(openTopLevel(m_channelInfo));
  KDialogBase::slotOk();
}

#include "NewWindowDialog.moc"

