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

#ifndef KPF_RESPONSE_H
#define KPF_RESPONSE_H

#include "Request.h"

namespace KPF
{
  /**
   * Represents some of the data which is used as a reponse to an
   * HTTP request.
   */
  class Response
  {
    public:

      /**
       *
       */
      Response();

      /**
       *
       */
      virtual ~Response();

      /**
       * Each response has a code. See the HTTP specification.
       */
      void setCode(uint);

      /**
       * Set the size, in bytes, of the resource that will be transferred
       * to the client.
       */
      void setSize(ulong);

      /**
       * @return true if code isn't 0.
       */
      bool          valid()               const;

      /**
       * @return size of requested resource.
       */
      ulong size()                const;

      /**
       * @return HTTP response code.
       */
      uint  code()                const;

      /**
       * @return header/body data to send to the client. This string is
       * constructed differently depending on HTTP response code.
       */
      QCString      text(const Request &) const;

      /**
       * Reset to initial state.
       */
      void clear();

    protected:

      /**
       * @internal
       * Create HTML.
       */
      QString data(uint, const Request &) const;

    private:

      uint code_;
      uint size_;
  };

} // End namespace KPF
#endif
// vim:ts=2:sw=2:tw=78:et
