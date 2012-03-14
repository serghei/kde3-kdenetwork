 /*    netmanager.cpp
 *
 *    Copyright (c) 2000, Alexander Neundorf
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

#include "netmanager.h"
#include "lisadefines.h"

#include <iostream>
#include <unistd.h>

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif


#ifdef LISA_DEBUG
#undef LISA_DEBUG
#endif

#ifdef dcerr
#undef dcerr
#endif

#ifdef mdcerr
#undef mdcerr
#endif

#define LISA_DEBUG 0
#define dcerr if (LISA_DEBUG==1) std::cerr<<"NetManager "
#define mdcerr if (LISA_DEBUG==1) std::cerr<<procId<<" NetManager "

static void sig_child_handler(int)
{
   int saved_errno = errno;
   
   while (waitpid(-1, NULL, WNOHANG) > 0)
      ; // repeat
   
   errno = saved_errno;
}

NetManager::NetManager(int& rawSocketFD, int portToUse, MyString configFile, int configStyle, int strictMode)
:NetScanner(rawSocketFD,strictMode)
//,validator()
,m_listenFD(-1)
,m_bcFD(-1)
,m_basePort(portToUse)
,m_pipeFD(-1)
,m_receiveBuffer(0)
,m_receivedBytes(0)
,m_childPid(0)
,m_lastUpdate(0)

,m_isInformed(0)
,m_isBeingScanned(0)
,m_firstRun(1)
,m_serverServer(0)
,m_servedThisPeriod(0)

,m_serveCount(0)
,m_refreshTime(60)
,m_initialRefreshTime(60)
,m_increasedRefreshTime(0)
,m_broadcastAddress(0)
,m_extraConfigFile(configFile)
,m_configStyle(configStyle)
,m_usedConfigFileName("")
{
   mdcerr<<"NetManager::NetManager"<<std::endl;
   m_startedAt=time(0);
   
   struct sigaction act;
   act.sa_handler=sig_child_handler;
   sigemptyset(&(act.sa_mask));
   sigaddset(&(act.sa_mask), SIGCHLD);
   // Make sure we don't block this signal. gdb tends to do that :-(
   sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0);

   act.sa_flags = SA_NOCLDSTOP;
#ifdef SA_RESTART
   act.sa_flags |= SA_RESTART;
#endif

   sigaction( SIGCHLD, &act, NULL );
}

NetManager::~NetManager()
{
   mdcerr<<"NetManager destructor ..."<<std::endl;
   if (m_receiveBuffer!=0) delete [] m_receiveBuffer;
   ::close(m_listenFD);
   ::close(m_bcFD);
}

void NetManager::readConfig()
{
   m_usedConfigFileName=getConfigFileName();
   if (m_usedConfigFileName.isEmpty())
   {
      std::cout<<"configfile not found"<<std::endl;
      std::cout<<"use the command line option --help and \ntake a look at the README for more information"<<std::endl;
      exit(1);
   }

   Config config(m_usedConfigFileName);
   NetManager::configure(config);
   NetScanner::configure(config);
   validator.configure(config);
   //after reading the new configuration we should really update
   m_lastUpdate=0;
}

void NetManager::configure(Config& config)
{
   m_refreshTime=config.getEntry("UpdatePeriod",300);
   MyString tmp=stripWhiteSpace(config.getEntry("BroadcastNetwork","0.0.0.0/255.255.255.255;"));
   tmp=tmp+";";
   mdcerr<<"NetManager::readConfig: "<<tmp<<std::endl;
   MyString netAddressStr=tmp.left(tmp.find('/'));
   tmp=tmp.mid(tmp.find('/')+1);
   tmp=tmp.left(tmp.find(';'));
   mdcerr<<"NetManager::readConfig: broadcastNet "<<netAddressStr<<" with mask "<<tmp<<std::endl;
   int netMask=inet_addr(tmp.c_str());
   int netAddress=inet_addr(netAddressStr.c_str());
   m_broadcastAddress= netAddress | (~netMask);
   mdcerr<<"NetManager::readConfig: net "<<std::ios::hex<<netAddress<<" with mask "<<netMask<<" gives "<<m_broadcastAddress<<std::endl;
   
   //maybe this way we can avoid that all servers on the net send
   //their requests synchronously, since now the refreshtime isn't
   //always the eact value of m_refreshTime, but differs always slightly
   if ((m_refreshTime%SELECT_TIMEOUT)==0) m_refreshTime+=2;
   //some limits from half a minute to half an hour
   if (m_refreshTime<30) m_refreshTime=30;
   if (m_refreshTime>1800) m_refreshTime=1800;
   m_initialRefreshTime=m_refreshTime;
}

int NetManager::prepare()
{
   mdcerr<<"NetManager::prepare"<<std::endl;

   ::close(m_listenFD);
   ::close(m_bcFD);
   int result(0);
   if (m_strictMode)
   {
      m_listenFD=::socket(AF_LOCAL, SOCK_STREAM, 0);
      //m_listenFD=::socket(AF_LOCAL, SOCK_STREAM, IPPROTO_TCP);
      MyString socketName("/tmp/resLisa-");
      struct passwd *user = getpwuid( getuid() );
      if ( user )
         socketName+=user->pw_name;
      else
         //should never happen
         socketName+="???";
      ::unlink(socketName.data());
      sockaddr_un serverAddr;
      if (socketName.length() >= sizeof(serverAddr.sun_path))
      {
         std::cout<<"NetManager::prepare: your user name  \""<<user->pw_name<<"\" is too long, exiting."<<std::endl;
         return 0;
      }
      memset((void*)&serverAddr, 0, sizeof(serverAddr));
      serverAddr.sun_family=AF_LOCAL;
      strncpy(serverAddr.sun_path,socketName.data(),sizeof(serverAddr.sun_path));
      result=::bind(m_listenFD,(sockaddr*) &serverAddr,sizeof(serverAddr));
      if (result!=0)
      {
         std::cout<<"NetManager::prepare: bind (UNIX socket) failed, errno: "<<errno<<std::endl;
         return 0;
      }
   }
   else
   {
      //create a listening port and listen
      m_listenFD = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (m_listenFD==-1)
      {
         std::cout<<"NetManager::prepare: socket(TCP) failed, errno: "<<errno<<std::endl;
         return 0;
      }

      sockaddr_in serverAddress;
//      bzero((char*)&serverAddress, sizeof(serverAddress));
      memset((void*)&serverAddress, 0, sizeof(serverAddress));
      serverAddress.sin_family      = AF_INET;
      serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
      serverAddress.sin_port        = htons(m_basePort);

      int on(1);
      result=setsockopt(m_listenFD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
      if (result!=0)
      {
         std::cout<<"NetManager::prepare: setsockopt(SO_REUSEADDR) failed"<<std::endl;
         return 0;
      }
      result=::bind(m_listenFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
      if (result!=0)
      {
         std::cout<<"NetManager::prepare: bind (TCP) failed, errno: "<<errno<<std::endl;
         return 0;
      }
   }
   result=::listen(m_listenFD, 32);
   if (result!=0)
   {
      std::cout<<"NetManager::prepare: listen failed"<<std::endl;
      return 0;
   }
   mdcerr<<"NetManager::prepare: listening on port "<<m_basePort<<"..."<<std::endl;

   return 1;
}

void NetManager::generateFDset(fd_set *tmpFDs)
{
   mdcerr<<"NetManager::generateFDset"<<std::endl;
   
   FD_ZERO(tmpFDs);
   FD_SET(m_listenFD,tmpFDs);
   mdcerr<<"NetManager::generateFDset: adding listen FD="<<m_listenFD<<std::endl;
//   for (Client* tmpClient=clients.first(); tmpClient!=0; tmpClient=clients.next())
   for (std::list<Client>::iterator tmpClient=clients.begin(); tmpClient != clients.end(); tmpClient++)
      if (tmpClient->fd()!=-1)
      {
         mdcerr<<"NetManager::generateFDset: adding client FD="<<tmpClient->fd()<<std::endl;
         FD_SET(tmpClient->fd(),tmpFDs);
      }

   if (m_pipeFD!=-1)
   {
      mdcerr<<"NetManager::generateFDset: adding pipeFD="<<m_pipeFD<<std::endl;
      FD_SET(m_pipeFD,tmpFDs);
   }

   if ((m_bcFD!=-1) && (!m_strictMode))
   {
      mdcerr<<"NetManager::generateFDset: adding m_bcFD="<<m_bcFD<<std::endl;
      FD_SET(m_bcFD,tmpFDs);
   }
}

int NetManager::waitForSomethingToHappen(fd_set *tmpFDs)
{
   mdcerr<<"NetManager::waitForSomethingToHappen for 10 seconds"<<std::endl;
   if (m_firstRun)
   {
      tv.tv_sec = 1;
      m_firstRun=0;
   }
   else
      tv.tv_sec = SELECT_TIMEOUT;

   tv.tv_usec = 0;
   //generateFDset(tmpFDs);
   int result=select(getMaxFD(),tmpFDs,0,0,&tv);
   if (result>0) return 1;
   else return 0;
}

int NetManager::getMaxFD()
{
   int maxFD(m_listenFD);
//   for (Client* tmpClient=clients.first(); tmpClient!=0; tmpClient=clients.next())
   for (std::list<Client>::iterator tmpClient=clients.begin(); tmpClient != clients.end(); tmpClient++)
      if (tmpClient->fd()>maxFD)
         maxFD=tmpClient->fd();

   if (m_pipeFD>maxFD) maxFD=m_pipeFD;
   if (m_bcFD>maxFD) maxFD=m_bcFD;

   mdcerr<<"NetManager::getMaxFD()="<<maxFD+1<<std::endl;
   return maxFD+1;
}

int fileReadable(const MyString& filename)
{
   FILE *file=::fopen(filename.data(), "r");
   if (file==0)
      return 0;
   fclose(file);
   return 1;
}

MyString NetManager::getConfigFileName()
{
   MyString tmpBase(CONFIGFILEBASENAME);

   if (m_strictMode)
      tmpBase=STRICTCONFIGFILEBASENAME;

   if (!m_extraConfigFile.isEmpty())
      m_configStyle=EXTRACONFIGSTYLE;

   MyString tmpFilename;
   if (m_configStyle==EXTRACONFIGSTYLE)
   {
      tmpFilename=m_extraConfigFile;
      if (fileReadable(tmpFilename))
         return tmpFilename;
      return "";
   }
   else if (m_configStyle==UNIXCONFIGSTYLE)
   {
      tmpFilename=getenv("HOME");
      tmpFilename+=MyString("/.")+tmpBase;
      if (fileReadable(tmpFilename))
         return tmpFilename;
      tmpFilename="/etc/";
      tmpFilename+=tmpBase;
      if (fileReadable(tmpFilename))
         return tmpFilename;
      return "";
   }
/*   else if (m_configStyle==KDE1CONFIGSTYLE)
   {
      tmpFilename=getenv("HOME");
      tmpFilename+=MyString("/.kde/share/config/")+tmpBase;
      if (fileReadable(tmpFilename))
         return tmpFilename;
      tmpFilename=getenv("KDEDIR");
      tmpFilename+=MyString("/share/config/")+tmpBase;
      if (fileReadable(tmpFilename))
         return tmpFilename;
      return "";
   }
   else if (m_configStyle==KDE2CONFIGSTYLE)
   {
      FILE *kdeConfig=popen("kde-config --path config","r");
      if (kdeConfig==0)
      {
         std::cout<<"could not execute kde-config, check your KDE 2 installation\n"<<std::endl;
         return "";
      };
      //this should be large enough
      char buf[4*1024];
      memset(buf,0,4*1024);
      fgets(buf,4*1024-1,kdeConfig);
      // Warning: The return value of plcose may be incorrect due to the
      // SIGCHLD handler that is installed. Ignore it!
      pclose(kdeConfig);
      int length=strlen(buf);
      if (buf[length-1]=='\n') buf[length-1]='\0';
      MyString kdeDirs(buf);
      while (kdeDirs.contains(':'))
      {
         MyString nextDir=kdeDirs.left(kdeDirs.find(':'));
         kdeDirs=kdeDirs.mid(kdeDirs.find(':')+1);
         nextDir=nextDir+tmpBase;
         mdcerr<<"trying to open "<<nextDir<<std::endl;
         if (fileReadable(nextDir))
            return nextDir;
      };
      kdeDirs=kdeDirs+tmpBase;
      mdcerr<<"trying to open "<<kdeDirs<<std::endl;
      if (fileReadable(kdeDirs))
         return kdeDirs;
      kdeDirs="/etc/";
      kdeDirs=kdeDirs+tmpBase;
      if (fileReadable(kdeDirs))
         return kdeDirs;
      return "";
   }*/
   return "";
}

