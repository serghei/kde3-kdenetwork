/*    netmanager.h
 *
 *    Copyright (c) 1998, 1999, Alexander Neundorf
 *    alexander.neundorf@rz.tu-ilmenau.de
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

#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <list>

#include "netscanner.h"
#include "client.h"
#include "mystring.h"
#include "configfile.h"
#include "lisadefines.h"

struct MyFrameType
{
   int id;
   //keep some room for later extensions
   int unused1;
   int unused2;
   int unused3;
};

#define MYFRAMELENGTH 16

class NetManager:public NetScanner
{
   public:
      NetManager(int& rawSocketFD, int portToUse, MyString configFile, int configStyle=UNIXCONFIGSTYLE, int strictMode=0);
      ~NetManager();

      int prepare();
      int run();
      int writeDataToFD(int fd, int serverServer);
      
      void readConfig();
      void configure(Config& config);
      int isBeingScanned()            {return m_isBeingScanned;};
      int isInformed()            {return m_isInformed;};
      //int uptime();
      void printState();
   protected:
      int m_listenFD;
      int m_bcFD;
      int m_basePort;

      int m_pipeFD;
      char *m_receiveBuffer;
      size_t m_receivedBytes;

      struct timeval tv;
      pid_t m_childPid;
      time_t m_lastUpdate;
      time_t m_startedAt;
      
      int m_isInformed;
      int m_isBeingScanned;
      int m_firstRun;
      int m_serverServer;
      int m_servedThisPeriod;

      int m_serveCount;
      int m_refreshTime;
      int m_initialRefreshTime;
      int m_increasedRefreshTime;
      int m_broadcastAddress;
      std::list<Client> clients;
      struct client_is_done : std::unary_function<Client, bool>
      {
         bool operator() (Client& c)
         {
            return (c.done() != 0 || c.age() > 30);
         }
      };
      
      int getMaxFD();
      void generateFDset(fd_set *tmpFDs);
      //returns 0 on timeout, otherwise 1
      int waitForSomethingToHappen(fd_set *tmpFDs);
      void scan();
      void addClient(int socketFD);
      void checkClientsAndPipes(fd_set *tmpFDs);
      int readDataFromFD(int fd);
      int processScanResults();
      int findServerServer();
      void getListFromServerServer(int address);
      void enableServerServer(int on);
      void serveAndClean();
      void answerBroadcast();
      void adjustRefreshTime(int serverServer);
      
      MyString m_extraConfigFile;
      int m_configStyle;
      MyString getConfigFileName();
      MyString m_usedConfigFileName;
};
#endif
