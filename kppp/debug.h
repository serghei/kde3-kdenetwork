/*
 *        kPPP: A pppd front end for the KDE project
 *
 * $Id: debug.h 212185 2003-03-07 22:11:39Z waba $
 *            Copyright (C) 1997  Bernd Wuebben
 *                 wuebben@math.cornel.edu
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _DEBUGWIDGET_
#define _DEBUGWIDGET_

#include <qdialog.h>
#include <qstring.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>


class myMultiEdit : public QMultiLineEdit {
public:

  myMultiEdit(QWidget *parent=0, const char *name=0);

  void newLine();
  void insertChar(unsigned char c);
};


class DebugWidget : public QDialog {
  Q_OBJECT
public:
  DebugWidget(QWidget *parent=0, const char *name=0);

  void clear();

public slots:
  void statusLabel(const QString &);
  void toggleVisibility();
  void addChar(unsigned char);

protected:
  virtual void hideEvent(QHideEvent *);
  virtual void resizeEvent(QResizeEvent *e);

private:
  void enter();
  QFrame *fline;
  QPushButton *dismiss;
  myMultiEdit *text_window;

  QLabel *statuslabel;
};

#endif

