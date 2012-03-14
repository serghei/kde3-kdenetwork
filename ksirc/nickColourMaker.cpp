/* This file is part of ksirc
   Copyright (c) 2003 Andrew Stanley-Jones <asj@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id: nickColourMaker.cpp 225919 2003-05-15 02:26:46Z asj $

#include <qstring.h>
#include <qcolor.h>
#include <qregexp.h>
#include <kdebug.h>

#include "nickColourMaker.h"
#include "ksopts.h"

nickColourMaker *nickColourMaker::s_ncm = 0;

nickColourMaker::nickColourMaker()
{
    s_ncm = this;
}

int nickColourMaker::findIdx(QString nick) const
{
    unsigned int value = 0;
    uint i;
    ushort c;

    if(ksopts->nickColourization == false)
        return -1;

    for(i = 0; i < nick.length(); i++){
        c = nick[i].unicode();
	switch(c){
	case '|':
	case '-':
	case '_':
            if(i != 0)
		i = nick.length();
	    else
                value += c;
	    break;
	default:
            value += c;
	}
    }
    value %= ksopts->numColors;
    uint start = value;
    while(ksopts->nickHighlight[value] == false){
	value = (value + 1) % ksopts->numColors;
	if(start == value)
            return -1;

    }


    return value;

}

QColor nickColourMaker::findFg(QString nick) const
{

    int value = findIdx(nick);

    if(value >= 0)
	return ksopts->ircColors[value];
    else if(ksopts->nickForeground.isValid())
        return ksopts->nickForeground;
    else
	return ksopts->textColor;
}

