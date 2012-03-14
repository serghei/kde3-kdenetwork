/* This file is part of ksirc
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

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

// $Id: ksparser.cpp 393645 2005-02-27 22:05:39Z staikos $

#include <qstring.h>
#include <qcolor.h>
#include <qregexp.h>

#include <kdebug.h>

#include "ksopts.h"
#include "ksparser.h"

QString KSParser::parse( const QString &message )
{
    QString res;
    m_tags.clear();
    m_attributes.clear();
    m_beeped = false;

    for (unsigned int i = 0; i < message.length();)
    {
        QChar ch = message[i++];
        if (ch.latin1() == 0x03 || ch == '~' && i < message.length())
        {
            QChar next = message[ i++ ];
            if (next.latin1() >= 0x30 && next.latin1() <= 0x39)
            {
                int fg = -1, len;
                int bg = -1;
                QRegExp colorRex("^[0-9]+");
                if (colorRex.search(message.mid(--i)) >= 0)
                {
                    len = colorRex.matchedLength();
                    fg = message.mid(i, len).toInt();
                    i += len;
                }
                if (message[i] == ',')
                {
                    if (colorRex.search(message.mid(++i)) >= 0)
                    {
                        len = colorRex.matchedLength();
                        bg = message.mid(i, len).toInt();
                        i += len;
                    }
		}
                QString at;
                QColor c = ircColor(fg);
		if ( c.isValid() ) {
		    at += QString( "color=\"%1\" " ).arg( c.name() );
		}

                c = ircColor(bg);
		if ( c.isValid() ) {
		    at += QString( "bgcolor=\"%1\" " ).arg( c.name() );
		}
		if(at.length() > 0){
                    res += pushTag( "font", at );
		}

            }
	    else if (ch.latin1() == 0x03) {
		res += popTag( "font" );
		/*
		 * We moved it forward looking for the number, back
		 * up or we miss a character.  Don't do
		 * res += next since we need to actaully parse that one
                 */
                --i;
	    }
            else if (ch == '~')
            {
                switch (next)
                {
		case 'c':
                    res += popTag( "font" );
                    break;
		case 'C':
                    res += popAll();
                    break;
		case 'r':
		    res += toggleTag( "r" );
                    break;
                case 's': break;
                case 'b':
                    res += toggleTag( "b" );
                    break;
                case 'g':
                    m_beeped = true;
                    break;
                case 'u':
                    res += toggleTag( "u" );
                    break;
                case 'n':
                    res += toggleTag( "font", QString( "color=\"%1\"" ).arg( ksopts->nickForeground.name() ) );
                    break;
                case 'o':
                    res += pushTag( "font", QString( "color=\"%1\"" ).arg( ksopts->msgContainNick.name() ) );
		    break;
		case '#':
		    res += pushTag( "font", QString( "color=\"#%1\"" ).arg( message.mid(i, 6) ));
		    i+=6;
                    break;
                case '~':
		    res += ch;
                    break;
		default:
		    res += ch;
		    res += next;
                    break;
                }
            }
        }
        else
            res += ch;

    }

    res.append( popAll() );

    return res;
}

QString KSParser::pushTag(const QString &tag, const QString &attributes)
{
    QString res;
    m_tags.push(tag);
    if (!m_attributes.contains(tag))
        m_attributes.insert(tag, attributes);
    else if (!attributes.isEmpty())
        m_attributes.replace(tag, attributes);
    res.append("<" + tag);
    if (!m_attributes[tag].isEmpty())
        res.append(" " + m_attributes[tag]);
    return res + ">";
}

QString KSParser::popTag(const QString &tag)
{
    if (!m_tags.contains(tag))
        return QString::null;

    QString res;
    QValueStack<QString> savedTags;
    while (m_tags.top() != tag)
    {
        savedTags.push(m_tags.pop());
        res.append("</" + savedTags.top() + ">");
    }
    res.append("</" + m_tags.pop() + ">");
    m_attributes.remove(tag);
    while (!savedTags.isEmpty())
        res.append(pushTag(savedTags.pop()));
    return res;
}

QString KSParser::toggleTag(const QString &tag, const QString &attributes)
{
    return m_attributes.contains(tag) ? popTag(tag) : pushTag(tag, attributes);
}

QString KSParser::popAll()
{
    QString res;
    while (!m_tags.isEmpty())
        res.append("</" + m_tags.pop() + ">");
    m_attributes.clear();
    return res;
}

QColor KSParser::ircColor(int code)
{
    if (code >= 0 && code < 16)
        return ksopts->ircColors[code];
    return QColor();
}
