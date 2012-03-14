/* -------------------------------------------------------------

   matchview.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   MatchView  This widget contains the list of matching definitions

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qregexp.h>

#include <kpopupmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "dict.h"
#include "options.h"
#include "matchview.h"


//*********  MatchViewItem  ********************************************


MatchViewItem::MatchViewItem(QListView *view, const QString &text)
  : QListViewItem(view,text)
{
}


MatchViewItem::MatchViewItem(QListView *view,QListViewItem *after,const QString &text)
  : QListViewItem(view,after,text)
{
}


MatchViewItem::MatchViewItem(QListViewItem *item,const QString &text,const QString &commandStr)
: QListViewItem(item,text), command(commandStr)
{
}


MatchViewItem::MatchViewItem(QListViewItem *item,QListViewItem *after,const QString &text,const QString &commandStr)
: QListViewItem(item,after,text), command(commandStr)
{
}


MatchViewItem::~MatchViewItem()
{
}


void MatchViewItem::setOpen(bool o)
{
  if (o && !childCount()) {
    listView()->setUpdatesEnabled(false);

    MatchViewItem *sub=0;
    QString command, label;
    QRegExp exp("\"*\"", true, true);
    QStringList::iterator it;
    for (it = subEntrys.begin(); it != subEntrys.end(); ++it) {
      command = "define ";
      command += (*it);
      command += "\r\n";
      exp.search((*it));
      label = exp.cap();
      label = label.mid(1, label.length()-2); // remove quotes
      if (sub)
        sub = new MatchViewItem(this, sub, label, command);
      else
        sub = new MatchViewItem(this, label, command);
    }

    subEntrys.clear();

    listView()->setUpdatesEnabled(true);
  }

  if (childCount())
    QListViewItem::setOpen(o);
}


void MatchViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  if(command.isEmpty()) {
    QFont font=p->font();
    font.setBold(true);
    p->setFont(font);
  }
  QListViewItem::paintCell(p,cg,column,width,alignment);
}


//********* MatchView  ******************************************


MatchView::MatchView(QWidget *parent, const char *name)
  : QWidget(parent,name),getOn(false),getAllOn(false)
{
  setCaption(kapp->makeStdCaption(i18n("Match List")));

  QVBoxLayout * boxLayout = new QVBoxLayout(this, 1, 0);

  boxLayout->addSpacing(1);
  w_strat = new QComboBox(false,this);
  w_strat->setFixedHeight(w_strat->sizeHint().height());
  connect(w_strat,SIGNAL(activated(int)),this,SLOT(strategySelected(int)));
  boxLayout->addWidget(w_strat,0);
  boxLayout->addSpacing(1);

  w_list = new QListView(this);
  w_list->setFocusPolicy(QWidget::StrongFocus);
  w_list->header()->hide();
  w_list->addColumn("foo");
  w_list->setColumnWidthMode(0,QListView::Maximum);
  w_list->setColumnWidth(0,0);
  w_list->setSelectionMode(QListView::Extended);
  w_list->setTreeStepSize(18);
  w_list->setSorting(-1);                                 // disable sorting
  w_list->setMinimumHeight(w_strat->sizeHint().height());
  connect(w_list,SIGNAL(selectionChanged()),SLOT(enableGetButton()));
  connect(w_list,SIGNAL(returnPressed(QListViewItem *)),SLOT(returnPressed(QListViewItem *)));
  connect(w_list,SIGNAL(doubleClicked(QListViewItem *)),SLOT(getOneItem(QListViewItem *)));
  connect(w_list,SIGNAL(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)),
                 SLOT(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)));
  connect(w_list,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int)),SLOT(buildPopupMenu(QListViewItem *,const QPoint &,int)));
  boxLayout->addWidget(w_list,1);

  boxLayout->addSpacing(1);
  w_get = new QPushButton(i18n("&Get Selected"),this);
  w_get->setFixedHeight(w_get->sizeHint().height()-3);
  w_get->setMinimumWidth(w_get->sizeHint().width()-20);
  w_get->setEnabled(false);
  connect(w_get, SIGNAL(clicked()), this, SLOT(getSelected()));
  boxLayout->addWidget(w_get,0);

  w_getAll = new QPushButton(i18n("Get &All"),this);
  w_getAll->setFixedHeight(w_getAll->sizeHint().height()-3);
  w_getAll->setMinimumWidth(w_getAll->sizeHint().width()-20);
  w_getAll->setEnabled(false);
  connect(w_getAll, SIGNAL(clicked()), this, SLOT(getAll()));
  boxLayout->addWidget(w_getAll,0);
  connect(interface,SIGNAL(matchReady(const QStringList &)),this,SLOT(newList(const QStringList &)));
  rightBtnMenu = new KPopupMenu();
}


MatchView::~MatchView()
{
}


void MatchView::updateStrategyCombo()
{
  w_strat->clear();
  w_strat->insertStringList(global->strategies);
  w_strat->setCurrentItem(global->currentStrategy);
}


bool MatchView::selectStrategy(const QString &strategy) const
{
  int newCurrent = global->strategies.findIndex(strategy);
  if (newCurrent == -1)
    return false;
  else {
    global->currentStrategy = newCurrent;
    w_strat->setCurrentItem(global->currentStrategy);
    return true;
  }
}


void MatchView::match(const QString &query)
{
  interface->match(query.utf8());
}


void MatchView::closeEvent ( QCloseEvent * e )
{
  e->accept();                            // hides the widget
  emit(windowClosed());
}


void MatchView::strategySelected(int num)
{
  global->currentStrategy = num;
}


void MatchView::enableGetButton()
{
  if (w_getAll->isEnabled()) {
    w_get->setEnabled(true);
    getOn = true;
  }
}


void MatchView::mouseButtonPressed(int button, QListViewItem *, const QPoint &, int)
{
  if (button == MidButton)
    emit(clipboardRequested());
}


void MatchView::returnPressed(QListViewItem *)
{
  getSelected();
}


void MatchView::getOneItem(QListViewItem *i)
{
  QStringList defines;

  if ((!i->childCount())&&(i->parent()))
    defines.append(((MatchViewItem *)(i))->command);
  else {
    i = i->firstChild();
    while (i) {
      defines.append(((MatchViewItem *)(i))->command);
      i = i->nextSibling();
    }
  }

  doGet(defines);
}


void MatchView::getSelected()
{
  QStringList defines;
  MatchViewItem *top = static_cast<MatchViewItem*>(w_list->firstChild());
  MatchViewItem *sub;

  while (top) {
    if (top->isSelected()&&(!top->subEntrys.isEmpty())) {
      QString command;
      QStringList::iterator it;
      for (it = top->subEntrys.begin(); it != top->subEntrys.end(); ++it) {
        command = "define ";
        command += (*it);
        command += "\r\n";
        defines.append(command);
      }
    } else {
      sub = static_cast<MatchViewItem*>(top->firstChild());
      while (sub) {
        if (top->isSelected()||sub->isSelected())
          defines.append(sub->command);
        sub = static_cast<MatchViewItem*>(sub->nextSibling());
      }
    }
    top = static_cast<MatchViewItem*>(top->nextSibling());
  }
  doGet(defines);
}


void MatchView::getAll()
{
  QStringList defines;
  MatchViewItem *top = static_cast<MatchViewItem*>(w_list->firstChild());
  MatchViewItem *sub;

  while (top) {
    if (!top->subEntrys.isEmpty()) {
      QString command;
      QStringList::iterator it;
      for (it = top->subEntrys.begin(); it != top->subEntrys.end(); ++it) {
        command = "define ";
        command += (*it);
        command += "\r\n";
        defines.append(command);
      }
    } else {
      sub = static_cast<MatchViewItem*>(top->firstChild());
      while (sub) {
        defines.append(sub->command);
        sub = static_cast<MatchViewItem*>(sub->nextSibling());
      }
    }
    top = static_cast<MatchViewItem*>(top->nextSibling());
  }
  doGet(defines);
}


void MatchView::doGet(QStringList &defines)
{
  if (defines.count() > 0) {
    if (defines.count() > global->maxDefinitions) {
      KMessageBox::sorry(global->topLevel,i18n("You have selected %1 definitions,\nbut Kdict will fetch only the first %2 definitions.\nYou can modify this limit in the Preferences Dialog.")
                                             .arg(defines.count()).arg(global->maxDefinitions));
      while (defines.count()>global->maxDefinitions)
        defines.pop_back();
    }
    interface->getDefinitions(defines);
  }
}


void MatchView::newList(const QStringList &matches)
{
  MatchViewItem *top=0;
  bool initialOpen = (matches.count()<200);
  int numDb = 0;

  rightBtnMenu->hide();
  w_list->clear();
  w_list->setColumnWidth(0,0);
  w_list->setUpdatesEnabled(false);
  w_get->setEnabled(false);
  getOn = false;

  if (matches.isEmpty()) {
    w_list->setColumnWidth(0,w_get->width()-5);
    w_list->setRootIsDecorated(false);
    w_getAll->setEnabled(false);
    getAllOn = false;
    top = new MatchViewItem(w_list,top,i18n(" No Hits"));
  } else {
    w_list->setRootIsDecorated(true);
    w_getAll->setEnabled(true);
    getAllOn = true;
    QString lastDb, db, match;

    QStringList::const_iterator it;
    for (it = matches.begin(); it != matches.end(); ++it) {
      db = (*it).section(' ', 0, 0);

      if (db != lastDb) {
        numDb++;
        if (top) {
          top->setOpen(initialOpen);
          top = new MatchViewItem(w_list, top, db);
        } else
          top = new MatchViewItem(w_list, db);
        top->setExpandable(true);
        lastDb = db;
      }

      if (top)
        top->subEntrys.append(*it);
    }

    if ((numDb == 1)||(initialOpen))
      top->setOpen(true);
  }

  w_list->setUpdatesEnabled(true);
  w_list->repaint();
  w_list->setFocus();
}


// construct the right-mouse-button-popup-menu on demand
void MatchView::buildPopupMenu(QListViewItem *i, const QPoint &_point, int)
{
  rightBtnMenu->clear();

  if ((i!=0L)&&(i->isExpandable()||i->parent())) {
    popupCurrent = (MatchViewItem *)(i);
    rightBtnMenu->insertItem(i18n("&Get"),this,SLOT(popupGetCurrent()));
    if (!i->isExpandable()) {       // toplevel item -> only "get"
      rightBtnMenu->insertItem(i18n("&Match"),this,SLOT(popupMatchCurrent()));
      rightBtnMenu->insertItem(i18n("&Define"),this,SLOT(popupDefineCurrent()));
    }
    rightBtnMenu->insertSeparator();
  }

  kapp->clipboard()->setSelectionMode(false);
  QString text = kapp->clipboard()->text();
  if (text.isEmpty()) {
    kapp->clipboard()->setSelectionMode(true);
    text = kapp->clipboard()->text();
  }
  if (!text.isEmpty()) {
    popupClip = kapp->clipboard()->text();
    rightBtnMenu->insertItem(i18n("Match &Clipboard Content"),this,SLOT(popupMatchClip()));
    rightBtnMenu->insertItem(SmallIcon("define_clip"),i18n("D&efine Clipboard Content"),this,SLOT(popupDefineClip()));
    rightBtnMenu->insertSeparator();
  }

  int ID = rightBtnMenu->insertItem(i18n("Get &Selected"),this,SLOT(getSelected()));
  rightBtnMenu->setItemEnabled(ID,getOn);
  ID = rightBtnMenu->insertItem(i18n("Get &All"),this,SLOT(getAll()));
  rightBtnMenu->setItemEnabled(ID,getAllOn);

  if (w_list->childCount()) {
    rightBtnMenu->insertSeparator();
    rightBtnMenu->insertItem(i18n("E&xpand List"),this,SLOT(expandList()));
    rightBtnMenu->insertItem(i18n("C&ollapse List"),this,SLOT(collapseList()));
  }

  rightBtnMenu->popup(_point);
}


void MatchView::popupGetCurrent()
{
  getOneItem(popupCurrent);
}


void MatchView::popupDefineCurrent()
{
  emit(defineRequested(popupCurrent->text(0)));
}


void MatchView::popupMatchCurrent()
{
  emit(matchRequested(popupCurrent->text(0)));
}


void MatchView::popupDefineClip()
{
  emit(defineRequested(popupClip));
}


void MatchView::popupMatchClip()
{
  emit(matchRequested(popupClip));
}


void MatchView::expandList()
{
  QListViewItem *top = w_list->firstChild();

  while (top) {
    w_list->setOpen(top,true);
    top = top->nextSibling();
  }
}


void MatchView::collapseList()
{
  w_list->setCurrentItem(w_list->firstChild());
  QListViewItem *top = w_list->firstChild();

  while (top) {
    w_list->setOpen(top,false);
    top = top->nextSibling();
  }
}

//--------------------------------

#include "matchview.moc"
