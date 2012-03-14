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

#ifndef KPF_UTILS_H
#define KPF_UTILS_H

#include <qstringlist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qfileinfo.h>

/**
 * Used to keep all parts of the kpf application out of the global namespace.
 */
namespace KPF
{
  /**
   * Safe version of QMIN
   * @return minimum of a and b.
   */
  template<class T> T min(T a, T b) { return a < b ? a : b; }

  /**
   * Safe version of QMAX
   * @return minimum of a and b.
   */
  template<class T> T max(T a, T b) { return b < a ? a : b; }

  /**
   * @return the current date and time as an HTTP/1.1 compliant date string.
   */
  QString dateString();

  /**
   * @return the passed QDateTime as an HTTP/1.1 compliant date string.
   */
  QString dateString(const QDateTime & dt);

  /**
   * Parse an HTTP/1.1 date to a QDateTime.
   * @param ret the QDateTime representation (result of parsing)
   * @return true if the date is an an acceptable format.
   */
  bool parseDate(const QString &, QDateTime & ret);

  /**
   * Parse an RFC 1123 format date to a QDateTime. Usually called by
   * @ref parseDate.
   */
  bool parseDateRFC1123(const QStringList &, QDateTime &);

  /**
   * Parse an RFC 850 format date to a QDateTime. Usually called by
   * @ref parseDate.
   */
  bool parseDateRFC850(const QStringList &, QDateTime &);

  /**
   * Parse an asctime(3) format date to a QDateTime. Usually called by
   * @ref parseDate.
   */
  bool parseDateAscTime(const QStringList &, QDateTime &);

  /**
   * @return i18n(HTTP response message for code)
   */
  QString translatedResponseName(uint code);

  /**
   * @return HTTP response message for code
   */
  QString responseName(uint code);

  /**
   * @return the passed QDateTime converted GMT, honouring daylight
   * saving time if necessary.
   */
  QDateTime toGMT(const QDateTime &);

  /**
   * Unquote hex quoted chars in string.
   */
  QString unquote(const QString &);

  /**
   * Quote naughty chars in %xx format.
   */
  QString quote(const QString &);

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
