/* -------------------------------------------------------------

   dcopinterface.h (part of The KDE Dictionary Client)
   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   KDictDCOPInterface   abstract base class that defines the
                        DCOP interface of Kdict

 ------------------------------------------------------------- */

#ifndef _DCOPINTERFACE_H
#define _DCOPINTERFACE_H

#include <dcopobject.h>
#include <qstringlist.h>

class KDictIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:

  /** Quit Kdict **/
  virtual void quit() = 0;
  virtual void makeActiveWindow() = 0;

  /** define/match a word/phrase **/
  virtual void definePhrase(QString phrase) = 0;
  virtual void matchPhrase(QString phrase) = 0;
  virtual void defineClipboardContent() = 0;
  virtual void matchClipboardContent() = 0;

  /** get info **/
  virtual QStringList getDatabases() = 0;
  virtual QString currentDatabase() = 0;
  virtual QStringList getStrategies() = 0;
  virtual QString currentStrategy() = 0;

  /** set current database/strategy (returns true on success) **/
  virtual bool setDatabase(QString db) = 0;
  virtual bool setStrategy(QString strategy) = 0;

  /** navigate in history **/
  virtual bool historyGoBack() = 0;
  virtual bool historyGoForward() = 0;

};

#endif
