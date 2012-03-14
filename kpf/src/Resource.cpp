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
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <kglobal.h>

#include <kmimetype.h>

#include "Utils.h"
#include "Defines.h"
#include "Resource.h"
#include "DirectoryLister.h"

namespace KPF
{
  enum FileType { Dir, File };

  class Resource::Private
  {
    public:

      Private()
        : size(0),
          sizeCalculated(false),
          offset(0)
      {
      }

      QString     root;
      FileType    fileType;
      QString     path;
      QFile       file;
      QFileInfo   fileInfo;
      QDir        dir;
      uint        size;
      bool        sizeCalculated;
      uint        offset;

      QByteArray  html;
  };

  Resource::Resource()
  {
    d = new Private;
  }

  Resource::~Resource()
  {
    delete d;
    d = 0;
  }

    void
  Resource::setPath(const QString & root, const QString & relativePath)
  {
    kpfDebug << "setPath(`" << root << "',`" << relativePath << "'" << endl;

    d->root           = root;
    d->path           = relativePath;
    d->size           = 0;
    d->offset         = 0;
    d->sizeCalculated = false;

    d->file.close();

    // Fix root if it doesn't have a trailing slash.

    if ('/' != d->root.at(d->root.length() - 1))
      d->root += '/';

    if (d->path.right(1) == "/")
    {
      // A directory was requested 
      kpfDebug << "Directory requested" << endl;

      // Does the path actually point to a directory ?

      if (QFileInfo(d->root + d->path).isDir())
      {
        kpfDebug << "Path points to directory" << endl;

        // Does an index.html exist in that directory ?

        if (QFileInfo(d->root + d->path + "index.html").exists())
        {
          kpfDebug << "Found index.html" << endl;

          // Ok, add `index.html'.

          d->path += "index.html";
        }
        else
        {
          kpfDebug << "NOT Found index.html" << endl;
        }

      }
      else
      {
        kpfDebug << "NOT Path points to directory" << endl;
      }
    }
    else
    {
      kpfDebug << "NOT Directory requested" << endl;
    }

    kpfDebug << "QFileInfo::setFile(`" << d->root << "' + `" << d->path << "'" << endl;
    d->fileInfo.setFile(d->root + d->path);
  }

    bool
  Resource::open()
  {
    if (!d->fileInfo.exists())
    {
      kpfDebug << "File doesn't exist" << endl;
      return false;
    }

    if (d->fileInfo.isDir())
    {
      d->fileType = Dir;
      d->dir.setPath(d->root + d->path);

      if (!d->dir.isReadable())
      {
        kpfDebug << "Dir isn't readable" << endl;
        return false;
      }
      else
      {
        generateHTML();
      }
    }
    else
    {
      d->fileType = File;
      d->file.setName(d->root + d->path);

      if (!d->file.open(IO_ReadOnly))
      {
        kpfDebug << "File isn't readable" << endl;
        return false;
      }
    }

    calculateSize();
    return true;
  }

    void
  Resource::close()
  {
    if (File == d->fileType)
      d->file.close();
  }

    bool
  Resource::seek(int pos)
  {
    if (File == d->fileType)
    {
      return d->file.at(pos);
    }
    else
    {
      // TODO STUB
      return false;
    }
  }

    int
  Resource::readBlock(char * data, uint maxlen)
  {
    int bytesRead(0);

    if (File == d->fileType)
    {
      bytesRead = d->file.readBlock(data, maxlen);
    }
    else
    {
      if (d->offset < d->size)
      {
        uint bytesAvailable = QMIN(maxlen, d->size - d->offset);

        memcpy(data, d->html.data() + d->offset, bytesAvailable);

        d->offset += bytesAvailable;

        return bytesAvailable;
      }
      else
      {
        // Else bytesRead is still 0, because the read was out of bounds.

        kpfDebug << "Out of bounds in html" << endl;
      }
    }

    return bytesRead;
  }

    uint
  Resource::size() const
  {
    return d->size;
  }

    int
  Resource::at() const
  {
    return d->offset;
  }

    bool
  Resource::atEnd() const
  {
    if (File == d->fileType)
    {
      return d->file.atEnd();
    }
    else
    {
      return d->offset >= d->size;
    }
  }

    void
  Resource::calculateSize()
  {
    if (File == d->fileType)
    {
      d->size = d->fileInfo.size();
    }
    else
    {
      d->size = d->html.size() - 1;
    }
  }


    bool
  Resource::readable() const
  {
    return d->fileInfo.isReadable();
  }

    QDateTime
  Resource::lastModified() const
  {
    return d->fileInfo.lastModified();
  }

    bool
  Resource::exists() const
  {
    bool b = d->fileInfo.exists();

    if (!b)
    {
      kpfDebug << "File doesn't exist" << endl;
    }

    return b;
  }

    bool
  Resource::symlink() const
  {
    if (d->fileInfo.isSymLink())
      return true;

    QString path(d->fileInfo.dirPath());

    QStringList l(QStringList::split('/', path));

    QString testPath;

    for (QStringList::ConstIterator it(l.begin()); it != l.end(); ++it)
    {
      testPath += '/';
      testPath += *it; 

      if (QFileInfo(testPath).isSymLink())
        return true;
    }

    return false;
  }

    bool
  Resource::seekable() const
  {
    return !(d->fileInfo.isDir());
  }

    QString
  Resource::mimeType() const
  {
    if (d->fileInfo.isDir())
      return "text/html; charset=utf-8";
    return KMimeType::findByPath( d->root + d->path )->name();
  }

    void
  Resource::generateHTML()
  {
    d->html = DirectoryLister::instance()->html(d->root, d->path); 
  }

    void
  Resource::clear()
  {
    delete d;
    d = new Private;
  }

} // End namespace KPF

// vim:ts=2:sw=2:tw=78:et
