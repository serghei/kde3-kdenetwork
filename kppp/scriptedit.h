
/*
 *            kPPP: A front end for pppd for the KDE project
 *
 * $Id: scriptedit.h 212185 2003-03-07 22:11:39Z waba $
 *
 *             Copyright (C) 1997 Bernd Johannes Wuebben
 *                    wuebben@math.cornell.edu
 *
 * based on EzPPP:
 * Copyright (C) 1997  Jay Painter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _SCRIPTEDIT_H_
#define _SCRIPTEDIT_H_

#include <qwidget.h>

class QComboBox;
class QLineEdit;

class ScriptEdit : public QWidget {
Q_OBJECT
public:
  ScriptEdit( QWidget *parent=0, const char *name=0 );
  ~ScriptEdit() {}

  QString text()const;
  void setText(const QString &);
  int type()const;

  virtual void setEnabled(bool);

signals:
  void returnPressed();

private slots:
  void setType(int);
  void seReturnPressed();

private:
  QComboBox *st;
  QLineEdit *se;

public:
  enum setypes { Expect = 0,
		 Send = 1,
		 Pause = 2,
		 Hangup = 3,
		 Answer = 4,
		 Timeout = 5,
		 Password = 6,
		 ID = 7,
		 Prompt = 8,
		 PWPrompt = 9,
		 LoopStart = 10,
		 LoopEnd = 11,
		 Scan = 12,
                 Save = 13,
		 SendNoEcho = 14
  };
};

#endif

