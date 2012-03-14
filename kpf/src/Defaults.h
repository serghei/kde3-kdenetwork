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

#ifndef KPF_DEFAULTS_H
#define KPF_DEFAULTS_H

#include <qstring.h>

namespace KPF
{
  /**
   * Used to provide a single point of access to config defaults and key names.
   */
  namespace Config
  {
    extern const uint  DefaultListenPort;
    extern const uint  DefaultBandwidthLimit;
    extern const uint  DefaultConnectionLimit;
    extern const bool  DefaultFollowSymlinks;
    extern const bool  DefaultCustomErrors;
    extern const bool  DefaultPaused;

    enum Option
    {
      ServerRootList,
      GroupPrefix,
      ListenPort,
      BandwidthLimit,
      ConnectionLimit,
      FollowSymlinks,
      CustomErrors,
      Paused,
      ServerName
    };

    /**
     * Name to be used for configuration file.
     */
    QString name();

    /**
     * Config key to use when accessing option.
     */
    QString key(Option);

  } // End namespace Config

} // End namespace KPF

#endif // KPF_DEFAULTS_H
// vim:ts=2:sw=2:tw=78:et
