/*
 * findnic.cpp
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

#include <config.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#ifdef USE_SOLARIS
/* net/if.h is incompatible with STL on Solaris 2.6 - 2.8, redefine
   map in the header file because we don't need it. -- Simon Josefsson */
#define map junkmap
#endif
#  include <net/if.h>
#ifdef USE_SOLARIS
#undef map
#endif

#include <sys/ioctl.h>

#ifndef	HAVE_STRUCT_SOCKADDR_SA_LEN
	#undef HAVE_GETNAMEINFO
	#undef HAVE_GETIFADDRS
#endif

#if defined(HAVE_GETNAMEINFO) && defined(HAVE_GETIFADDRS)
	#include <ifaddrs.h>
	#include <netdb.h>
	#include <qstring.h>

	QString flags_tos (unsigned int flags);
#endif

#include "findnic.h"

#include <klocale.h>
#include <ksockaddr.h>

NICList* findNICs()
{
   NICList* nl=new NICList;
   nl->setAutoDelete(true);

#if !defined(HAVE_GETIFADDRS) && !defined(HAVE_GETNAMEINFO)

   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   char buf[8*1024];
   struct ifconf ifc;
   ifc.ifc_len = sizeof(buf);
   ifc.ifc_req = (struct ifreq *) buf;
   int result=ioctl(sockfd, SIOCGIFCONF, &ifc);

   for (char* ptr = buf; ptr < buf + ifc.ifc_len; )
   {
      struct ifreq *ifr =(struct ifreq *) ptr;
      int len = sizeof(struct sockaddr);
#ifdef	HAVE_STRUCT_SOCKADDR_SA_LEN
      if (ifr->ifr_addr.sa_len > len)
         len = ifr->ifr_addr.sa_len;		/* length > 16 */
#endif
      ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

      int flags;
      struct sockaddr_in *sinptr;
      MyNIC *tmp=0;
      switch (ifr->ifr_addr.sa_family)
      {
      case AF_INET:
         sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
         flags=0;

         struct ifreq ifcopy;
         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFFLAGS,&ifcopy);
         flags=ifcopy.ifr_flags;

         tmp=new MyNIC;
         tmp->name=ifr->ifr_name;
         if ((flags & IFF_UP) == IFF_UP)
            tmp->state=i18n("Up");
         else
            tmp->state=i18n("Down");

         if ((flags & IFF_BROADCAST) == IFF_BROADCAST)
            tmp->type=i18n("Broadcast");
         else if ((flags & IFF_POINTOPOINT) == IFF_POINTOPOINT)
            tmp->type=i18n("Point to Point");
#ifndef _AIX
         else if ((flags & IFF_MULTICAST) == IFF_MULTICAST)
            tmp->type=i18n("Multicast");
#endif
         else if ((flags & IFF_LOOPBACK) == IFF_LOOPBACK)
            tmp->type=i18n("Loopback");
         else
            tmp->type=i18n("Unknown");

         tmp->addr=inet_ntoa(sinptr->sin_addr);

         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFNETMASK,&ifcopy);
         if (result==0)
         {
            sinptr = (struct sockaddr_in *) &ifcopy.ifr_addr;
            tmp->netmask=inet_ntoa(sinptr->sin_addr);
         }
         else
            tmp->netmask=i18n("Unknown");
         nl->append(tmp);
         break;

      default:
         break;
      }
   }
