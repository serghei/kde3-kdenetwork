/* -------------------------------------------------------------

   sets.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   dbSetsDialog    dialog for editing the user defined database sets
   
 ------------------------------------------------------------- */
 
#ifndef _KDICT_SETS_H_
#define _KDICT_SETS_H_

#include <kdialogbase.h>

class QListBox;


//*********  DbSetsDialog  ******************************************


class DbSetsDialog : public KDialogBase
{
  Q_OBJECT

public:

  DbSetsDialog(QWidget *parent=0, const char *name=0);

signals:

  void setsChanged();
  void dialogClosed();

protected:
  void hideEvent(QHideEvent *);

private slots:

  void newPressed();
  void deletePressed();
  void allLeftPressed();
  void leftPressed();
  void rightPressed();
  void allRightPressed();
  void closePressed();
  void transferSet();
  void activateSet(int num);
  void leftSelected(int index);
  void rightSelected(int index);
  void leftHighlighted(int index);
  void rightHighlighted(int index);

private:

  void checkButtons();

  QComboBox *w_set;
  QListBox *w_leftBox, *w_rightBox;
  QPushButton *w_delete,*w_save,*w_allLeft,*w_left,*w_right,*w_allRight;
};

#endif
