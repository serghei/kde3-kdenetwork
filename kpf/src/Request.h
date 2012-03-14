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

#ifndef KPF_REQUEST_H
#define KPF_REQUEST_H

#include <qcstring.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include "ByteRange.h"

namespace KPF
{
  /**
   * Represents an HTTP request.
   */
  class Request
  {
    public:

      /**
       * HTTP/1.1 specifies many request types, known as 'methods'.
       * An HTTP/1.1 compliant server must implement HEAD and GET.
       * We support only these two. All others are rejected either
       * because they are of limited use or because they are a security
       * risk.
       */
      enum Method { Head, Get, Unsupported };

      /**
       * Construct a Request object and set parameters to defaults. 
       */
      Request();
      virtual ~Request();

      /**
       * Take a list of headers and parse them, setting our values
       * appropriately.
       */
      void parseHeaders(const QStringList &);

      /**
       * Parse one header line and set whatever value is appropriate.
       */
      void handleHeader(const QString & name, const QString & value);

      /**
       * HTTP has had a few revisions (protocols) since birth. Here you may
       * specify the protocol which you believe the client is using. This
       * version of setProtocol parses a string of the form
       * "HTTP/major.minor", where major and minor are integers.
       */
      void setProtocol          (const QString &);

      /**
       * HTTP has had a few revisions (protocols) since birth. Here you may
       * specify the protocol which you believe the client is using.
       */
      void setProtocol          (uint major, uint minor);

      /**
       * Specify the 'method' requested by the client. This version parses
       * a string.
       */
      void setMethod            (const QString &);

      /**
       * Specify the 'method' requested by the client.
       */
      void setMethod            (Method);

      /**
       * Set the path to the requested resource. The path is
       * immediately decoded and canonicalised.
       */
      void setPath              (const QString &);

      /**
       * HTTP/1.1 requests must have a "Host:" header.
       */
      void setHost              (const QString &);

      /**
       * HTTP/1.1 allows an "If-Modified-Since" header, specifying that
       * the requested resource should only be retrieved if the resource
       * has been modified since a certain date and time.
       */
      void setIfModifiedSince   (const QDateTime &);

      /**
       * HTTP/1.1 allows an "If-Unmodified-Since" header, specifying that
       * the requested resource should only be retrieved if the resource
       * has NOT been modified since a certain date and time.
       */
      void setIfUnmodifiedSince (const QDateTime &);

      /**
       * HTTP/1.1 allows an "Expect: 100-continue" header, specifying
       * that the server should immediately respond with "100 Continue".
       */
      void setExpectContinue    (bool);

      /**
       * Specify a range of bytes which should be retrieved from the
       * resource. See RFC 2616.
       */
      void setRange             (const QString &);

      /**
       * HTTP/1.1 allows "persistent" connections. These may be specified
       * by "Keep-Alive:" or "Connection: Keep-Alive" headers.
       */
      void setPersist           (bool);

      /**
       * @return major version of protocol which was set.
       */
      uint  protocolMajor()         const;

      /**
       * @return minor version of protocol which was set.
       */
      uint  protocolMinor()         const;

      /**
       * @return version of protocol which was set, as major.minor, e.g.
       * if the protocol was set to HTTP/0.9, this would return 0.9.
       */
      float         protocol()              const;

      /**
       * @return the (enumerated) request type ('method').
       */
      Method        method()                const;

      /**
       * @return true if @ref setHost was called previously.
       */
      bool          haveHost()              const;

      /**
       * @return true if @ref setIfModifiedSince was called previously.
       */
      bool          haveIfModifiedSince()   const;

      /**
       * @return true if @ref setIfUnmodifiedSince was called previously.
       */
      bool          haveIfUnmodifiedSince() const;

      /**
       * @return the path that was set with (and modified by) setPath()
       */
      QString       path()                  const;

      /**
       * @return the host that was set previously.
       */
      QString       host()                  const;

      /**
       * @return the date/time set by @ref setIfModifiedSince previously.
       */
      QDateTime     ifModifiedSince()       const;

      /**
       * @return the date/time set by @ref setIfUnmodifiedSince previously.
       */
      QDateTime     ifUnmodifiedSince()     const;

      /**
       * @return the protocol as a string, e.g. "HTTP/1.1".
       */
      QCString      protocolString()        const;

      /**
       * @return true if @ref setExpectContinue was used previously.
       */
      bool          expectContinue()        const;

      /**
       * @return byte range set by @ref setRange previously.
       */
      ByteRange     range()                 const;

      /**
       * @return true if @ref setRange was called previously.
       */
      bool          haveRange()             const;

      /**
       * @return true if @ref setPersist was called previously.
       */
      bool          persist()               const;

      /**
       * Reset to initial state.
       */
      void          clear();

      /**
       * Clean up the path given in a request, so it's more like what we
       * expect.
       */
      QString       clean(const QString & path) const;

    private:

      // Order dependency
      uint    protocolMajor_;
      uint    protocolMinor_;
      Method  method_;
      bool    haveHost_;
      bool    haveIfModifiedSince_;
      bool    haveIfUnmodifiedSince_;
      bool    expectContinue_;
      bool    haveRange_;
      bool    persist_;
      // End order dependency

      QString       path_;
      QString       host_;
      QDateTime     ifModifiedSince_;
      QDateTime     ifUnmodifiedSince_;
      ByteRange     range_;
  };

} // End namespace KPF

#endif
// vim:ts=2:sw=2:tw=78:et