int NetManager::run()
{
   int continueMainLoop(1);
   //not forever
	while(continueMainLoop)
   {
      mdcerr<<"\nNetManager::run: next loop: "<<clients.size()<<" clients"<<std::endl;
      fd_set tmpFDs;
      generateFDset(&tmpFDs);

      int result=waitForSomethingToHappen(&tmpFDs);
      mdcerr<<"NetManager::run: something happened..."<<std::endl;
      //timeout
      if (result==0)
      {
         mdcerr<<"NetManager::run: serverServer=="<<m_serverServer<<std::endl;
         mdcerr<<"NetManager::run: scanning after timeout"<<std::endl;
         scan();
      }
      else
      {
         //a new connection ?
         if (FD_ISSET(m_listenFD,&tmpFDs))
         {
            mdcerr<<"NetManager::run: on m_listenFD"<<std::endl;
            struct sockaddr_in clientAddress;
            socklen_t clilen(sizeof(clientAddress));
//            bzero((char*)&clientAddress, clilen);
            memset((void*)&clientAddress,0,sizeof(clientAddress));
            int connectionFD=::accept(m_listenFD,(struct sockaddr *) &clientAddress, &clilen);
            if ((validator.isValid(clientAddress.sin_addr.s_addr)) || (m_strictMode))
            {
               mdcerr<<"NetManager::run: adding client"<<std::endl;
               addClient(connectionFD);
               m_servedThisPeriod=1;
               m_refreshTime=m_initialRefreshTime;
               m_increasedRefreshTime=0;
            }
            else
            {
               mdcerr<<"NetManager::run: kicking client"<<std::endl;
               ::close(connectionFD);
            }
         }
         checkClientsAndPipes(&tmpFDs);
      }
      serveAndClean();
   }
   return 1;
}

