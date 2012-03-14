/*
 * findnic.h
 *
 * Copyright (c) 2001 Alexander Neundorf <neundorf@kde.org>
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

#ifndef FINDNIC_H
#define FINDNIC_H

#include <qptrlist.h>
#include <qstring.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct MyNIC
{
   QString name;
   QString addr;
   QString netmask;
   QString state;
   QString type;
};

struct LisaConfigInfo
{
   LisaConfigInfo();
   void clear();
   QString pingAddresses;
   QString broadcastNetwork;
   QString allowedAddresses;
   int secondWait;
   bool secondScan;
   int firstWait;
   int maxPingsAtOnce;
   int updatePeriod;
   bool useNmblookup;
   bool unnamedHosts;
};


typedef QPtrList<MyNIC> NICList;

//some plain C-like helper functions

///Return a list with all NIC, which are up and broadcast-able
NICList* findNICs();
///Enter some settings into lci which might be appropriate for nic
void suggestSettingsForNic(MyNIC* nic, LisaConfigInfo& lci);

void suggestSettingsForAddress(const QString& addrMask, LisaConfigInfo& lci);

#endif

