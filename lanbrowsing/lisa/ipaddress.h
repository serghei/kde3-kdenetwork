/*    ipaddress.h
 *
 *    Copyright (c) 1998, 1999, Alexander Neundorf, Lukas Pustina
 *    alexander.neundorf@rz.tu-ilmenau.de
 *    lukas@tronet.de
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

#ifndef IPADDRESS_H
#define IPADDRESS_H

#include "lisadefines.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "mystring.h"

class IPAddress
{
   public:
      IPAddress();
      IPAddress(const MyString& ip);
      IPAddress(unsigned int ip);
      IPAddress(struct in_addr ip);
      IPAddress(int b1, int b2, int b3, int b4);
      //you know
      MyString asString()         {return s;};
      //in host byte order
      unsigned int asInt()      {return i;};
      //in network byte order
      struct in_addr asStruct() {return n;};
      /*operator= (IPAddress ip);
      operator+= (unsigned int add);*/
      void print();
   private:
      MyString int2String(unsigned int ip);
      MyString struct2String(struct in_addr ip);
      unsigned int string2Int(MyString ip);
      struct in_addr string2Struct(MyString ip);
      MyString s;
      unsigned int i;
      struct in_addr n;
};

#endif
