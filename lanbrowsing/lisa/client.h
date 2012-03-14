/*    client.h
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

#ifndef CLIENT_H
#define CLIENT_H

#include <time.h>

class NetManager;

class Client
{
   public:
      Client(NetManager* parent, int socketFD, int closeOnDelete=1);
      Client();
      Client(const Client& c);
      ~Client();
      int tryToGetInfo();
      int done() {return m_done;};
      int fd() {return m_fd;};
      void read();
      void close();
      time_t age() {return time(0)-m_creationTime;};
   private:
      NetManager *parentServer;
      int m_fd;
      //if something goes wrong close the connection after a timeout
      time_t m_creationTime;
      int m_done;
      int m_closeOnDelete;
};

#endif
