/*    tcpnode.h
 *
 *    Copyright (c) 2000 Alexander Neundorf
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

#ifndef _TCPNODE_H_
#define _TCPNODE_H_

#include "mystring.h"

struct Node
{
   Node(const MyString& n, int ipAddress)
      :name(n),ip(ipAddress) {};
   MyString name;
   unsigned int ip;
};

#endif
