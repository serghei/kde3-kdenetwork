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

#include <climits> // For ULONG_MAX

#include <qregexp.h>
#include <kurl.h>

#include "Defines.h"
#include "Utils.h"
#include "Request.h"

namespace KPF
{
  Request::Request()
    : protocolMajor_          (0),
      protocolMinor_          (9),
      method_                 (Unsupported),
      haveHost_               (false),
      haveIfModifiedSince_    (false),
      haveIfUnmodifiedSince_  (false),
      expectContinue_         (false),
      haveRange_              (false),
      persist_                (false)
  {
  }

  Request::~Request()
  {
  }

    void
  Request::parseHeaders(const QStringList & buf)
  {
    for (QStringList::ConstIterator it(buf.begin()); it != buf.end(); ++it)
    {
      QString line(*it);

      int colonPos = line.find(':');

      if (-1 != colonPos)
      {
        QString name(line.left(colonPos).stripWhiteSpace().lower());
        QString value(line.mid(colonPos + 1).stripWhiteSpace());
        handleHeader(name, value);
      }
    }
  }

    void
  Request::handleHeader(const QString & name, const QString & value)
  {
    if ("host" == name)
    {
      setHost(value);
    }
    if ("range" == name)
    {
      setRange(value);
    }
    else if ("if-modified-since" == name)
    {
      QDateTime dt;

      if (parseDate(value, dt))
        setIfModifiedSince(dt);
    }
    else if ("if-unmodified-since" == name)
    {
      QDateTime dt;

      if (parseDate(value, dt))
        setIfUnmodifiedSince(dt);
    }
    else if ("connection" == name)
    {
      QString v(value.lower());

      if ("keep-alive" == v)
      {
        setPersist(true);
      }
      else if ("close" == v)
      {
        setPersist(false);
      }
    }
  }

    void
  Request::setProtocol(const QString & _s)
  {
    QString s(_s);

    s.remove(0, 5);

    int dotPos = s.find('.');

    if (-1 != dotPos)
    {
      protocolMajor_ = s.left(dotPos).toUInt();
      protocolMinor_ = s.mid(dotPos + 1).toUInt();
    }
  }

    void
  Request::setProtocol(uint major, uint minor)
  {
    protocolMajor_ = major;
    protocolMinor_ = minor;
  }

    void
  Request::setMethod(const QString & s)
  {
    if ("GET" == s)
      method_ = Get;
    else if ("HEAD" == s)
      method_ = Head;
    else
      method_ = Unsupported;
  }

    void
  Request::setMethod(Method m)
  {
    method_ = m;
  }

    void
  Request::setPath(const QString & s)
  {
    KURL p(s);
    path_ = clean(p.path());

#if 0
    if ('/' == path_.at(path_.length() - 1))
    {
      path_.append("index.html");
    }
#endif
  }

    void
  Request::setHost(const QString & s)
  {
    host_ = s;
    haveHost_ = true;
  }

    void
  Request::setIfModifiedSince(const QDateTime & dt)
  {
    ifModifiedSince_ = dt;
    haveIfModifiedSince_ = true;
  }

    void
  Request::setIfUnmodifiedSince(const QDateTime & dt)
  {
    ifUnmodifiedSince_ = dt;
    haveIfUnmodifiedSince_ = true;
  }

    uint
  Request::protocolMajor() const
  {
    return protocolMajor_;
  }

    uint
  Request::protocolMinor() const
  {
    return protocolMinor_;
  }

    float
  Request::protocol() const
  {
    return (float(protocolMajor_) + float(protocolMinor_) / 10.0);
  }

    Request::Method
  Request::method() const
  {
    return method_;
  }

    bool
  Request::haveHost() const
  {
    return haveHost_;
  }

    bool
  Request::haveIfModifiedSince() const
  {
    return haveIfModifiedSince_;
  }

    bool
  Request::haveIfUnmodifiedSince() const
  {
    return haveIfUnmodifiedSince_;
  }

    QString
  Request::path() const
  {
    return path_;
  }

    QString
  Request::host() const
  {
    return host_;
  }

    QDateTime
  Request::ifModifiedSince() const
  {
    return ifModifiedSince_;
  }

    QDateTime
  Request::ifUnmodifiedSince() const
  {
    return ifUnmodifiedSince_;
  }

    QCString
  Request::protocolString() const
  {
    QCString s("HTTP/");
    s += QCString().setNum(protocolMajor_);
    s += '.';
    s += QCString().setNum(protocolMinor_);
    return s;
  }

    void
  Request::setExpectContinue(bool b)
  {
    expectContinue_ = b;
  }

    bool
  Request::expectContinue() const
  {
    return expectContinue_;
  }

    void
  Request::setRange(const QString & s)
  {
    kpfDebug << "Request::setRange(`" << s << "')" << endl;

    haveRange_ = true;

    ByteRangeList l(s, protocol());

    ulong first     (ULONG_MAX);
    ulong last      (0L);
    bool  haveLast  (false);

    for (ByteRangeList::ConstIterator it(l.begin()); it != l.end(); ++it)
    {
      ByteRange r(*it);
      first = min(r.first(), first);

      if (r.haveLast())
      {
        haveLast = true;
        last = max(r.last(), last);
      }
    }

    kpfDebug << "Request::setRange(): first == " << first << "d" << endl;

    range_.setFirst(first);

    if (haveLast)
    {
      kpfDebug << "Request::setRange(): last == " << last << "d" << endl;
      range_.setLast(last);
    }
    kpfDebug << "Request::setRange(): no last" << endl;
  }

    ByteRange
  Request::range() const
  {
    return range_;
  }

    bool
  Request::haveRange() const
  {
    return haveRange_;
  }

    void
  Request::setPersist(bool b)
  {
    if (protocol() > 1.0) // Bad, but makes wget work.
    {
      persist_ = b;
    }
  }

    bool
  Request::persist() const
  {
    return persist_;
  }

    void
  Request::clear()
  {
    protocolMajor_          = 0;
    protocolMinor_          = 9;
    method_                 = Unsupported;
    haveHost_               = false;
    haveIfModifiedSince_    = false;
    haveIfUnmodifiedSince_  = false;
    expectContinue_         = false;
    haveRange_              = false;
    persist_                = false;
    path_                   = QString::null;
    host_                   = QString::null;
    ifModifiedSince_        = QDateTime();
    ifUnmodifiedSince_      = QDateTime();
    range_.clear();
  }

    QString
  Request::clean(const QString & _path) const
  {
    QString s(_path);

    while (s.endsWith("/./"))
      s.truncate(s.length() - 2);

    while (s.endsWith("/."))
      s.truncate(s.length() - 1);

    // Double slash -> slash.

    QRegExp r("\\/\\/+");
    s.replace(r, "/");

    return s;
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