void NetManager::addClient(int socketFD)
{
   mdcerr<<"NetManager::addClient on FD="<<socketFD<<std::endl;
   if (socketFD==-1) return;
   Client c(this,socketFD,0);
   clients.push_back(c);
}

void NetManager::checkClientsAndPipes(fd_set *tmpFDs)
{
   mdcerr<<"NetManager::checkClientsAndPipes()"<<std::endl;
   //actually the clients should not send anything
//   for (Client *tmpClient=clients.first(); tmpClient!=0; tmpClient=clients.next())
   for (std::list<Client>::iterator tmpClient=clients.begin(); tmpClient != clients.end(); tmpClient++)
   {
      mdcerr<<"NetManager::checkClientsAndPipes: checking client"<<std::endl;
      if (FD_ISSET(tmpClient->fd(),tmpFDs))
      {
         mdcerr<<"NetManager::checkClientsAndPipes: client sent something"<<std::endl;
         tmpClient->read();
      }
   }

   //now check wether we received a broadcast
   //m_bcFD should always be -1 in strictMode
   if ((m_bcFD!=-1) && (!m_strictMode))
   {
      mdcerr<<"NetManager::checkClientsAndPipe: checking bcFD"<<std::endl;
      //yes !
      if (FD_ISSET(m_bcFD,tmpFDs))
         answerBroadcast();
   }

   //read the stuff from the forked pipe
   if (m_pipeFD!=-1)
   {
      mdcerr<<"NetManager::checkClientsAndPipe: checking pipe"<<std::endl;
      if (FD_ISSET(m_pipeFD,tmpFDs))
      {
         mdcerr<<"NetManager::checkClientsAndPipes: pipe sent something"<<std::endl;
         int result=readDataFromFD(m_pipeFD);
         if (result!=1)
         {
            ::close(m_pipeFD);
            m_pipeFD=-1;
            mdcerr<<"NetManager::checkClientsAndPipes: everything read from pipe from proc "<<m_childPid<<std::endl;
            processScanResults();
         }
      }
   }
}

