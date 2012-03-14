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

#include <qstringlist.h>

#include "Defines.h"
#include "ByteRange.h"

namespace KPF
{
  ByteRange::ByteRange()
    : first_    (0L),
      last_     (0L),
      haveLast_ (false)
  {
    // Empty.
  }

  ByteRange::ByteRange(ulong first)
    : first_    (first),
      last_     (0L),
      haveLast_ (false)
  {
    // Empty.
  }


  ByteRange::ByteRange(ulong first, ulong last)
    : first_    (first),
      last_     (last),
      haveLast_ (true)
  {
  }

    ulong
  ByteRange::first() const
  {
    return first_;
  }

    ulong
  ByteRange::last() const
  {
    return last_;
  }

    bool
  ByteRange::haveLast() const
  {
    return haveLast_;
  }

    void
  ByteRange::setFirst(ulong l)
  {
    first_ = l;
  }

    void
  ByteRange::setLast(ulong l)
  {
    last_ = l;
    haveLast_ = true;
  }

    bool
  ByteRange::valid() const
  {
    return haveLast_ ? (first_ < last_) : true;
  }

    void
  ByteRange::clear()
  {
    first_ = last_ = 0L;
    haveLast_ = false;
  }

  ByteRangeList::ByteRangeList()
  {
    // Empty.
  }

  ByteRangeList::ByteRangeList(const QString & _s, float /* protocol */)
  {
    kpfDebug << "ByteRangeList parsing `" << _s << "'" << endl;

    // Hey, parsing time :)

    QString s(_s);

    if ("bytes=" == s.left(6))
    {
      s.remove(0, 6);
      s = s.stripWhiteSpace();
    }

    QStringList byteRangeSpecList(QStringList::split(',', s));

    QStringList::ConstIterator it;

    for (it = byteRangeSpecList.begin(); it != byteRangeSpecList.end(); ++it)
      addByteRange(*it);
  }

    void
  ByteRangeList::addByteRange(const QString & s)
  {
    kpfDebug << "addByteRange(" << s << ")" << endl;

    int dashPos = s.find('-');

    if (-1 == dashPos)
    {
      kpfDebug << "No dash" << endl;
      return;
    }

    QString firstByte(s.left(dashPos).stripWhiteSpace());

    QString lastByte(s.mid(dashPos + 1).stripWhiteSpace());

    ulong first;

    if (firstByte.isEmpty())
      first = 0L;
    else
      first = firstByte.toULong();

    ulong last;

    bool haveLast = !lastByte.isEmpty();

    if (haveLast)
      last      = lastByte.toULong();
    else
      last      = 0L;

    if (haveLast)
    {
      if (first < last)
      {
        kpfDebug << "range: " << first << "d - " << last << "d" << endl;
        append(ByteRange(first, last));
      }
    }
    else
    {
      kpfDebug << "range: " << first << "d - end" << endl;
      append(ByteRange(first));
    }
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
