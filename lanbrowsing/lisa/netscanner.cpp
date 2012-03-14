/*    netscanner.cpp
 *
 *    Copyright (c) 2000, Alexander Neundorf,
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

#include "config.h"
#include "netscanner.h"
#include "ipaddress.h"
#include "lisadefines.h"
#include <iostream>

#ifdef LISA_DEBUG
#undef LISA_DEBUG
#endif
#define LISA_DEBUG 0

#ifdef dcerr
#undef dcerr
#endif

#ifdef mdcerr
#undef mdcerr
#endif

#define dcerr if (LISA_DEBUG==1) std::cerr<<"NetScanner::"
#define mdcerr if (LISA_DEBUG==1) std::cerr<<procId<<" NetScanner::"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef __osf__
#undef BYTE_ORDER
#define _OSF_SOURCE
#undef _MACHINE_ENDIAN_H_
#undef __STDC__
#define __STDC__ 0
#include <netinet/ip.h>
#undef __STDC__
#define __STDC__ 1
#endif
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

struct ICMPEchoRequest
{
   unsigned char type;
   unsigned char code;
   unsigned short int checkSum;
   unsigned short id;
   unsigned short seqNumber;
};

unsigned short in_cksum(unsigned short *addr, int len)
{
   int nleft = len;
	int sum(0);
	unsigned short	*w = addr;
	unsigned short	answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
   while (nleft > 1)
   {
      sum += *w++;
      nleft -= 2;
	}

   /* 4mop up an odd byte, if necessary */
   if (nleft == 1)
   {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
      sum += answer;
   }

   /* 4add back carry outs from top 16 bits to low 16 bits */
   sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
   sum += (sum >> 16);			/* add carry */
   answer = ~sum;				/* truncate to 16 bits */
   return(answer);
}


NetScanner::NetScanner(int& rawSocketFD, int strictMode)
:procId("")
,m_firstWait(5)
,m_secondWait(15)
,m_strictMode(strictMode)
,m_rawSocketFD(rawSocketFD)
,validator()
,ipRangeStr(";")
,m_maxPings(256)
,m_deliverUnnamedHosts(0)
,m_useNmblookup(0)
,hostList(0)
,tmpIPRange("")
{}

NetScanner::~NetScanner()
{
//   std::cerr<<"----------- Netscanner dtor "<<std::endl;
   delete hostList;
   ::close(m_rawSocketFD);
}

void addMissingSemicolon(MyString& text)
{
   if (text.isEmpty()) return;
   if (text[text.length()-1]!=';')
      text+=';';
}

void NetScanner::configure(Config& config)
{
   //ranges are not allowed in strict mode
   if (!m_strictMode)
   {
      ipRangeStr=stripWhiteSpace(config.getEntry("PingAddresses",""));
      addMissingSemicolon(ipRangeStr);
   }
   MyString pingNames=stripWhiteSpace(config.getEntry("PingNames",""));
   addMissingSemicolon(pingNames);
   MyString nextName;
   int semicolonPos=pingNames.find(';');
   int hostsAdded(0);
   while (semicolonPos!=-1)
   {
      nextName=pingNames.left(semicolonPos);
      mdcerr<<"configure(): looking up -"<<nextName<<"-"<<std::endl;
      //now the name lookup
      in_addr server_addr;
      hostent *hp=gethostbyname(nextName.data());
      if (hp!=0)
      {
         if ((m_strictMode) && (hostsAdded>=STRICTMODEMAXHOSTS))
            break;
         memcpy(&server_addr, hp->h_addr, sizeof(server_addr));
         char *ip=inet_ntoa(server_addr);
         mdcerr<<"configure(): looking up "<<nextName<<" gives -"<<ip<<"-"<<std::endl;
         ipRangeStr=ipRangeStr+ip+';';
         hostsAdded++;
      }

      pingNames=pingNames.mid(semicolonPos+1);
      semicolonPos=pingNames.find(';');
   }
   if ((!ipRangeStr.isEmpty()) && (ipRangeStr[0]==';'))
      ipRangeStr=ipRangeStr.mid(1);
   m_deliverUnnamedHosts=config.getEntry("DeliverUnnamedHosts",0);
   m_useNmblookup=config.getEntry("SearchUsingNmblookup",0);
   m_maxPings=config.getEntry("MaxPingsAtOnce",256);
   m_firstWait=config.getEntry("FirstWait",5);
   m_secondWait=config.getEntry("SecondWait",15);
   if (m_firstWait<1) m_firstWait=1;
   if (m_maxPings<8) m_maxPings=8;
   if (m_maxPings>1024) m_maxPings=1024;
   //on some systems (Solaris ?) select() doesn't work correctly
   // if the microseconds are more than 1.000.000
   if (m_firstWait>99) m_firstWait=99;
   if (m_secondWait>99) m_secondWait=99;
   mdcerr<<"configure(): "<<ipRangeStr<<std::endl;
}

