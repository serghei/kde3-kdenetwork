/****************************************************************************
** $Id: safedelete.h 207878 2003-02-19 08:47:26Z pfeiffer $
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org> 
**
****************************************************************************/

#ifndef SAFEDELETE_H
#define SAFEDELETE_H

class KURL;

class SafeDelete
{
public:
    static bool deleteFile( const KURL& url );
};

#endif // SAFEDELETE_H
