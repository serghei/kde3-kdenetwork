/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __boundscheckingarray__
#define __boundscheckingarray__

#include <assert.h>
#include <qvaluevector.h>

/* I wish QValueVector would check bounds upon extraction.. */

template <typename T, const unsigned int size>
class BoundsCheckingArray
{
public:
    BoundsCheckingArray() {}
    BoundsCheckingArray( const BoundsCheckingArray<T, size> &rhs )
    { (*this) = rhs; }
    BoundsCheckingArray &operator=( const BoundsCheckingArray<T, size> &rhs )
    {
        unsigned int i = 0;
        for (; i < size; ++i )
            m_data[ i ] = rhs.m_data[ i ];
        return *this;
    }

    T &operator[]( unsigned int index )
    {
        assert( index < size );
        return m_data[ index ];
    }

    const T &operator[]( unsigned int index ) const
    {
        assert( index < size );
        return m_data[ index ];
    }

    QValueVector<T> toValueVector() const
    {
        QValueVector<T> vector( size );
        for ( unsigned int i = 0; i < size; ++i )
            vector[ i ] = m_data[ i ];
        return vector;
    }

private:
    T m_data[ size ];
};

#endif

