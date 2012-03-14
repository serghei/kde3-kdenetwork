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

#ifndef KPF_INTERFACE_H
#define KPF_INTERFACE_H

#include <dcopobject.h>

#include "Defaults.h"

/**
 * DCOP interface to kpf.
 */
class KPFInterface : virtual public DCOPObject
{
  K_DCOP

  public:

    KPFInterface  ();
    ~KPFInterface ();

  k_dcop:

    /**
     * @return list of root directories used by WebServer objects.
     */
    virtual QStringList serverRootList();

    virtual bool createServer
      (
       QString  root,
       uint     port,
       uint     bandwidthLimit  = KPF::Config::DefaultBandwidthLimit,
       uint     connectionLimit = KPF::Config::DefaultConnectionLimit,
       bool     followSymlinks  = KPF::Config::DefaultFollowSymlinks
      );

    virtual bool        disableServer     (QString root);
    virtual bool        restartServer     (QString root);
    virtual bool        reconfigureServer (QString root);
    virtual bool        pauseServer       (QString root);
    virtual bool        unpauseServer     (QString root);
    virtual bool        serverPaused      (QString root);
};

#endif
// vim:ts=2:sw=2:tw=78:et
