/****************************************************************************
** $Id: kget_iface.h 388164 2005-02-10 21:24:50Z ratz $
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KGET_IFACE_H
#define KGET_IFACE_H

#include <dcopobject.h>
#include <kurl.h>

class KGetIface : public DCOPObject
{
    K_DCOP

protected:
    KGetIface( QCString objId ) : DCOPObject( objId ) {}

k_dcop:
    /**
     * @param src The urls to download
     * @param destDir The destination direction or QString::null if unspecified
     */
    virtual ASYNC addTransfers( const KURL::List& src, const QString& destDir = QString::null ) = 0;
    
    virtual bool isDropTargetVisible() const = 0;
    
    virtual void setDropTargetVisible( bool setVisible ) = 0;

	virtual void setOfflineMode( bool offline ) = 0;

	virtual bool isOfflineMode() const = 0;
};

#endif // KGET_IFACE_H
