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

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kconfig.h>

#include "Defines.h"
#include "Utils.h"
#include "Response.h"
#include "Defaults.h"

namespace KPF
{
  Response::Response()
    : code_(0),
      size_(0)
  {
    // Empty.
  }

      void
  Response::setCode(uint code)
  {
    code_ = code;
  }

      void
  Response::setSize(ulong size)
  {
    size_ = size;
  }

  Response::~Response()
  {
    // Empty.
  }

    bool
  Response::valid() const
  {
    return 0 != code_;
  }

    ulong
  Response::size() const
  {
    return size_;
  }

    uint
  Response::code() const
  {
    return code_;
  }

    QCString
  Response::text(const Request & request) const
  {
    QString s;

    // XXX: Ensure that all codes we know about are enumerated here.
    switch (code_)
    {
      case 200:
      case 206:
      case 304:
        if (request.protocol() >= 1.0)
        {
          s = QString(request.protocolString())
            + QString(" %1 %2\r\n").arg(code_).arg(responseName(code_));
        }
        break;

      case 400:
      case 403:
      case 404:
      case 412:
      case 416:
      case 500:
      case 501:
      case 505:
        s = QString(request.protocolString())
          + QString(" %1 %2\r\n").arg(code_).arg(responseName(code_))
          + data(code_, request);
        break;

      default:
        kpfDebug << "Huh ?" << endl;
        break;
    }

    return s.utf8();
  }

    QString
  Response::data(uint code, const Request & request) const
  {
    QString contentType = "Content-Type: text/html; charset=utf-8\r\n";

    KConfig config(Config::name());

    config.setGroup("General");

    QString html;

    if
      (
       config.readBoolEntry
       (Config::key(Config::CustomErrors), Config::DefaultCustomErrors)
      )
    {
      config.setGroup("ErrorMessageOverrideFiles");

      QString filename = config.readPathEntry(QString::number(code));

      if (!filename.isEmpty())
      {
        QFile f(filename);

        if (f.open(IO_ReadOnly))
        {
          QRegExp regexpMessage   ("ERROR_MESSAGE");
          QRegExp regexpCode      ("ERROR_CODE");
          QRegExp regexpResource  ("RESOURCE");

          QTextStream str(&f);

          while (!str.atEnd())
          {
            QString line(str.readLine());

            line.replace(regexpMessage,   responseName(code));
            line.replace(regexpCode,      QString::number(code));
            line.replace(regexpResource,  request.path());

            html = line + "\r\n";
          }
        }
      }
    }
    else
    {
      html  = "<html>\r\n";
      html += "<head>\r\n";
      html += "<title>\r\n" + responseName(code) + "</title>\r\n";
      html += "<style type=\"text/css\">\r\n";
      html += "BODY { color: black; background-color: rgb(228, 228, 228); }\r\n";
      html += "H1 { font-size: 1.7em; color: rgb(60, 85, 110); }\r\n";
      html += "P { margin: 40px, 40px, 10px, 10px; }\r\n";
      html += "</style>\r\n";
      html += "</head>\r\n";
      html += "<body>\r\n<h1>\r\nError: " + responseName(code) + "\r\n</h1>\r\n";
      html += "<p>Requested resource: " + request.path() + "</p>\r\n";
      html += "</body>\r\n</html>\r\n";
    }

    QString contentLength =
      QString("Content-Length: %1\r\n").arg(html.length());

    return (contentType + contentLength + "\r\n" + html);
  }

    void
  Response::clear()
  {
    code_ = size_ = 0;
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
