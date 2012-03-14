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

#ifndef KPF_BYTE_RANGE_H
#define KPF_BYTE_RANGE_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KPF
{
  /**
   * Parse and store an HTTP 'byte range'. 
   * A range consists of a first and, optionally, a last byte.
   * If the last byte is unspecified, it should be assumed to be
   * the last byte of the resource being retrieved.
   * A valid range has first <= last.
   */
  class ByteRange
  {
    public:

      /**
       * Constructs an empty range.
       */
      ByteRange();

      /**
       * Constructs a range with the first byte specified and the last
       * byte set to 'none'.
       */
      ByteRange(ulong first);

      /**
       * Constructs a range with the first and last bytes specified.
       */
      ByteRange(ulong first, ulong last);

      /**
       * @return first byte in range.
       */
      ulong first() const;

      /**
       * @return last byte in range, if specified. Otherwise undefined.
       */
      ulong last()  const;

      /**
       * @return true if last byte was specified.
       */
      bool haveLast() const;

      /**
       * Specify the first byte of the range.
       */
      void setFirst (ulong l);

      /**
       * Specify the last byte of the range. After setting this
       * value, haveLast() will return true.
       */
      void setLast  (ulong l);

      /**
       * @return true if first <= last or last is undefined.
       */
      bool valid() const;

      /**
       * Reset to initial state.
       */
      void clear();

    private:

      ulong first_;
      ulong last_;
      bool  haveLast_;
  };

  /**
   * Encapsulates a list of ByteRange.
   */
  class ByteRangeList : public QValueList<ByteRange>
  {
    public:

      ByteRangeList();

      /**
       * Contructs a ByteRangeList from a string, which is parsed.
       * @param protocol specifies the HTTP protocol which should
       * be assumed whilst parsing.
       */
      ByteRangeList(const QString &, float protocol);

      /**
       * Parses a byte range represented as a string and, if successful,
       * appends the resultant ByteRange object to this list.
       */
      void addByteRange(const QString &);
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
