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

#ifndef KPF_RESOURCE_H
#define KPF_RESOURCE_H

#include <qstring.h>
#include <qdatetime.h>

namespace KPF
{
  /**
   * Provide any resource the client requests. This may be a file, a
   * directory listing, or nothing at all, depending on the flabbiness
   * of the client's arse.
   */
  class Resource
  {
    public:

      /**
       * Default ctor, object unusable until you setPath().
       */
      Resource();

      /**
       * Closes all open files.
       */
      virtual ~Resource();

      /**
       * Reset this object and tell it what the new paths are.
       */
      void setPath(const QString & root, const QString & relativePath);

      /**
       * @return true if the file was opened ok or the dir was readable.
       */
      bool open();

      /**
       * Just close.
       */
      void close();

      /**
       * Seek to the specified position.
       * @return false if this is a dir.
       */
      bool seek(int);

      /**
       * Read a block of the file or the generated HTML.
       */
      int readBlock(char * data, uint maxlen);

      /**
       * @return false if the file or directory doesn't exist.
       */
      bool exists() const;

      /**
       * Performs a search through the entire path, looking for symbolic links.
       *
       * Expensive !
       *
       * @return true if the path contains a symbolic link.
       */
      bool symlink() const;

      /**
       * @return true if the resource is readable.
       */
      bool readable() const;

      /**
       * @return mtime of resource.
       */
      QDateTime lastModified() const;

      /**
       * @return size of file, or size of HTML that will be generated.
       */
      uint size() const;

      /**
       * @return current file position.
       */
      int at() const;

      /**
       * @return true if nothing left to read.
       */
      bool atEnd() const;

      /**
       * @return true if file, false if dir. Perhaps I'll make the HTML
       * seekable later.
       */
      bool seekable() const;

      /**
       * @return mime type of file if available. If dir, returns text/html.
       * If nothing available, returns text/plain.
       */
      QString mimeType() const;

      /**
       * Reset to initial state.
       */
      void clear();

    private:

      /**
       * Update d->size;
       */
      void calculateSize();

      void generateHTML();

      class Private;
      Private * d;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
