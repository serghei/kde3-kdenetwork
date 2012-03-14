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

#ifndef KPF_DIRECTORY_LISTER_H
#define KPF_DIRECTORY_LISTER_H

#include <qstring.h>
#include <qcstring.h>

namespace KPF
{
  /**
   * Creates an HTML document by listing the contents of a directory.
   */
  class DirectoryLister
  {
    public:

      /**
       * @return a pointer to the single instance of DirectoryLister.
       */
      static DirectoryLister * instance();

      virtual ~DirectoryLister();

      /**
       * Get a directory listing (HTML) for the specified path. Uses
       * cache if directory has not been modified since last read.
       */
      QByteArray html(const QString &root, const QString & path);

      uint headerLength() const;
      uint footerLength() const;
      uint emptyEntryLength() const;

    private:

      /**
       * Constructs a directory listing object.
       */
      DirectoryLister();

      static DirectoryLister * instance_;

      class Private;
      Private * d;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
