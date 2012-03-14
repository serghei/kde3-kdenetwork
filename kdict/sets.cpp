/* -------------------------------------------------------------

   sets.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   DbSetsDialog    dialog for editing the user defined database sets
   
 ------------------------------------------------------------- */

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <kiconloader.h>
#include <kseparator.h>
#include <klocale.h>

#include "options.h"
#include "sets.h"


//********* DbSetsDialog  ******************************************


DbSetsDialog::DbSetsDialog(QWidget *parent, const char *name)
  : KDialogBase(Plain, i18n("Database Sets"),Close | Help, Close, parent, name, false, true)
{
  QFrame* page=plainPage();

  QStringList sets;
  for(unsigned int i=1;i<global->databaseSets.count()+1;i++)
    sets.append(global->databases[i]);

  QVBoxLayout * topLayout = new QVBoxLayout(page, 0, 0);

  QHBoxLayout * subLayout1 = new QHBoxLayout(5);
  topLayout->addLayout(subLayout1,0);

  w_set = new QComboBox(true,page);
  w_set->setFixedHeight(w_set->sizeHint().height());
  w_set->setInsertionPolicy (QComboBox::NoInsertion);
  w_set->insertStringList(sets);
  connect(w_set, SIGNAL(activated(int)),this, SLOT(activateSet(int)));
  QLabel *l = new QLabel(w_set, i18n("&Set:"),page);
  l->setMinimumSize(l->sizeHint());
  subLayout1->addWidget(l,0);
  subLayout1->addWidget(w_set,1);

  subLayout1->addSpacing(8);

  w_save = new QPushButton(i18n("S&ave"),page);
  connect(w_save,SIGNAL(clicked()),this, SLOT(transferSet()));
  subLayout1->addWidget(w_save,0);

  QPushButton *btn = new QPushButton(i18n("&New"),page);
  btn->setMinimumSize(btn->sizeHint());
  connect(btn, SIGNAL(clicked()),this, SLOT(newPressed()));
  subLayout1->addWidget(btn,0);

  w_delete = new QPushButton(i18n("&Delete"),page);
  w_delete->setMinimumSize(w_delete->sizeHint());
  connect(w_delete, SIGNAL(clicked()),this, SLOT(deletePressed()));
  subLayout1->addWidget(w_delete,0);

  topLayout->addSpacing(8);

  KSeparator *sep = new KSeparator(page);
  topLayout->addWidget(sep,0);

  topLayout->addSpacing(8);

  QGridLayout * subLayout2 = new QGridLayout(7,3,6);
  topLayout->addLayout(subLayout2,1);

  w_leftBox = new QListBox(page);
  connect(w_leftBox, SIGNAL(selected(int)),this, SLOT(leftSelected(int)));
  connect(w_leftBox, SIGNAL(highlighted(int)),this, SLOT(leftHighlighted(int)));
  QLabel *leftLabel = new QLabel(w_leftBox, i18n("S&elected databases:"),page);
  leftLabel->setMinimumSize(leftLabel->sizeHint());
  subLayout2->addWidget(leftLabel,0,0);
  subLayout2->addMultiCellWidget(w_leftBox,1,6,0,0);

  w_allLeft = new QPushButton(page);
  w_allLeft->setIconSet(BarIconSet("2leftarrow"));
  connect(w_allLeft, SIGNAL(clicked()),this, SLOT(allLeftPressed()));
  subLayout2->addWidget(w_allLeft,2,1);

  w_left = new QPushButton(page);
  w_left->setIconSet(BarIconSet("1leftarrow"));
  connect(w_left, SIGNAL(clicked()),this, SLOT(leftPressed()));
  subLayout2->addWidget(w_left,3,1);

  w_right = new QPushButton(page);
  w_right->setIconSet(BarIconSet("1rightarrow"));
  connect(w_right, SIGNAL(clicked()),this, SLOT(rightPressed()));
  subLayout2->addWidget(w_right,4,1);

  w_allRight = new QPushButton(page);
  w_allRight->setIconSet(BarIconSet("2rightarrow"));
  connect(w_allRight, SIGNAL(clicked()),this, SLOT(allRightPressed()));
  subLayout2->addWidget(w_allRight,5,1);

  w_rightBox = new QListBox(page);
  connect(w_rightBox, SIGNAL(selected(int)),this, SLOT(rightSelected(int)));
  connect(w_rightBox, SIGNAL(highlighted(int)),this, SLOT(rightHighlighted(int)));
  QLabel *rightLabel = new QLabel(w_rightBox, i18n("A&vailable databases:"),page);
  rightLabel->setMinimumSize(rightLabel->sizeHint());
  subLayout2->addWidget(rightLabel,0,2);
  subLayout2->addMultiCellWidget(w_rightBox,1,6,2,2);

  subLayout2->setRowStretch(1,1);
  subLayout2->setRowStretch(6,1);
  subLayout2->setColStretch(0,1);
  subLayout2->setColStretch(2,1);

  setHelp("database-sets");

  if (global->setsSize.isValid())
    resize(global->setsSize);
  else
    resize(300,200);

  if ((global->currentDatabase>=1)&&(global->currentDatabase<=global->databaseSets.count()))
    activateSet(global->currentDatabase-1);
  else
    activateSet(0);
  w_set->setFocus();
}


void DbSetsDialog::hideEvent(QHideEvent *)
{
  global->setsSize = size();
  emit(dialogClosed());
}


