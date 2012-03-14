/*
 * portsettingsbar.cpp
 *
 * Copyright (c) 2000, 2005 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "portsettingsbar.h"
#include <klocale.h>

#include <qlabel.h>

PortSettingsBar::PortSettingsBar(const QString& title, QWidget *parent)
:QHBox(parent)
{
   QLabel* label=new QLabel(title, this);
   m_box=new QComboBox(this);
   label->setBuddy(m_box);

   m_box->insertItem(i18n("Check Availability"));
   m_box->insertItem(i18n("Always"));
   m_box->insertItem(i18n("Never"));

   connect(m_box,SIGNAL(activated(int)),this,SIGNAL(changed()));
}

int PortSettingsBar::selected() const
{
   return m_box->currentItem();
}

void PortSettingsBar::setChecked(int what)
{
   m_box->setCurrentItem(what);
}

#include "portsettingsbar.moc"