void NetManager::answerBroadcast()
{
   //actually we should never get here in strictMode
   if (m_strictMode) return;
   
   //this one is called only in checkClientsAndPipes()
   //if we are sure that we received something on m_bcFD
   //so we don't need to check here again
   
   mdcerr<<"NetManager::answerBroadcast: received BC"<<std::endl;
   struct sockaddr_in sAddr;
   socklen_t length(sizeof(sockaddr_in));
   char buf[1024];
   int result=recvfrom(m_bcFD, (char*)buf, 1024, 0, (sockaddr*)&sAddr,&length);
   mdcerr<<"NetManager::answerBroadcast: received successfully"<<std::endl;
   //did recvfrom() succeed ?
   //our frame is exactly MYFRAMELENGTH bytes big, if the received one has a different size,
   //it must be something different
   if (result!=MYFRAMELENGTH) return;
   //if it has the correct size, it also must have the correct identifier
   MyFrameType *frame=(MyFrameType*)(void*)buf;
   if ((ntohl(frame->id)!=MY_ID) ||
       ((ntohl(frame->unused1)==getpid()) && (ntohl(frame->unused2)==m_startedAt)))
   {
      mdcerr<<"NetManager::answerBroadcast: must be the same machine"<<std::endl;
      return;
   }

   //mdcerr<<"received "<<ntohl(buf[0])<<" from "<<inet_ntoa(sAddr.sin_addr)<<hex<<" ";
   //mdcerr<<sAddr.sin_addr.s_addr<<" //"<<ntohl(sAddr.sin_addr.s_addr)<<dec<<std::endl;
   //will we answer this request ?
   if (!validator.isValid(sAddr.sin_addr.s_addr))
   {
      mdcerr<<"NetManager::answerBroadcast: invalid sender"<<std::endl;
      return;
   }

   //create the answering socket

   int answerFD=::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (answerFD==-1)
   {
      mdcerr<<"NetManager::answerBroadcast: could not create answering socket"<<std::endl;
      return;
   }
   
   sAddr.sin_family=AF_INET;
   sAddr.sin_port=htons(m_basePort+1);
   MyFrameType answerFrame;
   answerFrame.id=htonl(MY_ID);
   answerFrame.unused1=0;
   answerFrame.unused2=0;
   //don't care about the result
   mdcerr<<"NetManager::answerBroadcast: sending answer"<<std::endl;

   result=::sendto(answerFD,(char*)&answerFrame,sizeof(answerFrame),0,(sockaddr*)&sAddr,length);
   mdcerr<<"sent "<<result<<" byte using sendto"<<std::endl;
   ::close(answerFD);
   //sent answer
}