void DbSetsDialog::newPressed()
{
  QStringList *temp = new QStringList;
  temp->append(i18n("New Set"));
  global->databaseSets.append(temp);
  global->databases.insert(global->databases.at(global->databaseSets.count()),i18n("New Set"));
  if (global->currentDatabase >= global->databaseSets.count())
    global->currentDatabase++;

  QStringList sets;    // reread sets, because w_sets internal list is not correct in all cases
  for(unsigned int i=1;i<global->databaseSets.count()+1;i++)
    sets.append(global->databases[i]);
  w_set->clear();
  w_set->insertStringList(sets);
  emit(setsChanged());
  activateSet(global->databaseSets.count()-1);
  w_set->setFocus();
}


void DbSetsDialog::deletePressed()
{
  int pos = w_set->currentItem();
  if (pos>=0) {
    global->databaseSets.remove(global->databaseSets.at(pos));
    global->databases.remove(global->databases.at(pos+1));
    if ((int)global->currentDatabase >= pos+1)
      global->currentDatabase--;
    w_set->removeItem(pos);
    if (pos >= w_set->count())
      pos--;
    emit(setsChanged());
    activateSet(pos);
    w_set->setFocus();
  }
}


void DbSetsDialog::allLeftPressed()
{
  while (w_rightBox->count()) {
    w_leftBox->insertItem(w_rightBox->text(0));
    w_rightBox->removeItem(0);
  }
  w_leftBox->sort();
  checkButtons();
}


void DbSetsDialog::leftPressed()
{
  int pos = w_rightBox->currentItem();
  if (pos>=0) {
    w_leftBox->insertItem(w_rightBox->text(pos));
    w_leftBox->sort();
    w_rightBox->removeItem(pos);
    if (pos >= (int)w_rightBox->count())
      pos--;
    if (pos>=0)
      w_rightBox->setCurrentItem(pos);
    checkButtons();
  }
}


void DbSetsDialog::rightPressed()
{
  int pos = w_leftBox->currentItem();
  if (pos>=0) {
    w_rightBox->insertItem(w_leftBox->text(pos));
    w_rightBox->sort();
    w_leftBox->removeItem(pos);
    if (pos >= (int)w_leftBox->count())
      pos--;
    if (pos>=0)
      w_leftBox->setCurrentItem(pos);
    checkButtons();
  }
}


void DbSetsDialog::allRightPressed()
{
  while (w_leftBox->count()) {
    w_rightBox->insertItem(w_leftBox->text(0));
    w_leftBox->removeItem(0);
  }
  w_rightBox->sort();
  checkButtons();
}


void DbSetsDialog::closePressed()
{
  accept();
  global->setsSize = size();
  emit(dialogClosed());
}


void DbSetsDialog::transferSet()
{
  global->databaseSets.at(w_set->currentItem())->clear();
  global->databaseSets.at(w_set->currentItem())->append(w_set->currentText());
  for (unsigned int i = 0;i<w_leftBox->count();i++)
    global->databaseSets.at(w_set->currentItem())->append(w_leftBox->text(i));
  global->databases.remove(global->databases.at(w_set->currentItem()+1));
  global->databases.insert(global->databases.at(w_set->currentItem()+1),w_set->currentText());
  w_set->changeItem(w_set->currentText(),w_set->currentItem());
  emit(setsChanged());
}


void DbSetsDialog::activateSet(int num)
{
  w_leftBox->clear();
  w_rightBox->clear();

  if ((num < 0)||(num>=(int)global->databaseSets.count())) {
    w_set->clearEdit();
    w_delete->setEnabled(false);
    w_save->setEnabled(false);
    w_rightBox->repaint(true);       // Workaround for repaint-bug
    w_leftBox->repaint(true);       // Workaround for repaint-bug
  } else {
    w_set->setCurrentItem(num);
    for (unsigned int i=0;i<global->serverDatabases.count();i++)
      if (global->databaseSets.at(num)->findIndex(global->serverDatabases[i])>0)
        w_leftBox->insertItem(global->serverDatabases[i]);
      else
        w_rightBox->insertItem(global->serverDatabases[i]);
    w_leftBox->sort();
    w_rightBox->sort();
    w_delete->setEnabled(true);
    w_save->setEnabled(true);
    if (w_rightBox->count()==0)
      w_rightBox->repaint(true);       // Workaround for repaint-bug
    if (w_leftBox->count()==0)
      w_leftBox->repaint(true);      // Workaround for repaint-bug
    w_leftBox->clearSelection();
    w_leftBox->centerCurrentItem();
    w_rightBox->clearSelection();
    w_rightBox->centerCurrentItem();
  }
  checkButtons();
}


void DbSetsDialog::leftSelected(int)
{
  rightPressed();
}


void DbSetsDialog::rightSelected(int)
{
  leftPressed();
}


void DbSetsDialog::leftHighlighted(int)
{
  w_right->setEnabled(true);
}


void DbSetsDialog::rightHighlighted(int)
{
  w_left->setEnabled(true);
}

void DbSetsDialog::checkButtons()
{
  w_allLeft->setEnabled((w_rightBox->count()>0));
  w_allRight->setEnabled((w_leftBox->count()>0));
  w_right->setEnabled((w_leftBox->currentItem()>=0));
  w_left->setEnabled((w_rightBox->currentItem()>=0));
}

//--------------------------------

#include "sets.moc"
