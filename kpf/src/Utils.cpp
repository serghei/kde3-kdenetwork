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

#include <ctime>
#include <clocale>

#include <qfile.h>
#include <qregexp.h>

#include <klocale.h>

#include "Defines.h"
#include "Utils.h"

namespace KPF
{
  static bool dateInitDone = false;

  static QStringList monthList;

  static const char rfc1123Format[] = "%a, %d %b %Y %H:%M:%S GMT";

  static time_t qDateTimeToTimeT(const QDateTime & t)
  {
    struct tm tempTm;

    tempTm.tm_year  = t.date().year();
    tempTm.tm_mon   = t.date().month();
    tempTm.tm_mday  = t.date().day();
    tempTm.tm_hour  = t.time().hour();
    tempTm.tm_min   = t.time().minute();
    tempTm.tm_sec   = t.time().second();
    tempTm.tm_isdst = -1;

    // Fix up differences between QDateTime and tm.

    tempTm.tm_year -= 1900;

    --tempTm.tm_mon;

    return ::mktime(&tempTm);
  }

    QDateTime
  toGMT(const QDateTime & dt)
  {
    time_t dtAsTimeT = qDateTimeToTimeT(dt);

    struct tm * dtAsGmTm = ::gmtime(&dtAsTimeT);

    if (0 == dtAsGmTm)
      return QDateTime();

    time_t dtAsGmTimeT = ::mktime(dtAsGmTm);

    QDateTime ret;

    ret.setTime_t(dtAsGmTimeT);

    return ret;
  }

  void dateInit()
  {
    if (dateInitDone)
      return;

    dateInitDone = true;

    monthList
      << "Jan"
      << "Feb"
      << "Mar"
      << "Apr"
      << "May"
      << "Jun"
      << "Jul"
      << "Aug"
      << "Sep"
      << "Oct"
      << "Nov"
      << "Dec"
      ;
  }

  QString dateString()
  {
    return dateString(QDateTime::currentDateTime());
  }


  QString dateString(const QDateTime & t)
  {
    time_t asTimeT = qDateTimeToTimeT(t);

    struct tm * asTm = ::gmtime(&asTimeT);

    if (0 == asTm)
    {
      kpfDebug << "::gmtime() failed" << endl;
      return QString::null;
    }

    asTm->tm_isdst = -1;

    const int len = 128;

    char buf[len];

    // Ensure we use locale "C" for strftime.

    QCString oldLC_ALL  = ::strdup(::setlocale(LC_TIME, "C"));
    QCString oldLC_TIME = ::strdup(::setlocale(LC_ALL, "C"));
    {
      ::strftime(static_cast<char *>(buf), len, rfc1123Format, asTm);
    }
    ::setlocale(LC_TIME,  oldLC_TIME.data());
    ::setlocale(LC_ALL,   oldLC_ALL.data());

    return QString::fromUtf8(buf);
  }

    bool
  parseDate(const QString & s, QDateTime & dt)
  {
    dateInit();

//    kpfDebug << "Parsing date `" << s << "'" << endl;

    QStringList l(QStringList::split(' ', s));

    switch (l.count())
    {
      case 4:
//        kpfDebug << "Date type is RFC 850" << endl;
        return parseDateRFC850(l, dt);
        break;
      case 5:
//        kpfDebug << "Date type is asctime" << endl;
        return parseDateAscTime(l, dt);
        break;
      case 6:
//        kpfDebug << "Date type is RFC1123" << endl;
        return parseDateRFC1123(l, dt);
        break;
      default:
//        kpfDebug << "Date type is unknown" << endl;
        return false;
        break;
    }
    
    return false;
  }

    bool
  parseDateRFC1123(const QStringList & l, QDateTime & dt)
  {
    if ("GMT" != l[5])
      return false;

    uint day(l[1].toUInt());

    bool haveMonth = false;
    uint month = 0;

    QStringList::ConstIterator it;

    for (it = monthList.begin(); it != monthList.end(); ++it)
    {
      if (*it == l[2])
      {
        haveMonth = true;
        break;
      }

      ++month;
    }

    if (!haveMonth)
      return false;

    uint year(l[3].toUInt());

    QStringList timeTokenList(QStringList::split(':', l[4]));

    if (3 != timeTokenList.count())
      return false;

    uint hours    (timeTokenList[0].toUInt());
    uint minutes  (timeTokenList[1].toUInt());
    uint seconds  (timeTokenList[2].toUInt());

    dt.setDate(QDate(year, month + 1, day));
    dt.setTime(QTime(hours, minutes, seconds));

    return dt.isValid();
  }