void NetManager::serveAndClean()
{
   //try to serve the request
//   for (Client *tmpClient=clients.first(); tmpClient!=0; tmpClient=clients.next())
   for (std::list<Client>::iterator tmpClient=clients.begin(); tmpClient != clients.end(); tmpClient++)
   {
      mdcerr<<"NetManager::serveAndClean: trying to get info"<<std::endl;
      tmpClient->tryToGetInfo();
   }
   
   //try to delete served the clients
   /** PM: substituted STL code
   for (Client* tmpClient=clients.first();tmpClient!=0; tmpClient=clients.next())
   {
      //if we served the client or if he's already half a minute
      //connected remove it
      //this way we get rid of clients if something went wrong and
      //maybe it's even a security point, I don't know
      if ((tmpClient->done()) || (tmpClient->age()>30))
      {
         mdcerr<<"NetManager::serveAndClean: removing Client"<<std::endl;
         clients.remove(tmpClient);
         tmpClient=clients.first();
      }
      }*/
   clients.remove_if(client_is_done());
}

void NetManager::scan()
{
   mdcerr<<"NetManager::scan()"<<std::endl;
   if (isBeingScanned()) return;

   time_t currentTime=time(0);
   mdcerr<<"currentTime: "<<currentTime<<" lastUpdate: "<<m_lastUpdate<<std::endl;
   if ((currentTime-m_lastUpdate)<m_refreshTime) return;
   mdcerr<<"NetManager::scan: scanning..."<<std::endl;

   m_isBeingScanned=1;
   int fileDescr[2];
   ::pipe(fileDescr);
   mdcerr<<"NetScanner::scan: file-descr[0]: "<<fileDescr[0]<<std::endl;
   mdcerr<<"NetScanner::scan: file-descr[1]: "<<fileDescr[1]<<std::endl;
   int pid=fork();
	if (pid==-1)
	{
      mdcerr<<"NetScanner::scan: error occurred"<<std::endl;
      return;
   }
   else if (pid!=0)
   {
      //parent
      ::close(fileDescr[1]);
      m_pipeFD=fileDescr[0];
      m_childPid=pid;
      return;
   }
   //child
   procId="** child ** ";
   mdcerr<<" NetManager::scan: a child was born"<<std::endl;
   if (m_strictMode)
   {
      mdcerr<<" NetManager::scan: scanning myself in strict mode, becoming serverServer"<<std::endl;
      doScan();
      //in the child we don't have to call setServerServer()
      //since this opens the listening socket, what has to be done
      //in the parent process
      m_serverServer=1;
   }
   else
   {
      int serverAddress=findServerServer();
      if (serverAddress==0)
      {
         mdcerr<<" NetManager::scan: scanning myself, becoming serverServer"<<std::endl;
         doScan();
         //in the child we don't have to call setServerServer()
         //since this opens the listening socket, what has to be done
         //in the parent process
         m_serverServer=1;
      }
      else
      {
         mdcerr<<" NetManager::scan: getting list from serverServer"<<std::endl;
         getListFromServerServer(serverAddress);
         m_serverServer=0;
      }
   }
   mdcerr<<" NetScanner::scan: sending information to parent process"<<std::endl;
   writeDataToFD(fileDescr[1],m_serverServer);
   mdcerr<<" NetScanner::scan: closed FD: "<<::close(fileDescr[1])<<std::endl;
   mdcerr<<" NetScanner::scan: exiting now"<<std::endl;
   ::exit(0);
}

int NetManager::writeDataToFD(int fd, int serverServer)
{
   mdcerr<<" NetManager::writeDataToFD fd="<<fd<<std::endl;
   m_serveCount++;
   char buffer[1024];
   
   int length;
//   for (Node* tmpNode=hostList->first(); tmpNode!=0; tmpNode=hostList->next())
   for (std::list<Node>::iterator tmpNode=hostList->begin(); tmpNode!=hostList->end(); tmpNode++)
   {
      sprintf(buffer,"%u %s\n",tmpNode->ip,tmpNode->name.left(1000).c_str());
      length=strlen(buffer)+1;
      const char *currentBuf=buffer;
      //make sure that everything is written
      while (length>0)
      {
         int result=::write(fd,currentBuf,length);
         mdcerr<<"NetManager::writeDataToFD: wrote "<<result<<" bytes"<<std::endl;
         if (result==-1)
         {
            perror("hmmpf: ");
            return 0;
         }
         length-=result;
         currentBuf+=result;
      }
   }
   //and a last line
   sprintf(buffer,"%d succeeded\n",serverServer);
   length=strlen(buffer)+1;
   const char *currentBuf=buffer;
   //make sure that everything is written
   while (length>0)
   {
      int result=::write(fd,currentBuf,length);
      if (result==-1) return 0;
      length-=result;
      currentBuf+=result;
   }
   return 1;
}

