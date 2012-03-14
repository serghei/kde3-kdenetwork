#ifndef NEW_WINDOW_DIALOG_H
#define NEW_WINDOW_DIALOG_H

#include <kdialogbase.h>

#include "ksircchannel.h"

class KHistoryCombo;
class KLineEdit;

class NewWindowDialog : public KDialogBase
{
  Q_OBJECT

  public:

    NewWindowDialog(const KSircChannel &, QWidget * parent = 0, const char * name = 0);
    ~NewWindowDialog();

  protected slots:

    void slotOk();
  void slotTextChanged( const QString &);
  signals:

    void openTopLevel(const KSircChannel &);

  private:

      KHistoryCombo * m_combo;
      KLineEdit * m_le;
      KSircChannel m_channelInfo;
};

#endif
