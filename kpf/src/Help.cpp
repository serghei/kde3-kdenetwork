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

#include "Help.h"
#include <klocale.h>

#include <dnssd/servicebrowser.h>

namespace KPF
{
  namespace HelpText
  {

  QString getServerNameHelp()
  {
    switch(DNSSD::ServiceBrowser::isAvailable()) {
      case DNSSD::ServiceBrowser::Working:
        return i18n("<p>Specify the name that will be used when announcing"
                    " this server on network.</p>");
      case DNSSD::ServiceBrowser::Stopped:
        return i18n("<p>The Zeroconf daemon is not running. See the Handbook"
                    " for more information.<br/>"
                    "Other users will not see this system when browsing"
                    " the network via zeroconf, but sharing will still work.</p>");
      case DNSSD::ServiceBrowser::Unsupported:
        return i18n("<p>Zeroconf support is not available in this version of KDE."
                    " See the Handbook for more information.<br/>"
                    "Other users will not see this system when browsing"
                    " the network via zeroconf, but sharing will still work.</p>");
      default:
        return i18n("<p>Unknown error with Zeroconf.<br/>"
                     "Other users will not see this system when browsing"
                     " the network via zeroconf, but sharing will still work.</p>");
    }
  }


  } // End namespace HelpText

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
