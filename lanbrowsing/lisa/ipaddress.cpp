/*    ipaddress.cpp
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

#include "ipaddress.h"

#include <iostream>

IPAddress::IPAddress()
{
}

IPAddress::IPAddress(const MyString& ip)
{
   if (ip.length()==0)
      s="0.0.0.0";
   else s=ip;
   i=string2Int(s);
   n=string2Struct(s);
}

IPAddress::IPAddress(unsigned int ip)
{
   i=ip;
   s=int2String(i);
   n.s_addr=htonl(i);
   //n=string2Struct(s);
}

IPAddress::IPAddress(struct in_addr ip)
{
   n=ip;
   s=struct2String(n);
   i=string2Int(s);
}

IPAddress::IPAddress(int b1, int b2, int b3, int b4)
{
   char tmp[30];
   sprintf(tmp,"%i.%i.%i.%i",b1,b2,b3,b4);
   s=tmp;
   i=string2Int(s);
   n=string2Struct(s);
}


MyString IPAddress::int2String(unsigned int ip)
{
   MyString tmp("");
   struct in_addr addr;
   addr.s_addr=htonl(ip);
   tmp=inet_ntoa(addr);
   return tmp;
}

MyString IPAddress::struct2String(struct in_addr ip)
{
   return MyString(inet_ntoa(ip));
}

unsigned int IPAddress::string2Int(MyString ip)
{
//   struct in_addr addr;
//   inet_aton(ip.c_str(),&addr);
//   cout<<addr.s_addr<<endl;
//   return ntohl(addr.s_addr);
   return ntohl(inet_addr(ip.c_str()));
}

struct in_addr IPAddress::string2Struct(MyString ip)
{
   struct in_addr tmp;
//   inet_aton(ip.c_str(),&tmp);
   tmp.s_addr = inet_addr(ip.c_str());
   return tmp;
}

void IPAddress::print()
{
   std::cout<<"address as string: "<<s<<std::endl;
   std::cout<<"address in host byte order:    "<<std::ios::hex<<i<<std::ios::dec<<std::endl;
   std::cout<<"address in network byte order: "<<std::ios::hex<<n.s_addr<<std::ios::dec<<std::endl;
}


