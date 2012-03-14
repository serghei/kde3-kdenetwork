/*    configfile.h
 *
 *    Copyright (c) 1998,2000 Alexander Neundorf
 *    neundorf@kde.org
 *
 *    You may distribute under the terms of the GNU General Public
 *    License as specified in the COPYING file.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 */

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <map>

#include "mystring.h"

class Config
{
   public:
      Config(const MyString& name);
      MyString getEntry(const char *key, const char* defaultValue);
      int getEntry(const char *key, int defaultValue);
   protected:
      std::map<MyString, MyString> m_entries;
};

#endif
