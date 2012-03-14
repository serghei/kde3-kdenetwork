/*
  main.cpp - The KControl module for ktalkd

  Copyright (C) 1998 by David Faure, faure@kde.org
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   
  */

#ifndef __kcmktalkd_main_h
#define __kcmktalkd_main_h

#include <kcmodule.h>

class QTabWidget;

class KSimpleConfig;

class KSoundPageConfig;
class KAnswmachPageConfig;
class KForwmachPageConfig;

class KTalkdConfigModule : public KCModule
{
	Q_OBJECT

public:

    KTalkdConfigModule(QWidget *parent, const char *name);
    virtual ~KTalkdConfigModule();
    
    //void init();
    void load();
    void save();
    void defaults();
    
protected:
	void resizeEvent(QResizeEvent *);

private:
	KSimpleConfig *config;
	KSimpleConfig *announceconfig;

	QTabWidget *tab;
	
    KSoundPageConfig *soundpage;
    KAnswmachPageConfig *answmachpage;
    KForwmachPageConfig *forwmachpage;
};

#endif

