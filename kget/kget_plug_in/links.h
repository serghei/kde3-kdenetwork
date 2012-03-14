/****************************************************************************
** $Id: links.h 209342 2003-02-25 15:43:03Z pfeiffer $
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef LINKS_H
#define LINKS_H

#include <dom/dom_element.h>

#include <kurl.h>

class LinkItem
{
public:
    LinkItem( DOM::Element link );

    KURL url;
    QString icon;
    QString text;
    QString mimeType;
    
    bool isValid() const { return m_valid; }
    
private:
    bool m_valid : 1;
};


#endif // LINKS_H