int NetManager::readDataFromFD(int fd)
{
   mdcerr<<"NetManager::readDataFromFD"<<std::endl;
   char tmpBuf[64*1024];
   int result=::read(fd,tmpBuf,64*1024);
   mdcerr<<"NetManager::readDataFromFD: read "<<result<<" bytes"<<std::endl;
   if (result==-1) return -1;
   if (result==0)
   {
      mdcerr<<"NetManager::readDataFromFD: FD was closed"<<std::endl;
      return 0;
   }
   char *newBuf=new char[m_receivedBytes+result];
   if (m_receiveBuffer!=0) memcpy(newBuf,m_receiveBuffer,m_receivedBytes);
   memcpy(newBuf+m_receivedBytes,tmpBuf,result);
   m_receivedBytes+=result;
   if (m_receiveBuffer!=0) delete [] m_receiveBuffer;
   m_receiveBuffer=newBuf;
   // too much data - abort at 2MB to avoid memory exhaustion
   if (m_receivedBytes>2*1024*1024)
     return 0;

   return 1;
}

int NetManager::processScanResults()
{
   mdcerr<<"NetManager::processScanResults"<<std::endl;
   if (m_receiveBuffer==0) return 0;
   std::list<Node> *newNodes=new std::list<Node>;

   char *tmpBuf=m_receiveBuffer;
   int bytesLeft=m_receivedBytes;
   mdcerr<<"m_receivedBytes: "<<m_receivedBytes<<" bytesLeft: "<<bytesLeft<<std::endl;
   //this should be large enough for a name
   //and the stuff which is inserted into the buffer
   //comes only from ourselves ... or attackers :-(
   char tmpName[1024*4];
   while (bytesLeft>0)
   {
      int tmpIP=2;  // well, some impossible IP address, 0 and 1 are already used for the last line of output
      tmpName[0]='\0';
      if ((memchr(tmpBuf,0,bytesLeft)==0) || (memchr(tmpBuf,int('\n'),bytesLeft)==0))
      {
         delete newNodes;
         hostList->clear();

         m_lastUpdate=time(0);
         delete [] m_receiveBuffer;
         m_receiveBuffer=0;
         m_receivedBytes=0;
         m_isInformed=1;
         m_isBeingScanned=0;
         return 0;
      }
      //mdcerr<<"NetManager::processScanResults: processing -"<<tmpBuf;
      //since we check for 0 and \n with memchr() we can be sure
      //at this point that tmpBuf is correctly terminated
      int length=strlen(tmpBuf)+1;
      if (length<(4*1024))
         sscanf(tmpBuf,"%u %s\n",&tmpIP,tmpName);

      bytesLeft-=length;
      tmpBuf+=length;
      mdcerr<<"length: "<<length<<" bytesLeft: "<<bytesLeft<<std::endl;
      if ((bytesLeft==0) && ((tmpIP==0) ||(tmpIP==1)) && (strstr(tmpName,"succeeded")!=0))
      {
         mdcerr<<"NetManager::processScanResults: succeeded :-)"<<std::endl;
         delete hostList;
         hostList=newNodes;

         m_lastUpdate=time(0);
         delete [] m_receiveBuffer;
         m_receiveBuffer=0;
         m_receivedBytes=0;
         m_isInformed=1;
         m_isBeingScanned=0;
         adjustRefreshTime(tmpIP);
         enableServerServer(tmpIP);
         //m_serverServer=tmpIP;

         return 1;
      }
      else if (tmpIP!=2)
      {
         //mdcerr<<"NetManager::processScanResults: adding host: "<<tmpName<<" with ip: "<<tmpIP<<std::endl;
         newNodes->push_back(Node(tmpName,tmpIP));
      }
   }
   //something failed :-(
   delete newNodes;
   hostList->clear();

   m_lastUpdate=time(0);
   delete [] m_receiveBuffer;
   m_receiveBuffer=0;
   m_receivedBytes=0;
   m_isInformed=1;
   m_isBeingScanned=0;

   mdcerr<<"NetScanner::processScanResult: finished"<<std::endl;
   return 0;
}

