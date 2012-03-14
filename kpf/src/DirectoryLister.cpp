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

#include <cmath>

#include <qapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qstylesheet.h>
#include <qpalette.h>
#include <qtextstream.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>

#include "Defines.h"
#include "DirectoryLister.h"
#include "Utils.h"

namespace KPF
{
  class DirectoryLister::Private
  {
    public:

      Private()
      {
      }
  };

  QString colorToCSS(const QColor &c)
  {
    return
      "rgb("
      + QString::number(c.red())
      + ", "
      + QString::number(c.green())
      + ", "
      + QString::number(c.blue())
      + ")";
  }

  QByteArray buildHTML(const QString & title, const QString & body)
  {
    QPalette pal = qApp->palette();
    QByteArray temp_string;
    QTextStream html(temp_string, IO_WriteOnly);

    html.setEncoding(QTextStream::UnicodeUTF8);

    html
      << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      << endl
      << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
      << endl
      << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
      << endl
      << "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
      << endl
      << "\t<head>"
      << endl
      << "\t\t<title>"
      << title
      << "</title>"
      << endl
      << "<style type=\"text/css\">"
      << endl
      << "<!--"
      << endl
      << "table.filelist { "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Foreground))
      << "; "
      << "background-color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Background))
      << "; "
      << "border: thin outset; "
      << "width: 100%; "
      << "}"
      << endl
      << "td { "
      << "margin: 0px; "
      << "white-space: nowrap; "
      << "}"
      << endl
      << "td.norm { "
      << "background-color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Base))
      << "; "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Foreground))
      << "; "
      << "}"
      << endl
      << "td.alt { "
      << "background-color: "
      << colorToCSS
      (
        KGlobalSettings::calculateAlternateBackgroundColor
        (pal.color(QPalette::Normal, QColorGroup::Base))
      )
      << "; "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Foreground))
      << "; "
      << "}"
      << endl
      << "a { "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Text))
      << "; "
      << "text-decoration: none; "
      << "}"
      << endl
      << "th.listheading { "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::ButtonText))
      << "; "
      << "background-color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Button))
      << "; "
      << "text-align: left; "
      << "white-space: nowrap; "
      << "border: thin outset; "
      << "}"
      << endl
      << "a.direntry { "
      << "font-weight: bold; "
      << "}"
      << endl
      << "div.sizeentry { "
      << "color: "
      << colorToCSS(pal.color(QPalette::Normal, QColorGroup::Text))
      << "; "
      << "text-align: right; "
      << "}"
      << endl
      << "-->"
      << endl
      << "</style>"
      << endl
      << "\t</head>"
      << endl
      << "\t<body>"
      << endl
      << body
      << "\t</body>"
      << endl
      << "</html>"
      << endl
      ;

    return temp_string;
  }

  QString prettySize(uint size)
  {
    QString suffix;
    QString temp;
    float floated_size;

    if (size > 1023)
    {
      if (size > 1048575)
      {
        floated_size = size / 1048576.0;
        suffix = i18n(" MB");
      }
      else
      {
        floated_size = size / 1024.0;
        suffix = i18n(" KB");
      }
    }
    else
    {
      temp.setNum(size);
      temp += i18n(" bytes");
      return temp;
    }

    temp.setNum(floated_size, 'f', 1);
    temp += suffix;
    return temp;
  }

  DirectoryLister * DirectoryLister::instance_ = 0L;

    DirectoryLister *
  DirectoryLister::instance()
  {
    if (0 == instance_)
      instance_ = new DirectoryLister;

    return instance_;
  }

  DirectoryLister::DirectoryLister()
  {
    d = new Private;
  }

  DirectoryLister::~DirectoryLister()
  {
    delete d;
  }

    QByteArray
  DirectoryLister::html(const QString & root, const QString & _path)
  {
    kpfDebug << "root: " << root << " path: " << _path << endl;

    QString path;

    if (_path.right(1) != "/")
      path = _path + "/";
    else
      path = _path;

    if (path[0] == '/')
      path + "";

    QDir d(root + path);

    if (!d.exists())
    {
      return buildHTML
        (
          i18n("Error"),
          i18n("Directory does not exist: %1 %2").arg(root).arg(path)
        );
    }

    const QFileInfoList * infoList =
      d.entryInfoList(QDir::DefaultFilter, QDir::Name | QDir::DirsFirst);

    if (0 == infoList)
    {
      return buildHTML
        (
          i18n("Error"),
          i18n("Directory unreadable: %1 %2").arg(root).arg(path)
        );
    }

    QString html;

    html  += "<table";
    html  += " width=\"100%\"";
    html  += " class=\"filelist\">\n";

    html  += "<tr>\n";
    html  += "<th align=\"left\" class=\"listheading\">Name</th>\n";
    html  += "<th align=\"left\" class=\"listheading\">Size</th>\n";
    html  += "</tr>\n";

    for (QFileInfoListIterator it(*infoList); it.current(); ++it)
    {
      static int counter = 0;

      QFileInfo * fi(it.current());

      if (
        (fi->fileName()[0] == '.')
        && ((fi->fileName() != "..") || path == "/")
      )
      {
        // Don't show hidden files
        continue;
      }

      ++counter;

      QString td_class = (counter % 2) ? "alt" : "norm";

      html += "<tr>\n";

      html += "<td class=\"" + td_class + "\">";

      QString item_class = QString((fi->isDir()) ? "direntry" : "fileentry");

      KURL fu(path+fi->fileName());
      html +=
        "<a href=\""
        + fu.encodedPathAndQuery()
        + (fi->isDir() ? "/" : "")
        + "\" class=\""
        + item_class
        + "\">";

      if (fi->fileName() != "..")
          html += QStyleSheet::escape(fi->fileName());
      else
          html += i18n("Parent Directory");

      html += "</a>";

      if (fi->isDir())
        html += "/";

      html += "</td>\n";

      html += "<td class=\"" + td_class + "\">";

      if (!fi->isDir())
          html
            += "<div class=\"sizeentry\">" + prettySize(fi->size()) + "</div>";

      html += "</td>\n";
      html += "</tr>\n";
    }

    html += "</table>\n";

    return buildHTML
      (
        i18n("Directory listing for %1").arg(QStyleSheet::escape(path)),
        html
      );
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
