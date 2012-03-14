/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_RMBMENU_H
#define PAGE_RMBMENU_H

#include "page_rmbmenubase.h"
#include "ksopts.h"

class PageRMBMenu : public PageRMBMenuBase
{
Q_OBJECT

public:
    PageRMBMenu( QWidget *parent = 0, const char *name = 0 );
    ~PageRMBMenu();

  void saveConfig();
  void defaultConfig();
  void readConfig( const KSORMBMenu * = ksopts );
protected slots:
  void highlighted(int);
  void moveUp();
  void moveDown();
  void insSeperator();
  void insCommand();
  void delCommand();
signals:
  void modified();
};

#endif