void NetManager::adjustRefreshTime(int serverServer)
{
   //we are becoming server server
   if (((m_serverServer==0) && (serverServer)) || (m_servedThisPeriod))
   {
      m_increasedRefreshTime=0;
      m_refreshTime=m_initialRefreshTime;
   }
   //nobody accessed the server since the last update
   //so increase the refresh time
   //this should happen more seldom to the serverServer
   //than to others
   else
   {
      //up to the 16 times refresh time
      if (m_increasedRefreshTime<4)
      {
         m_increasedRefreshTime++;
         m_refreshTime*=2;
      };
   };
   m_servedThisPeriod=0;
   
}

void NetManager::enableServerServer(int on)
{
   mdcerr<<"NetManager::enableServerServer: "<<on<<std::endl;
   //in strictMode we don't listen to broadcasts from the network
   if (m_strictMode) return;

   m_serverServer=on;
   if (on)
   {
      //nothing has to be done
      if (m_bcFD!=-1) return;
      // otherwise create the socket which will listen for broadcasts
      sockaddr_in my_addr;
      my_addr.sin_family=AF_INET;
      my_addr.sin_port=htons(m_basePort);
      my_addr.sin_addr.s_addr=0;

      m_bcFD=::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (m_bcFD==-1)
      {
         mdcerr<<"NetManager::enableServerServer: socket() failed"<<std::endl;
         m_serverServer=0;
         return;
      };
      int on(1);
      int result=setsockopt(m_bcFD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
      if (result!=0)
      {
         mdcerr<<"NetManager::enableServerServer: setsockopt(SO_REUSEADDR) failed"<<std::endl;
         m_serverServer=0;
         ::close(m_bcFD);
         m_bcFD=-1;
         return;
      };
      result=::bind(m_bcFD, (struct sockaddr *) &my_addr, sizeof(my_addr));
      if (result!=0)
      {
         mdcerr<<"NetManager::enableServerServer: bind (UDP) failed"<<std::endl;
         m_serverServer=0;
         ::close(m_bcFD);
         m_bcFD=-1;
         return;
      };
   }
   else
   {
      ::close(m_bcFD);
      m_bcFD=-1;
   };
}

int NetManager::findServerServer()
{
   mdcerr<<" NetManager::findServerServer"<<std::endl;
   //actually this should never be called in strictMode
   if (m_strictMode) return 0;
   if (!validator.isValid(m_broadcastAddress))
   {
      mdcerr<<" NetManager::findServerServer: invalid broadcastAddress"<<std::endl;
      return 0;
   };
   //create a socket for sending the broadcast
   //we don't have to set SO_REUSEADDR, since we don't call bind()
   //to this socket
   int requestFD=::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (requestFD==-1)
   {
      mdcerr<<" NetManager::findServerServer: could not create request socket"<<std::endl;
      return 0;
   };

   int on(1);
   //this is actually the only socket which will send broacasts
   int result=setsockopt(requestFD, SOL_SOCKET, SO_BROADCAST,(char*)&on, sizeof(on));
   if (result!=0)
   {
      mdcerr<<"setsockopt(SO_BROADCAST) failed"<<std::endl;
      ::close(requestFD);
      return 0;
   };
   
   //create a socket for receiving the answers
   int answerFD=::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (answerFD==-1)
   {
      mdcerr<<" NetManager::findServerServer"<<std::endl;
      ::close(requestFD);
      return 0;
   };
   
   //since this socket will be bound, we have to set SO_REUSEADDR
   result=setsockopt(answerFD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
   if (result!=0)
   {
      mdcerr<<"setsockopt(SO_REUSEADDR) failed"<<std::endl;
      ::close(answerFD);
      ::close(requestFD);
      return 0;
   };
   
   sockaddr_in my_addr;
   my_addr.sin_family=AF_INET;
   my_addr.sin_port=htons(m_basePort+1);
   my_addr.sin_addr.s_addr=0;
   //this one has to be bound
   result=::bind(answerFD, (struct sockaddr *) &my_addr, sizeof(my_addr));
   if (result!=0)
   {
      mdcerr<<"bind (UDP) failed"<<std::endl;
      ::close(answerFD);
      ::close(requestFD);
      return 0;
   };
   
   //now send the broadcast
   struct sockaddr_in sAddr;
   sAddr.sin_addr.s_addr=m_broadcastAddress;
   sAddr.sin_family=AF_INET;
   sAddr.sin_port=htons(m_basePort);
   socklen_t length(sizeof(sockaddr_in));
   mdcerr<<" NetManager::findServerServer: broadcasting to: "
   <<std::ios::hex<<m_broadcastAddress<<std::ios::dec<<std::endl;
   
   MyFrameType requestFrame;
   requestFrame.id=htonl(MY_ID);
   requestFrame.unused1=htonl(getppid());
   requestFrame.unused2=htonl(m_startedAt);

   result=::sendto(requestFD,(char*)&requestFrame,sizeof(requestFrame),0,(sockaddr*)&sAddr,length);
   ::close(requestFD);
   if (result!=MYFRAMELENGTH)
   {
      mdcerr<<" NetManager::findServerServer: sent wrong number of bytes: "<<result<<std::endl;
      ::close(answerFD);
      return 0;
   };
   //wait for an answer
   struct timeval tv;
   tv.tv_sec=0;
   tv.tv_usec=1000*250; //0.1 sec
   fd_set fdSet;
   FD_ZERO(&fdSet);
   FD_SET(answerFD,&fdSet);
   result=select(answerFD+1,&fdSet,0,0,&tv);
   if (result<0)
   {
      mdcerr<<" NetManager::findServerServer: select() failed: "<<result<<std::endl;
      mdcerr<<" NetManager::findServerServer: answerFD="<<answerFD<<std::endl;
      perror("select:");
      ::close(answerFD);
      return 0;
   }
   if (result==0)
   {
      mdcerr<<" NetManager::findServerServer: nobody answered "<<std::endl;
      ::close(answerFD);
      return 0;
   }
   mdcerr<<"received offer "<<std::endl;
   struct sockaddr_in addr;
   length=sizeof(sockaddr_in);
   char buf[1024];
   result=recvfrom(answerFD, (char*)buf, 1024, 0, (sockaddr*) &addr,&length);
   if (result!=MYFRAMELENGTH)
   {
      mdcerr<<" NetManager::findServerServer: wrong number of bytes: "<<result<<std::endl;
      ::close(answerFD);
      return 0;
   };
   MyFrameType *frame=(MyFrameType*)(void*)buf;
   //wrong identifier ?
   if (ntohl(frame->id)!=MY_ID)
   {
      mdcerr<<" NetManager::findServerServer: wrong id"<<std::endl;
      ::close(answerFD);
      return 0;
   };

   mdcerr<<"received from "<<inet_ntoa(addr.sin_addr)<<std::endl;

   ::close(answerFD);
   //return the ip of the server server in network byte order
   return addr.sin_addr.s_addr;
}

void NetManager::getListFromServerServer( int address)
{
   mdcerr<<"NetManager::getListFromServerServer"<<std::endl;
   //actually we should never get here in strictMode
   if (m_strictMode) return;
   //open a tcp socket to the serverserver
   int serverServerFD=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (serverServerFD==-1)
      return;
   sockaddr_in addr;
   //we get the address already in network byte order
   addr.sin_addr.s_addr=address;
   addr.sin_family=AF_INET;
   addr.sin_port=htons(m_basePort);
   int result=::connect(serverServerFD,(sockaddr*)&addr,sizeof(addr));
   if (result!=0)
   {
      ::close(serverServerFD);
      return;
   };
   do
   {
      result=readDataFromFD(serverServerFD);
   } while (result==1);
   ::close(serverServerFD);
   processScanResults();
   mdcerr<<"NetManager::getListFromServerServer succeeded"<<std::endl;
}

void NetManager::printState()
{
   std::cerr<<"LAN Information Server Lisa "MYVERSION"\nAlexander Neundorf <neundorf@kde.org>\n";
   std::cerr<<"Reading options from config file: "<<m_usedConfigFileName<<std::endl;
   std::cerr<<"StrictMode: "<<m_strictMode<<std::endl;
   std::cerr<<"ServerServer: "<<m_serverServer<<std::endl;
   std::cerr<<"UseNmblookup: "<<m_useNmblookup<<std::endl;
   std::cerr<<"Pinging: "<<ipRangeStr<<std::endl;
   std::cerr<<"Allowed hosts: "<<validator.validAddresses()<<std::endl;
   std::cerr<<"Broadcasting to: "<<std::ios::hex<<ntohl(m_broadcastAddress)<<std::ios::dec<<std::endl;
   std::cerr<<"Initial update period: "<<m_initialRefreshTime<<" seconds"<<std::endl;
   std::cerr<<"Current update period: "<<m_refreshTime<<" seconds"<<std::endl;
   std::cerr<<"Last update: "<<time(0)-m_lastUpdate<<" seconds over"<<std::endl;
   std::cerr<<"Waiting "<<m_firstWait<<" 1/100th seconds for echo answers on the first try"<<std::endl;
   std::cerr<<"Waiting "<<m_secondWait<<" 1/100th seconds for echo answers on the second try"<<std::endl;
   std::cerr<<"Sending "<<m_maxPings<<" echo requests at once"<<std::endl;
   std::cerr<<"Publishing unnamed hosts: "<<m_deliverUnnamedHosts<<std::endl;
   std::cerr<<"Already served "<<m_serveCount<<" times"<<std::endl;
}

//this one is not used at the moment
/*int NetManager::uptime()
{
   return (time(0)-m_startedAt);
};*/