#else
  struct ifaddrs *ifap, *ifa;
  if (getifaddrs(&ifap) != 0) {
    return nl;
  }

  MyNIC *tmp=0;
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    switch (ifa->ifa_addr->sa_family) {
    case AF_INET6:
    case AF_INET: {
      tmp = new MyNIC;
      tmp->name = ifa->ifa_name;

      char buf[128];

      bzero(buf, 128);
      getnameinfo(ifa->ifa_addr, ifa->ifa_addr->sa_len, buf, 127, 0, 0, NI_NUMERICHOST);
      tmp->addr = buf;

      if (ifa->ifa_netmask != NULL) {
#ifdef Q_OS_FREEBSD
	struct sockaddr_in *sinptr = (struct sockaddr_in *)ifa->ifa_netmask;
	tmp->netmask=inet_ntoa(sinptr->sin_addr);
#else
	bzero(buf, 128);
	getnameinfo(ifa->ifa_netmask, ifa->ifa_netmask->sa_len, buf, 127, 0, 0, NI_NUMERICHOST);
	tmp->netmask = buf;
#endif
      }

      if (ifa->ifa_flags & IFF_UP)
	tmp->state=i18n("Up");
      else
	tmp->state=i18n("Down");

      tmp->type = flags_tos(ifa->ifa_flags);

      nl->append(tmp);
      break;
    }
    default:
      break;
    }
  }

  freeifaddrs(ifap);
#endif
   return nl;
}

void suggestSettingsForAddress(const QString& addrMask, LisaConfigInfo& lci)
{
   QString ip=addrMask.left(addrMask.find("/"));
   QString mask=addrMask.mid(addrMask.find("/")+1);

   if (mask[mask.length()-1]==';')
      mask=mask.left(mask.length()-1);
   MyNIC tmpNic;
   KInetSocketAddress::stringToAddr(AF_INET, mask.latin1(), &tmpNic.netmask);
   KInetSocketAddress::stringToAddr(AF_INET, ip.latin1(), &tmpNic.addr);
   suggestSettingsForNic(&tmpNic,lci);
}

void suggestSettingsForNic(MyNIC* nic, LisaConfigInfo& lci)
{
   lci.clear();
   if (nic==0)
      return;
   QString address = nic->addr;
   QString netmask = nic->netmask;

   QString addrMask(address+"/"+netmask+";");
   struct in_addr tmpaddr;
   inet_aton(nic->netmask.latin1(), &tmpaddr);
   unsigned int tmp= ntohl(tmpaddr.s_addr);

   //if the host part is less than 20 bits simply take it
   //this might be a problem on 64 bit machines
   if (tmp>0xfffff000)
   {
      lci.pingAddresses=addrMask;
      lci.broadcastNetwork=addrMask;
      lci.allowedAddresses=addrMask;
      lci.secondWait=0;
      lci.secondScan=false;
      lci.firstWait=30;
      lci.maxPingsAtOnce=256;
      lci.updatePeriod=300;
      lci.useNmblookup=false;
      lci.unnamedHosts=false;
   }
   else
   {
      lci.pingAddresses="";
      lci.broadcastNetwork=addrMask;
      lci.allowedAddresses=addrMask;
      lci.secondWait=0;
      lci.secondScan=false;
      lci.firstWait=30;
      lci.maxPingsAtOnce=256;
      lci.updatePeriod=300;
      lci.useNmblookup=true;
      lci.unnamedHosts=false;
   }
}

LisaConfigInfo::LisaConfigInfo()
{
   clear();
}

void LisaConfigInfo::clear()
{
   pingAddresses="";
   broadcastNetwork="";
   allowedAddresses="";
   secondWait=0;
   firstWait=0;
   secondScan=false;
   maxPingsAtOnce=256;
   updatePeriod=0;
   useNmblookup=false;
   unnamedHosts=false;
}

#if defined(HAVE_GETNAMEINFO) && defined(HAVE_GETIFADDRS)
QString flags_tos (unsigned int flags)
{
  QString tmp;
  if (flags & IFF_POINTOPOINT) {
    tmp +=  i18n("Point to Point");
  }

  if (flags & IFF_BROADCAST) {
    if (tmp.length()) {
      tmp += QString::fromLatin1(", ");
    }
    tmp += i18n("Broadcast");
  }
  
  if (flags & IFF_MULTICAST) {
    if (tmp.length()) {
      tmp += QString::fromLatin1(", ");
    }
    tmp += i18n("Multicast");
  }
  
  if (flags & IFF_LOOPBACK) {
    if (tmp.length()) {
      tmp += QString::fromLatin1(", ");
    }
    tmp += i18n("Loopback");
  }
  return tmp;
}
#endif
