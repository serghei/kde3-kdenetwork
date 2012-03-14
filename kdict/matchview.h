/* -------------------------------------------------------------

   matchview.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   MatchView  This widget contains the list of matching definitions

 ------------------------------------------------------------- */


#ifndef _MATCHVIEW_H_
#define _MATCHVIEW_H_

#include <qlistview.h>
class KPopupMenu;


//*********  MatchViewItem  ********************************************

class MatchViewItem : public QListViewItem
{
  
public:
                
  MatchViewItem(QListView *view,const QString &text);
  MatchViewItem(QListView *view,QListViewItem *after,const QString &text);
  MatchViewItem(QListViewItem *item,const QString &text,const QString &commandStr);
  MatchViewItem(QListViewItem *item,QListViewItem *after,const QString &text,const QString &commandStr);
  ~MatchViewItem();

  void setOpen(bool o);
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

  QString command;
  QStringList subEntrys;
};


//*********  MatchView  ******************************************


class MatchView : public QWidget
{
  Q_OBJECT

public:

  MatchView(QWidget *parent=0,const char *name=0);
  ~MatchView();

  void updateStrategyCombo();
  bool selectStrategy(const QString &strategy) const;
  void match(const QString &query);

signals:

  void defineRequested(const QString &query);
  void matchRequested(const QString &query);
  void clipboardRequested();
  void windowClosed();

protected:

  void closeEvent ( QCloseEvent * e );

private slots:

  void strategySelected(int num);
  void enableGetButton();
  void mouseButtonPressed(int, QListViewItem *, const QPoint &, int);
  void returnPressed(QListViewItem *i);
  void getOneItem(QListViewItem *i);
  void getSelected();
  void getAll();
  void doGet(QStringList &defines);
  void newList(const QStringList &matches);
  void buildPopupMenu(QListViewItem *, const QPoint &, int);
  void popupGetCurrent();
  void popupDefineCurrent();
  void popupMatchCurrent();
  void popupDefineClip();
  void popupMatchClip();
  void expandList();
  void collapseList();

private:

  QComboBox *w_strat;
  QListView *w_list;
  QPushButton *w_get,*w_getAll;

  bool getOn, getAllOn;

  KPopupMenu *rightBtnMenu;
  MatchViewItem *popupCurrent;
  QString popupClip;   // needed for rightbtn-popup menu
};

#endif
