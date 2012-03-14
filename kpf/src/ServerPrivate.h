/*
  KPF - Public fileserver for KDE

  Copyright 2001 Rik Hemsley (rikkus) <rik@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef KPF_SERVER_PRIVATE_H
#define KPF_SERVER_PRIVATE_H

#include <qcstring.h>
#include <qtimer.h>

#include "ServerSocket.h"
#include "Server.h"
#include "Request.h"
#include "Response.h"
#include "Resource.h"

namespace KPF
{
  /**
   * Data for Server class. Kept here to speed up recompilation.
   */
  class Server::Private
  {
    public:

      Private();
      ~Private();

      // Order dependency
      ServerSocket  socket;
      Server::State state;
      ulong         bytesWritten;
      ulong         headerBytesLeft;
      ulong         fileBytesLeft;
      ulong         dataRead;
      bool          followSymlinks;
      bool          generateDirectoryListings;
      uint          requestCount;
      // End order dependency

      QString           dir;
      Request           request;
      Response          response;
      Resource          resource;
      QStringList       incomingHeaderLineBuffer;
      QStringList       incomingLineBuffer;
      QDateTime         birth;
      QDateTime         death;
      QCString          outgoingHeaderBuffer;
      QTimer            idleTimer;
      QTimer            readTimer;
      ulong             id;
      static ulong      ID;
  };

} // End namespace KPF

#endif // KPF_SERVER_PRIVATE_H
// vim:ts=2:sw=2:tw=78:et
