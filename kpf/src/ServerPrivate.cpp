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

#include "Defines.h"
#include "ServerPrivate.h"
#include "Defaults.h"

namespace KPF
{
  ulong Server::Private::ID = 0L;

  Server::Private::Private()
    : socket                    (0, "KPF::Server::Private.socket"),
      state                     (WaitingForRequest),
      bytesWritten              (0L),
      headerBytesLeft           (0L),
      fileBytesLeft             (0L),
      dataRead                  (0L),
      followSymlinks            (Config::DefaultFollowSymlinks),
      generateDirectoryListings (false),
      requestCount              (0),
      id                        (ID++)
  {
    // Empty.
  }

  Server::Private::~Private()
  {
    // Empty.
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
