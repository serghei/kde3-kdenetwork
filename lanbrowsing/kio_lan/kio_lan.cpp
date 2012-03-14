/*  This file is part of the KDE project
    Copyright (C) 2000,2001 Alexander Neundorf <neundorf@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kprocess.h>

#include <qfile.h>

#include <iostream>
#include <string.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

#include "kio_lan.h"

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#define PORTSETTINGS_CHECK 0
#define PORTSETTINGS_PROVIDE 1
#define PORTSETTINGS_DISABLE 2

using namespace KIO;

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

extern "C" { KDE_EXPORT int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_lan" );

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_lan protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }
  int isLanIoslave=(strcmp("lan",argv[1])==0);

  // Trigger creation to make sure we pick up KIOSK settings correctly.
  (void)KGlobal::dirs();
  (void)KGlobal::locale();
  (void)KGlobal::config();

  kdDebug(7101) << "LAN: kdemain: starting" << endl;

  LANProtocol slave(isLanIoslave, argv[2], argv[3]);
  slave.dispatchLoop();
  return 0;
}

LANProtocol::LANProtocol(int isLanIoslave, const QCString &pool, const QCString &app )
:TCPSlaveBase(7741,isLanIoslave?"lan":"rlan", pool, app)
,m_currentHost("")
,m_port(7741)
,m_maxAge(15*60)
,m_isLanIoslave(isLanIoslave?true:false)
{
   KConfig *config=KGlobal::config();

   m_protocolInfo[KIOLAN_FTP].enabled=config->readNumEntry("Support_FTP",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_HTTP].enabled=config->readNumEntry("Support_HTTP",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_NFS].enabled=config->readNumEntry("Support_NFS",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_SMB].enabled=config->readNumEntry("Support_SMB",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_FISH].enabled=config->readNumEntry("Support_FISH",PORTSETTINGS_CHECK);

   m_defaultLisaHost=config->readEntry("DefaultLisaHost", "localhost");
   m_shortHostnames=config->readBoolEntry("ShowShortHostnames",false);
   m_maxAge=config->readNumEntry("MaxAge",15)*60;
   if (m_maxAge<0) m_maxAge=0;

   strcpy(m_protocolInfo[KIOLAN_NFS].name,"NFS");
   strcpy(m_protocolInfo[KIOLAN_FTP].name,"FTP");
   strcpy(m_protocolInfo[KIOLAN_SMB].name,"SMB");
   strcpy(m_protocolInfo[KIOLAN_HTTP].name,"HTTP");
   strcpy(m_protocolInfo[KIOLAN_FISH].name,"FISH");

   // Now we check for port 445 for SMB/CIFS also.  But we call both entries
   // SMB.  Clients will see only one SMB folder, though, whichever
   // port (or both) is detected.  The smb ioslave should be able
   // to figure out which port to actually use.

   m_protocolInfo[KIOLAN_NFS].ports.push_back(2049);
   m_protocolInfo[KIOLAN_FTP].ports.push_back(21);
   m_protocolInfo[KIOLAN_SMB].ports.push_back(445);
   m_protocolInfo[KIOLAN_SMB].ports.push_back(139);
   m_protocolInfo[KIOLAN_HTTP].ports.push_back(80);
   m_protocolInfo[KIOLAN_FISH].ports.push_back(22);

   m_hostInfoCache.setAutoDelete(true);
}

LANProtocol::~LANProtocol()
{
   m_hostInfoCache.clear();
}

int LANProtocol::readDataFromServer()
{
   if (m_isLanIoslave)
      return lanReadDataFromServer();
   else
      return rlanReadDataFromServer();
   return 0;
}

int LANProtocol::lanReadDataFromServer()
{
   kdDebug(7101)<<"LANProtocol::lanReadDataFromServer() host: "<<m_currentHost<<" port: "<<m_port<<endl;
   if (!connectToHost(m_currentHost.latin1(), m_port, false))
   {
      error(ERR_SLAVE_DEFINED, i18n("<qt>The Lisa daemon does not appear to be running.<p>"
                                    "In order to use the LAN Browser the Lisa daemon must be "
                                    "installed and activated by the system administrator."));
      return 0;
   }
   kdDebug(7101)<<"LANProtocol::lanReadDataFromServer() connected"<<endl;

   int receivedBytes(0);
   char* receiveBuffer(0);
   char tmpBuf[64*1024];
   int bytesRead(0);
   do
   {
      fd_set tmpFDs;
      FD_ZERO(&tmpFDs);
      FD_SET(m_iSock,&tmpFDs);
      timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      select(m_iSock+1,&tmpFDs,0,0,&tv);
      if (FD_ISSET(m_iSock,&tmpFDs))
      {
         bytesRead=read(tmpBuf,64*1024);
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: read "<<bytesRead<<" bytes"<<endl;

         if (bytesRead>0)
         {
            char *newBuf=new char[receivedBytes+bytesRead];
            if (receiveBuffer!=0) memcpy(newBuf,receiveBuffer,receivedBytes);
            memcpy(newBuf+receivedBytes,tmpBuf,bytesRead);
            receivedBytes+=bytesRead;
            if (receiveBuffer!=0) delete [] receiveBuffer;
            receiveBuffer=newBuf;
         }
      }
   } while (bytesRead>0);
   closeDescriptor();
   if ((bytesRead<0) || (receivedBytes<4))
   {
      delete [] receiveBuffer;
      error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(m_currentHost));
      return 0;
   }

   UDSEntry entry;

   char *currentBuf=receiveBuffer;
   int bytesLeft=receivedBytes;
   //this should be large enough for a name
   char tmpName[4*1024];
   //this should be large enough for the hostname
   char tmpHostname[4*1024];
   while (bytesLeft>0)
   {
      int tmpIP=2;
      tmpName[0]='\0';
      if ((memchr(currentBuf,0,bytesLeft)==0) || (memchr(currentBuf,int('\n'),bytesLeft)==0))
      {
         delete [] receiveBuffer;
         error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(m_currentHost));
         return 0;
      }
      kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: processing "<<currentBuf;
      //since we check for 0 and \n with memchr() we can be sure
      //at this point that tmpBuf is correctly terminated
      int length=strlen(currentBuf)+1;
      if (length<(4*1024))
         sscanf(currentBuf,"%u %s\n",&tmpIP,tmpName);
      else
      {
        kdDebug(7101)<<"LANProtocol::lanReadDataFromServer:  buffer overflow attempt detected, aborting"<<endl;
        break;
      }

      bytesLeft-=length;
      currentBuf+=length;
      if ((bytesLeft==0) && ((tmpIP==0) ||(tmpIP==1)) && (strstr(tmpName,"succeeded")!=0))
      {
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: succeeded"<<endl;
      }
      else if (tmpIP!=2)
      {
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: listing host: "<<tmpName<<" with ip: "<<tmpIP<<endl;
         UDSAtom atom;

         atom.m_uds = KIO::UDS_NAME;
         if (m_shortHostnames)
         {
            if (inet_addr(tmpName)!=-1)
               atom.m_str=tmpName;
            else
            {
               sscanf(tmpName,"%[^.]",tmpHostname);
               kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: Hostname of " << tmpName <<  " is " << tmpHostname << "\n";
               atom.m_str = tmpHostname;
            }
         }
         else
            atom.m_str = tmpName;

         entry.append( atom );
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         atom.m_uds = KIO::UDS_FILE_TYPE;
         atom.m_long = S_IFDIR; // it is always a directory
         entry.append( atom );
         listEntry(entry,false);
      }
   }

   listEntry( entry, true ); // ready
   delete [] receiveBuffer;
   return 1;
}

int LANProtocol::rlanReadDataFromServer()
{
   kdDebug(7101)<<"RLANProtocol::readDataFromServer"<<endl;

   int sockFD=socket(AF_LOCAL, SOCK_STREAM, 0);
   sockaddr_un addr;
   memset((char*)&addr,0,sizeof(addr));
   addr.sun_family=AF_LOCAL;
   QCString socketname="/tmp/resLisa-";

   struct passwd *user = getpwuid( getuid() );
   if ( user )
      socketname+=user->pw_name;
   else
      //should never happen
      socketname+="???";

   strlcpy(addr.sun_path,socketname,sizeof(addr.sun_path));
   int result=::connect(sockFD,(sockaddr*)&addr, sizeof(addr));

   kdDebug(7101)<<"readDataFromServer(): result: "<<result<<" name: "<<addr.sun_path<<" socket: "<<sockFD<<endl;

   if (result!=0)
   {
      ::close(sockFD);
      KProcess proc;
      proc<<"reslisa";

      bool ok=proc.start(KProcess::DontCare);
      if (!ok)
      {
         error( ERR_CANNOT_LAUNCH_PROCESS, "reslisa" );
         return 0;
      }
      //wait a moment
      //reslisa starts kde-config, then does up to 64
      //name lookups and then starts to ping
      //results won't be available before this is done
      kdDebug(7101)<<"sleeping..."<<endl;
      ::sleep(1);
      kdDebug(7101)<<"sleeping again..."<<endl;
      ::sleep(5);
      kdDebug(7101)<<"woke up "<<endl;
      sockFD=socket(AF_LOCAL, SOCK_STREAM, 0);

      memset((char*)&addr,0,sizeof(addr));
      addr.sun_family=AF_LOCAL;
      strlcpy(addr.sun_path,socketname,sizeof(addr.sun_path));

      kdDebug(7101)<<"connecting..."<<endl;
      result=::connect(sockFD,(sockaddr*)&addr, sizeof(addr));
      kdDebug(7101)<<"readDataFromServer() after starting reslisa: result: "<<result<<" name: "<<addr.sun_path<<" socket: "<<sockFD<<endl;
      if (result!=0)
      {
         error( ERR_CANNOT_OPEN_FOR_READING, socketname );
         return 0;
      }
      kdDebug(7101)<<"succeeded :-)"<<endl;
   }

   int receivedBytes(0);
   char* receiveBuffer(0);
   char tmpBuf[64*1024];
   int bytesRead(0);
   do
   {
      fd_set tmpFDs;
      FD_ZERO(&tmpFDs);
      FD_SET(sockFD,&tmpFDs);
      timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      select(sockFD+1,&tmpFDs,0,0,&tv);
      if (FD_ISSET(sockFD,&tmpFDs))
      {
         bytesRead=::read(sockFD,tmpBuf,64*1024);
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: read "<<bytesRead<<" bytes"<<endl;

         if (bytesRead>0)
         {
            char *newBuf=new char[receivedBytes+bytesRead];
            if (receiveBuffer!=0) memcpy(newBuf,receiveBuffer,receivedBytes);
            memcpy(newBuf+receivedBytes,tmpBuf,bytesRead);
            receivedBytes+=bytesRead;
            if (receiveBuffer!=0) delete [] receiveBuffer;
            receiveBuffer=newBuf;
         }
      }
   } while (bytesRead>0);
   ::close(sockFD);


   if ((bytesRead<0) || (receivedBytes<4))
   {
      delete [] receiveBuffer;
      error(ERR_CANNOT_OPEN_FOR_READING,socketname);
      return 0;
   }

   UDSEntry entry;

   char *currentBuf=receiveBuffer;
   int bytesLeft=receivedBytes;
   //this should be large enough for a name
   char tmpName[4*1024];
   //this should be large enough for the hostname
   char tmpHostname[4*1024];
   while (bytesLeft>0)
   {
      int tmpIP=2;
      tmpName[0]='\0';
      if ((memchr(currentBuf,0,bytesLeft)==0) || (memchr(currentBuf,int('\n'),bytesLeft)==0))
      {
         delete [] receiveBuffer;
         error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(socketname));
         return 0;
      }
      kdDebug(7101)<<"RLANProtocol::readDataFromServer: processing "<<currentBuf;
      //since we check for 0 and \n with memchr() we can be sure
      //at this point that tmpBuf is correctly terminated
      int length=strlen(currentBuf)+1;
      if (length<(4*1024))
         sscanf(currentBuf,"%u %s\n",&tmpIP,tmpName);
      else
      {
        kdDebug(7101)<<"RLANProtocol::readDataFromServer:  buffer overflow attempt detected, aborting"<<endl;
        break;
      }

      bytesLeft-=length;
      currentBuf+=length;
      if ((bytesLeft==0) && ((tmpIP==0) ||(tmpIP==1)) && (strstr(tmpName,"succeeded")!=0))
      {
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: succeeded"<<endl;
      }
      else if (tmpIP!=2)
      {
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: listing host: "<<tmpName<<" with ip: "<<tmpIP<<endl;
         UDSAtom atom;

         atom.m_uds = KIO::UDS_NAME;
         if (m_shortHostnames)
         {
            if (inet_addr(tmpName)!=-1)
               atom.m_str=tmpName;
            else
            {
               sscanf(tmpName,"%[^.]",tmpHostname);
               kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: Hostname of " << tmpName <<  " is " << tmpHostname << "\n";
               atom.m_str = tmpHostname;
            }
         }
         else
            atom.m_str = tmpName;
         entry.append( atom );
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         atom.m_uds = KIO::UDS_FILE_TYPE;
         atom.m_long = S_IFDIR; // it is always a directory
         entry.append( atom );
         listEntry(entry,false);
      }
   }

   listEntry( entry, true ); // ready
   delete [] receiveBuffer;
   return 1;
}

int LANProtocol::checkHost(const QString& host)
{
   kdDebug(7101)<<"LAN::checkHost() "<<host<<endl;

   QString hostUpper=host.upper();
   HostInfo* hostInfo=m_hostInfoCache[hostUpper];
   if (hostInfo!=0)
   {
      kdDebug(7101)<<"LAN::checkHost() getting from cache"<<endl;
      //this entry is too old, we delete it !
      if ((time(0)-hostInfo->created)>m_maxAge)
      {
         kdDebug(7101)<<"LAN::checkHost() cache content too old, deleting it"<<endl;
         m_hostInfoCache.remove(hostUpper);
         hostInfo=0;
      }
   }
   if (hostInfo==0)
   {
      hostInfo=new HostInfo;
      in_addr ip;

      struct hostent *hp=gethostbyname(host.latin1());
      if (hp==0)
      {
         error( ERR_UNKNOWN_HOST, host.latin1() );
         delete hostInfo;
         return 0;
      }
      memcpy(&ip, hp->h_addr, sizeof(ip));

      for (int i=0; i<KIOLAN_MAX; i++)
      {
         int result(0);
         if (m_protocolInfo[i].enabled==PORTSETTINGS_DISABLE)
            result=0;
         else if (m_protocolInfo[i].enabled==PORTSETTINGS_PROVIDE)
            result=1;
         else if (m_protocolInfo[i].enabled==PORTSETTINGS_CHECK)
            result=checkPort(m_protocolInfo[i].ports,ip);

         //host not reachable
         if (result==-1)
         {
            delete hostInfo;
            hostInfo=0;
            error( ERR_UNKNOWN_HOST, host.latin1() );
            return 0;
         }
         hostInfo->services[i]=result;
      }
      hostInfo->created=time(0);
      m_hostInfoCache.insert(hostUpper,hostInfo);
   }
   //here hostInfo is always != 0
   if (hostInfo==0)
   {
      error( ERR_INTERNAL, "hostInfo==0" );
      return 0;
   }

   UDSEntry entry;
   for (int i=0; i<KIOLAN_MAX; i++)
   {
      if (hostInfo->services[i]==1)
      {
         kdDebug(7101)<<"LAN::checkHost(): Host ["<<hostUpper<<"] Service ["<<m_protocolInfo[i].name<<"]"<<endl;
         UDSAtom atom;
         // name
         atom.m_uds = KIO::UDS_NAME;
         atom.m_str = m_protocolInfo[i].name;
         entry.append( atom );
         // size
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         // access permissions
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         // file type
         atom.m_uds = KIO::UDS_FILE_TYPE;
         if (strcmp(m_protocolInfo[i].name,"HTTP")==0)
         {
            // normal file -- if we called stat(2) on this,
            // this flag would be set in the st_mode field of struct stat
            atom.m_long=S_IFREG;
            entry.append(atom);

            // also define the mime-type for this file
            atom.m_uds = KIO::UDS_MIME_TYPE;
            atom.m_str="text/html";
            entry.append( atom );
         }
         else
         {
            // directory -- if we called stat(2) on this, then this
            // flag would be set in the st_mode field of the struct stat
            atom.m_long = S_IFDIR; // it is always a directory
            entry.append(atom);

            // also set the mime-type
            atom.m_uds = KIO::UDS_MIME_TYPE;
            atom.m_str="inode/directory";
            entry.append( atom );
         }
         listEntry(entry,false);
      }
   }
   listEntry( entry, true ); // ready
   return 1;
}

// Check if a service is running on a host with IP address 'ip'
// by checking all the ports in '_ports', using a non-blocking connect.
// Right now -- assume if *any* of these ports are active,
// the service is active.
int LANProtocol::checkPort( QValueVector<int>& _ports, in_addr ip )
{
   int _port=0;
   struct sockaddr_in to_scan;

   to_scan.sin_family = AF_INET;
   to_scan.sin_addr = ip;

   for (QValueVector<int>::iterator i= _ports.begin(); i != _ports.end(); i++)
   {
      _port=(*i);
      kdDebug(7101)<<"LANProtocol::checkPort: "<<_port<<endl;
      to_scan.sin_port = htons(_port);
      // open a TCP socket
      int mysocket = ::socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (mysocket< 0 )
      {
         std::cerr << "LanProt::checkPort: Error while opening Socket" << std::endl;
         ::close( mysocket );
         return 0;
      }
      //make the socket non blocking
      long int options = O_NONBLOCK | ::fcntl(mysocket, F_GETFL);
      if (::fcntl( mysocket, F_SETFL, options )!=0)
      {
         std::cerr << "LanProt::checkPort: Error making it nonblocking"<< std::endl;
         ::close( mysocket );
         return 0;
      }
      int result=connect( mysocket, (struct sockaddr *) &to_scan, sizeof( to_scan ));
      //it succeeded immediately
      if (result==0)
      {
         std::cerr<<"LANProtocol::checkPort("<<_port<<") connect succeeded immediately"<<std::endl;
         ::shutdown( mysocket, SHUT_RDWR );
         return 1;
      }
      //it failed
      if ((result<0) && (errno!=EINPROGRESS))
      {
         // errno was not EINPROGRESS, so there is some serious problem
         ::shutdown( mysocket, SHUT_RDWR );
         // maybe some other port will work
         continue;
      }
      // last errno was EINPROGRESS, so we should select() on socket
      // and wait for the final verdict
      timeval tv;
      tv.tv_sec=5;
      tv.tv_usec=0;
      fd_set rSet, wSet;
      FD_ZERO(&rSet);
      FD_SET(mysocket,&rSet);
      wSet=rSet;
      //timeout or error
      result=select(mysocket+1,&rSet,&wSet,0,&tv);
      ::shutdown( mysocket, SHUT_RDWR );
      if (result==1)
         return 1;
   }
   // gosh, no port worked
   return 0;

}

void LANProtocol::setHost( const QString& host, int port, const QString& , const QString& )
{
   if (m_isLanIoslave)
   {
      m_currentHost=host;
      if (port==0)
         m_port=7741;
      else
         m_port=port;
      kdDebug(7101)<<"LANProtocol::setHost: "<<m_currentHost<<endl;
   }
   else
   {
      if (!host.isEmpty())
         error(ERR_MALFORMED_URL,i18n("No hosts allowed in rlan:/ URL"));
   }
}

void LANProtocol::mimetype( const KURL& url)
{
   kdDebug(7101)<<"LANProtocol::mimetype -"<<url.prettyURL()<<"-"<<endl;
   QString path( QFile::encodeName(url.path()));
   QStringList pathList=QStringList::split( "/",path);
   if ((pathList.count()==2) && (pathList[1].upper()=="HTTP"))
   {
      //kdDebug(7101)<<"LANProtocol::mimeType text/html"<<endl;
      mimeType("text/html");
   }
   else
   {
      mimeType("inode/directory");
      //kdDebug(7101)<<"LANProtocol::mimeType inode/directory"<<endl;
   }
   finished();
}

void LANProtocol::listDir( const KURL& _url)
{
   KURL url(_url);
   QString path( QFile::encodeName(url.path()));
   if (path.isEmpty())
   {
      url.setPath("/");
      redirection(url);
      finished();
      return;
   }
   if ((m_currentHost.isEmpty()) && (m_isLanIoslave))
   {
      url.setHost(m_defaultLisaHost);
      redirection(url);
      finished();
      return;
   }
   kdDebug(7101)<<"LANProtocol::listDir: host: "<<m_currentHost<<" port: "<<m_port<<" path: "<<path<<endl;
   QStringList pathList=QStringList::split("/", path);
   kdDebug(7101)<<"parts are: "<<endl;
   for (QStringList::Iterator it=pathList.begin(); it !=pathList.end(); it++)
      kdDebug(7101)<<"-"<<(*it)<<"-"<<endl;
   if (pathList.count()>2)
   {
      kdDebug(7101)<<"LANProtocol::listDir: too deep path: "<<pathList.count()<<endl;
      error(ERR_DOES_NOT_EXIST,_url.prettyURL());
      return;
   }
   int succeeded(0);
   if (path=="/")
   {
      //get the stuff from the netland server
      succeeded=readDataFromServer();
   }
   else if (pathList.count()==1)
   {
      //check the ports and stuff
      succeeded=checkHost(pathList[0]);
   }
   else
   {
      kdDebug(7101)<<"LANProtocol::listDir: trying to redirect"<<endl;
      int isSupportedProtocol(0);
      for (int i=0; i<KIOLAN_MAX; i++)
      {
         if (pathList[1].upper()==m_protocolInfo[i].name)
         {
            isSupportedProtocol=m_protocolInfo[i].enabled;
            break;
         }
      }
      if (isSupportedProtocol==PORTSETTINGS_DISABLE)
      {
         kdDebug(7101)<<"LANProtocol::listDir: protocol not enabled "<<endl;
         error(ERR_DOES_NOT_EXIST,_url.prettyURL());
         return;
      }
      //redirect it
      KURL newUrl(pathList[1]+"://"+pathList[0]);
      redirection(newUrl);
      succeeded=1;
   }
   if (succeeded) finished();
}

void LANProtocol::stat( const KURL & url)
{
   kdDebug(7101)<<"LANProtocol::stat: "<<endl;
   UDSEntry entry;
   UDSAtom atom;

   atom.m_uds = KIO::UDS_NAME;
   atom.m_str = url.path();
   entry.append( atom );
   atom.m_uds = KIO::UDS_SIZE;
   atom.m_long = 1024;
   entry.append(atom);

   atom.m_uds = KIO::UDS_ACCESS;
   atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
   //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
   entry.append(atom);


   QString path( QFile::encodeName(url.path()));
   QStringList pathList=QStringList::split( "/",path);
   if ((pathList.count()==2) && (pathList[1].upper()=="HTTP"))
   {
      atom.m_uds = KIO::UDS_FILE_TYPE;
      atom.m_long=S_IFREG;
      entry.append(atom);
      atom.m_uds = KIO::UDS_MIME_TYPE;
      atom.m_str="text/html";
      //kdDebug(7101)<<"LANProtocol::stat: http is reg file"<<endl;
      entry.append( atom );
   }
   else
   {
      atom.m_uds = KIO::UDS_FILE_TYPE;
      atom.m_long = S_IFDIR; // it is always a directory
      entry.append(atom);
      atom.m_uds = KIO::UDS_MIME_TYPE;
      atom.m_str="inode/directory";
      //kdDebug(7101)<<"LANProtocol::stat: is dir"<<endl;
      entry.append( atom );
   }

   statEntry( entry );
   finished();
}

void LANProtocol::get(const KURL& url )
{
   QString path(QFile::encodeName(url.path()));

   kdDebug(7101)<<"LANProtocol::get: "<<path<<endl;
   QStringList pathList=QStringList::split("/", path);
   kdDebug(7101)<<"parts are: "<<endl;
   for (QStringList::Iterator it=pathList.begin(); it !=pathList.end(); it++)
      kdDebug(7101)<<"-"<<(*it)<<"-"<<endl;
   if ((pathList.count()!=2) || (pathList[1].upper()!="HTTP"))
   {
      kdDebug(7101)<<"LANProtocol::get: invalid url: "<<pathList.count()<<endl;
      error(ERR_DOES_NOT_EXIST,url.prettyURL());
      return;
   }
   KURL newUrl("http://"+pathList[0]);
   redirection(newUrl);
   finished();
   return;
}
