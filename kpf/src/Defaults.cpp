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
#include "Defaults.h"

namespace KPF
{
  namespace Config
  {
    const uint  DefaultListenPort      = 8001;
    const uint  DefaultBandwidthLimit  = 4;
    const uint  DefaultConnectionLimit = 64;
    const bool  DefaultFollowSymlinks  = false;
    const bool  DefaultCustomErrors    = false;
    const bool  DefaultPaused          = false;
    const QString& DefaultServername   = QString::null;

    static const char       Name[]                    = "kpfappletrc";
    static const char       KeyServerRootList[]       = "ServerRootList";
    static const char       KeyGroupPrefix[]          = "Server_";
    static const char       KeyListenPort[]           = "ListenPort";
    static const char       KeyBandwidthLimit[]       = "BandwidthLimit";
    static const char       KeyConnectionLimit[]      = "ConnectionLimit";
    static const char       KeyFollowSymlinks[]       = "FollowSymlinks";
    static const char       KeyCustomErrors[]         = "CustomErrors";
    static const char       KeyPaused[]               = "Paused";
    static const char       KeyServerName[]           = "ServerName";

    QString name()
    {
      return QString::fromUtf8(Name);
    }

    QString key(Option o)
    {
      switch (o)
      {
        case ServerRootList:
          return QString::fromUtf8(KeyServerRootList);

        case GroupPrefix:
          return QString::fromUtf8(KeyGroupPrefix);

        case ListenPort:
          return QString::fromUtf8(KeyListenPort);

        case BandwidthLimit:
          return QString::fromUtf8(KeyBandwidthLimit);

        case ConnectionLimit:
          return QString::fromUtf8(KeyConnectionLimit);

        case FollowSymlinks:
          return QString::fromUtf8(KeyFollowSymlinks);

        case CustomErrors:
          return QString::fromUtf8(KeyCustomErrors);
	 
	case Paused:
	  return QString::fromUtf8(KeyPaused);

 	case ServerName:
 	  return QString::fromUtf8(KeyServerName);

        /* default intentionally left out to have the compiler generate
         * warnings in case we add values to the enumeration but forget
         * to extend this switch.
         */
      }
      return QString::null;
    }
  } // End namespace Config

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
