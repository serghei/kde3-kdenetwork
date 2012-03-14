/* This file is part of the KDE project
   Copyright (C) 2003 Andrew Stanley-Jones

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GPL.
*/
#ifndef __nickcolourmaker_h__
#define __nickcolourmaker_h__

#include <qdict.h>
#include <qstring.h>
#include <qmap.h>
#include <qcolor.h>
#include "ksopts.h"

struct nickColourInfo
{
    nickColourInfo(int _fg) { fg = _fg; }
    int fg;
};

// Helper class to parse IRC colour nicks in the
// irc channel and the nick list
class nickColourMaker
{
public:
    nickColourMaker();

    int findIdx(QString nick) const;
    QColor findFg(QString nick) const;
    QColor operator[]( QString nick ) const
    {
        return findFg(nick);
    }

    static nickColourMaker *colourMaker() { return s_ncm; }


private:
    static nickColourMaker *s_ncm;
    QDict<nickColourInfo> m_nicks;
};

#endif