struct in_addr NetScanner::getIPfromArray(unsigned int index)
{
   //mdcerr<<std::endl<<"*** start ***"<<std::endl;
   unsigned int tmpIndex(0),indexLeft(index);
   resetIPRange();
   MyString tmp(getNextIPRange());
//   mdcerr<<"NetScanner::getIPFromArray: -"<<tmp<<"-"<<std::endl;
   while (!tmp.isEmpty())
   {
      if (tmp.contains('/'))
      {
         //mdcerr<<"net/mask combination detected"<<std::endl;
         MyString netStr(tmp.left(tmp.find("/")));
         MyString maskStr(tmp.mid(tmp.find("/")+1));
         unsigned int mask(IPAddress(maskStr).asInt());
         unsigned int net(IPAddress(netStr).asInt()&mask);
         if ((~mask)<indexLeft)
         {
            indexLeft=indexLeft-(~mask+1);
            tmpIndex+=(~mask)+1;
            //mdcerr<<"i: "<<tmpIndex<<" left: "<<indexLeft<<std::endl;
         }
         else
         {
            net+=indexLeft;
            return IPAddress(net).asStruct();
            //return string2Struct(ipInt2String(net));
         }
      }
      else if (tmp.contains('-')==1)
      {
         //mdcerr<<"single range detected"<<std::endl;
         MyString fromIPStr(tmp.left(tmp.find("-")));
         MyString toIPStr(tmp.mid(tmp.find("-")+1));
         //mdcerr<<"fromIPStr: "<<fromIPStr<<std::endl;
         //mdcerr<<"toIPStr: "<<toIPStr<<std::endl;
         unsigned int fromIP(IPAddress(fromIPStr).asInt());
         unsigned int toIP(IPAddress(toIPStr).asInt());
         //unsigned int fromIP(ipString2Int(fromIPStr)), toIP(ipString2Int(toIPStr));
         //index hinter diesem bereich
         if ((fromIP+indexLeft)>toIP)
         {
            tmpIndex+=1+toIP-fromIP;
            indexLeft=indexLeft-(1+toIP-fromIP);
            //mdcerr<<"i: "<<tmpIndex<<" left: "<<indexLeft<<std::endl;
         }
         //index in diesem bereich
         else
         {
            fromIP+=indexLeft;
            return IPAddress(fromIP).asStruct();
            //return string2Struct(ipInt2String(fromIP));
         }
         
      }
      else if (tmp.contains('-')==4)
      {
         //mdcerr<<"multiple range detected"<<std::endl;
         int cp(tmp.find('-'));
         int from1(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to1(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from2(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to2(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from3(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to3(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from4(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         int to4(atoi(tmp.data()));
         
         unsigned int count((1+to4-from4)*(1+to3-from3)*(1+to2-from2)*(1+to1-from1));
         if (count<indexLeft)
         {
            tmpIndex+=count;
            indexLeft-=count;
            //mdcerr<<"i: "<<tmpIndex<<" left: "<<indexLeft<<std::endl;
         }
         else
         {
            for (int b1=from1; b1<=to1; b1++)
               for (int b2=from2; b2<=to2; b2++)
                  for (int b3=from3; b3<=to3; b3++)
                     for (int b4=from4; b4<=to4; b4++)
                     {
                        if (tmpIndex==index)
                        {
                           return IPAddress(b1,b2,b3,b4).asStruct();
                        };
                        tmpIndex++;
                        indexLeft--;
                        //mdcerr<<"i: "<<tmpIndex<<" left:"<<indexLeft<<std::endl;
                     }
         }
      }
      //single IP address
      else if (tmp.contains('.')==3)
      {
         //mdcerr<<"single IP address detected"<<std::endl;
         //if (tmpIndex==index) return string2Struct(tmp);
         if (tmpIndex==index) return IPAddress(tmp).asStruct();
         else
         {
            tmpIndex++;
            indexLeft--;
            //mdcerr<<"i: "<<tmpIndex<<" left: "<<indexLeft<<std::endl;
         }
      }
      //mdcerr<<"nextIPRange: *"<<tmp<<"*"<<std::endl;
      tmp=getNextIPRange();
   }
   return IPAddress("0.0.0.0").asStruct();
}

void NetScanner::resetIPRange()
{
   tmpIPRange=ipRangeStr;
}

MyString NetScanner::getNextIPRange()
{
   if (tmpIPRange.contains(';')<1) return "";
   int cp(tmpIPRange.find(';'));
   MyString tmp(tmpIPRange.left(cp));
   tmpIPRange=tmpIPRange.mid(cp+1);
   return tmp;
}

char* NetScanner::ip2Name(struct in_addr ip)
{
   struct hostent *hostname=0;
   // Set the hostname of node
   if ( ( hostname = gethostbyaddr( (char *) &ip.s_addr, 4, AF_INET  ) ) == 0 )
   {
      mdcerr << "ip2Name gethostbyname* error" << std::endl;
      return inet_ntoa( ip );
   }
   mdcerr<<"ip2name -"<<hostname->h_name<<std::endl;
   return hostname->h_name;
}

void NetScanner::nmblookupScan(std::list<Node>* newList)
{
   mdcerr<<"nmblookupScan()"<<std::endl;
   //newList->clear();
   FILE * nmblookupFile=popen("nmblookup \"*\"","r");
   //no success
   if (nmblookupFile==0)
   {
      mdcerr<<"nmblookupScan(): could not start nmblookup"<<std::endl;
      return;
   };
   MyString dummy("");
   char *receiveBuffer=0;
   int bufferSize(0);
   int nmblookupFd=fileno(nmblookupFile);
   struct timeval tv;
   fd_set fds;
   int done(0);
   int timeOuts(0);
   char *tmpBuf=new char[16*1024];
   do
   {
      FD_ZERO(&fds);
      FD_SET(nmblookupFd,&fds);
      tv.tv_sec=10;
      tv.tv_usec=0;
      int result=select(nmblookupFd+1,&fds,0,0,&tv);
      //error
      if (result<0)
         done=1;
      //timeout
      else if (result==0)
      {
         timeOuts++;
         //3 time outs make 30 seconds, this should be *much* more than enough
         if (timeOuts>=3)
            done=1;
      }
      else if (result>0)
      {
         //read stuff
         int bytesRead=::read(nmblookupFd,tmpBuf,16*1024);
         //pipe closed
         if (bytesRead==0)
            done=1;
         else
         {
            char *newBuf=new char[bufferSize+bytesRead];
            if (receiveBuffer!=0)
            {
               memcpy(newBuf,receiveBuffer,bufferSize);
               delete [] receiveBuffer;
            }
            memcpy(newBuf+bufferSize,tmpBuf,bytesRead);
            receiveBuffer=newBuf;
            bufferSize+=bytesRead;
         }
      }
   } while (!done);

   // Warning: The return value of pclose may be incorrect due to the
   // SIGCHLD handler that is installed. Ignore it!
   pclose(nmblookupFile);

   delete [] tmpBuf;
   //we received nothing
   if (receiveBuffer==0)
      return;

   //check for a terrminating '\0'
   if (receiveBuffer[bufferSize-1]=='\0')
      receiveBuffer[bufferSize-1]='\0';

   //std::cerr<<receiveBuffer<<std::endl;

   tmpBuf=receiveBuffer;
   int bytesLeft=bufferSize;

   int bufferState=0;
   while (bytesLeft>0)
   {
      //mdcerr<<"bytesLeft: "<<bytesLeft<<" received: "<<bufferSize<<std::endl;
      //since we added a terminating \0 we can be sure here
      char *endOfLine=(char*)memchr(tmpBuf,'\n',bytesLeft);
      //point to the last character
      if (endOfLine==0)
         endOfLine=receiveBuffer+bufferSize-1;

      //0-terminate the string
      *endOfLine='\0';
      //now tmpBuf to endOfLine is a 0-terminated string
      int strLength=strlen(tmpBuf);
      //hmm, if this happens, there must be something really wrong
      if (strLength>1000)
         break;

      bytesLeft=bytesLeft-strLength-1;

      if (bufferState==0)
      {
         if (isdigit(tmpBuf[0]))
            bufferState=1;
      }
      //yes, no else !
      if (bufferState==1)
      {
         char tmpIP[1024];
         //std::cerr<<"tmpBuf: -"<<tmpBuf<<"-"<<std::endl;
         if (sscanf(tmpBuf,"%s *<00>\n",tmpIP) == 1) {
             mdcerr<<"nmblookupScan: tmpIP: -"<<tmpIP<<"-"<<std::endl;
             struct sockaddr_in addr;
             if ((addr.sin_addr.s_addr = inet_addr(tmpIP)) != INADDR_NONE)
                 if  (!hostAlreadyInList(addr.sin_addr.s_addr,newList))
                 {
                     if (validator.isValid(addr.sin_addr.s_addr))
                     {
                         mdcerr<<"nmblookupScan: adding "<<inet_ntoa(addr.sin_addr)<<std::endl;
                         newList->push_back(Node(dummy,addr.sin_addr.s_addr));
                     }
                 }
             }
      }
      tmpBuf=endOfLine+1;
   };
   mdcerr<<"nmblookupScan: finished"<<std::endl;
   delete [] receiveBuffer;
}

void NetScanner::pingScan(std::list<Node>* newList)   //the ping-version
{
   mdcerr<<"pingScan()"<<std::endl;
   //newList->clear();
   MyString dummy("");
   mdcerr<<"pingScan: m_maxPings: "<<m_maxPings<<std::endl;

   pid_t pid=getpid();
   ICMPEchoRequest echo;
   echo.type=ICMP_ECHO;
   echo.code=0;
   echo.id=pid;
   echo.seqNumber=0;
   echo.checkSum=0;
   echo.checkSum=in_cksum((unsigned short *)&echo,8);

   char receiveBuf[16*1024];
   //before we start first read everything what might be in the receive buffer
   //of the raw socket

   timeval tv1;
   //wait a moment for answers
   tv1.tv_sec = 0;
   tv1.tv_usec = 0;
   fd_set clearSet;
   FD_ZERO(&clearSet);
   FD_SET(m_rawSocketFD,&clearSet);
   while(select(m_rawSocketFD,&clearSet,0,0,&tv1)>0)
   {
      ::recvfrom(m_rawSocketFD,(char*)&receiveBuf,16*1024,0,0,0);
      tv1.tv_sec = 0;
      tv1.tv_usec = 0;
      FD_ZERO(&clearSet);
      FD_SET(m_rawSocketFD,&clearSet);
   }
   //now the buffer should be empty
   
   //wait a moment for answers
   tv1.tv_sec = 0;
   tv1.tv_usec = m_firstWait*10*1000;//0.5 sec

   int loopCount(2);
   if (m_secondWait<0)
      loopCount=1;
   for (int repeatOnce=0; repeatOnce<loopCount; repeatOnce++)
   {
      mdcerr<<"******************** starting loop *****************"<<std::endl;
      unsigned int current(0);
      int finished(0);
      while (!finished)
      {
         for (int con=0; con<m_maxPings; con++)
         {
            struct in_addr tmpIP;
            do
            {
               tmpIP=getIPfromArray(current);
               current++;
//               mdcerr<<"pingScan(): trying "<<inet_ntoa(tmpIP)<<std::endl;
               if (hostAlreadyInList(tmpIP.s_addr,newList))
                  mdcerr<<"already in list :-)"<<std::endl;
               if (!validator.isValid(tmpIP.s_addr))
                  mdcerr<<"pingScan(): invalid IP :-("<<std::endl;
               //ping only hosts which are allowed to receive the results
               //and which are not in the list
            } while ( (tmpIP.s_addr!=0)
                      && ((!validator.isValid(tmpIP.s_addr))
                      || (hostAlreadyInList(tmpIP.s_addr,newList))));

            finished=(tmpIP.s_addr==0);
            if (!finished)
            {
               //send the icmp echo request
               struct sockaddr_in toAddr;
               toAddr.sin_family = AF_INET;
               toAddr.sin_addr = tmpIP;
               toAddr.sin_port = 0;
               (void)sendto(m_rawSocketFD,(char*)&echo,sizeof(echo),0,(sockaddr*)&toAddr,sizeof(toAddr));
               //int sb=sendto(sockFD,(char*)&echo,sizeof(echo),0,(sockaddr*)&toAddr,sizeof(toAddr));
//               mdcerr<<"pingScan: pinging "<<inet_ntoa(toAddr.sin_addr)<<std::endl;
            }
            else break;
         }
         select(0,0,0,0,&tv1);
         //now read the answers, hopefully
         struct sockaddr_in fromAddr;
         socklen_t length(sizeof(fromAddr));
         int received(0);

         fd_set sockFDset;
         FD_ZERO(&sockFDset);
         FD_SET(m_rawSocketFD,&sockFDset);
         tv1.tv_sec=0;
         tv1.tv_usec=0;
         while(select(m_rawSocketFD+1,&sockFDset,0,0,&tv1)>0)
         {
            received=recvfrom(m_rawSocketFD, (char*)&receiveBuf, 16*1024, 0,
			      (sockaddr*)&fromAddr, &length);
            if (received!=-1)
            {
//               mdcerr<<"pingScan: received from "<<inet_ntoa(fromAddr.sin_addr)<<" "<<received<<" b, ";
               struct ip *ipFrame=(ip*)&receiveBuf;
               int icmpOffset=(ipFrame->ip_hl)*4;
               icmp *recIcmpFrame=(icmp*)(receiveBuf+icmpOffset);
               int iType=recIcmpFrame->icmp_type;
               //if its a ICMP echo reply
               if ((iType==ICMP_ECHOREPLY)
                   //to an echo request we sent
                   && (recIcmpFrame->icmp_id==pid)
                   //and the host is not yet in the list
                   && (!hostAlreadyInList(fromAddr.sin_addr.s_addr,newList)))
               {
                  //this is an answer to our request :-)
//                  mdcerr<<"pingScan: adding "<<inet_ntoa(fromAddr.sin_addr)<<std::endl;
                  newList->push_back(Node(dummy,fromAddr.sin_addr.s_addr));
               }
            }
            tv1.tv_sec=0;
            tv1.tv_usec=0;
            FD_ZERO(&sockFDset);
            FD_SET(m_rawSocketFD,&sockFDset);
            //FD_SET(sockFD,&sockFDset);
         }
      }
      tv1.tv_sec = 0;
      tv1.tv_usec = m_secondWait*10*1000;//0.5 sec
   }
   mdcerr<<"pingScan() ends"<<std::endl;
}

void NetScanner::doScan()
{
   mdcerr<<"doScan"<<std::endl;
   //child
   std::list<Node>* tmpPingList=new std::list<Node>;
   mdcerr<<"doScan: created list"<<std::endl;
   if (m_useNmblookup)
      nmblookupScan(tmpPingList);
   pingScan(tmpPingList);
   // get the names from cache or lookup
   completeNames(tmpPingList);
   mdcerr<<"doScan: completed names"<<std::endl;
   if (m_deliverUnnamedHosts==0)
      removeUnnamedHosts(tmpPingList);

   mdcerr<<"doScan: added hosts"<<std::endl;
   
   delete hostList;
   hostList=tmpPingList;
}

int NetScanner::hostAlreadyInList(int ip, std::list<Node>* nodes)
{
   for (std::list<Node>::iterator node = nodes->begin(); node != nodes->end(); node++)
   {
      if (node->ip==ip)
         return 1;
   }
   return 0;
}

void NetScanner::removeUnnamedHosts(std::list<Node>* nodes)
{
   nodes->remove_if(is_unnamed_node());
}

void NetScanner::completeNames(std::list<Node>* nodes)
{
   struct sockaddr_in tmpAddr;
   //for every host
   for (std::list<Node>::iterator node = nodes->begin(); node != nodes->end(); node++)
   {
      tmpAddr.sin_addr.s_addr=node->ip;
      mdcerr<<"completeNames: looking up "<<inet_ntoa(tmpAddr.sin_addr)<<std::endl;
      int done(0);
      //first look wether we have the name already
      if (hostList!=0) for (std::list<Node>::iterator oldNode=hostList->begin(); oldNode!=hostList->end(); oldNode++)
      {
         if (node->ip==oldNode->ip)
         {
            mdcerr<<"completeNames: cached: "<<oldNode->name<<" :-)"<<std::endl;
            node->name=oldNode->name;
            done=1;
            break;
         }
      }
      //otherwise do a name lookup
      if (!done)
      {
         //IPAddress tmpAddress(node->ip);
         //mdcerr<<"NetScanner::completeNames: doing actual lookup"<<std::endl;
         node->name=ip2Name(tmpAddr.sin_addr);
         mdcerr<<"completeNames: resolved: "<<node->name<<std::endl;
      }
   }
}

