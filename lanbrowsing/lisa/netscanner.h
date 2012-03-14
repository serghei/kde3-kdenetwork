/*    netscanner.h
 *
 *    Copyright (c) 1998, 1999, 2000 Alexander Neundorf
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

#ifndef NETSCANNER_H
#define NETSCANNER_H

#include <list>

#include "mystring.h"
#include "addressvalidator.h"
#include "tcpnode.h"
#include "configfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include <stdio.h>

class NetScanner
{
   public:
      NetScanner(int& rawSocket, int strictMode);
      ~NetScanner();

      void doScan();

      void completeNames(std::list<Node>* nodes);
      
      //using nmblookup to determine the ip-address
      //MyString ip2NetbiosName(const MyString& target, MyString& groupName);
      
      char* ip2Name(struct in_addr ip);
      
      MyString procId;
      int m_firstWait;
      int m_secondWait;
      void configure(Config& config);
   protected:
      int m_strictMode;
      int& m_rawSocketFD;
      AddressValidator validator;
      MyString ipRangeStr;
      MyString nameMask;

      int m_maxPings;
      int m_deliverUnnamedHosts;
      int m_useNmblookup;
      //return ip-address with number index from a virtual array
      //with all ip-addresses according to ipRangeStr
      struct in_addr getIPfromArray(unsigned int index);

      std::list<Node>* hostList;

      //needed for getIPfromArray()
      //contains the part of ipRangeStr, which is not yet parsed
      MyString tmpIPRange;
      //has to be called before every first call of getNextIPRange
      void resetIPRange();
      //returns the next range/part from tmpIPRange before the next semicolon
      MyString getNextIPRange();

      void pingScan(std::list<Node>* newList);   //the ping-version
      void nmblookupScan(std::list<Node>* newList); //the nmblookup "*"-version
      int hostAlreadyInList(int ip, std::list<Node>* nodes);
      void removeUnnamedHosts(std::list<Node>* nodes);

      struct is_unnamed_node : std::unary_function<Node&, bool>
      {
         bool operator() (Node& n)
         {
            struct in_addr tmpAddr;
            tmpAddr.s_addr = n.ip;
            return (strcmp(inet_ntoa(tmpAddr), n.name.data()) == 0);
         }
      };
};

#endif