    bool
  parseDateRFC850(const QStringList & l, QDateTime & dt)
  {
    if ("GMT" != l[3])
      return false;

    QStringList dateTokenList(QStringList::split('-', l[1]));

    if (3 != dateTokenList.count())
      return false;

    uint day(dateTokenList[0].toUInt());

    bool haveMonth = false;
    uint month = 0;

    QStringList::ConstIterator it;

    for (it = monthList.begin(); it != monthList.end(); ++it)
    {
      if (*it == dateTokenList[1])
      {
        haveMonth = true;
        break;
      }

      ++month;
    }

    if (!haveMonth)
      return false;

    uint year(dateTokenList[2].toUInt());

    if (year < 50)
      year += 2000;
    else if (year < 100)
      year += 1900;

    QStringList timeTokenList(QStringList::split(':', l[2]));

    if (3 != timeTokenList.count())
      return false;

    uint hours    (timeTokenList[0].toUInt());
    uint minutes  (timeTokenList[1].toUInt());
    uint seconds  (timeTokenList[2].toUInt());

    dt.setDate(QDate(year, month + 1, day));
    dt.setTime(QTime(hours, minutes, seconds));

    return dt.isValid();
  }

     bool
  parseDateAscTime(const QStringList & l, QDateTime & dt)
  {
    bool haveMonth = false;
    uint month = 0;

    QStringList::ConstIterator it;

    for (it = monthList.begin(); it != monthList.end(); ++it)
    {
      if (*it == l[1])
      {
        haveMonth = true;
        break;
      }

      ++month;
    }

    if (!haveMonth)
      return false;

    uint day(l[2].toUInt());

    QStringList timeTokenList(QStringList::split(':', l[3]));

    if (3 != timeTokenList.count())
      return false;

    uint hours    (timeTokenList[0].toUInt());
    uint minutes  (timeTokenList[1].toUInt());
    uint seconds  (timeTokenList[2].toUInt());

    uint year(l[4].toUInt());

    dt.setDate(QDate(year, month + 1, day));
    dt.setTime(QTime(hours, minutes, seconds));

    return dt.isValid();
  }

     QString
  translatedResponseName(uint code)
  {
    QString s;

    switch (code)
    {
      case 200:
        s = i18n("OK");
        break;
      case 206:
        s = i18n("Partial content");
        break;
      case 304:
        s = i18n("Not modified");
        break;
      case 400:
        s = i18n("Bad request");
        break;
      case 403:
        s = i18n("Forbidden");
        break;
      case 404:
        s = i18n("Not found");
        break;
      case 412:
        s = i18n("Precondition failed");
        break;
      case 416:
        s = i18n("Bad range");
        break;
      case 500:
        s = i18n("Internal error");
        break;
      case 501:
        s = i18n("Not implemented");
        break;
      case 505:
        s = i18n("HTTP version not supported");
        break;
      default:
        s = i18n("Unknown");
        break;
    }

    return s;
  }

     QString
  responseName(uint code)
  {
    QString s;

    switch (code)
    {
      case 200:
        s = "OK";
        break;
      case 206:
        s = "Partial content";
        break;
      case 304:
        s = "Not modified";
        break;
      case 400:
        s = "Bad request";
        break;
      case 403:
        s = "Forbidden";
        break;
      case 404:
        s = "Not found";
        break;
      case 412:
        s = "Precondition failed";
        break;
      case 416:
        s = "Bad range";
        break;
      case 500:
        s = "Internal error";
        break;
      case 501:
        s = "Not implemented";
        break;
      case 505:
        s = "HTTP version not supported";
        break;
      default:
        s = "Unknown";
        break;
    }

    return s;
  }


} // End namespace KPF


// vim:ts=2:sw=2:tw=78:et
