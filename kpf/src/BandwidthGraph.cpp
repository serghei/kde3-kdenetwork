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

#include <qdrawutil.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "Defines.h"
#include "Utils.h"
#include "BandwidthGraph.h"
#include "WebServer.h"

namespace KPF
{
  BandwidthGraph::BandwidthGraph
    (
     WebServer      * server,
     OverlaySelect    overlaySelect,
     QWidget        * parent,
     const char     * name
    )
    : QWidget       (parent, name, WRepaintNoErase),
      server_       (server),
      max_          (0L),
      overlaySelect_  (overlaySelect)
  {
    setBackgroundOrigin(AncestorOrigin);
    history_.resize(width());
    history_.fill(0L);

    connect
      (
       server_,
       SIGNAL(wholeServerOutput(ulong)),
       SLOT(slotOutput(ulong))
      );

    if (UseOverlays == overlaySelect_)
    {
      connect
        (
         server_,
         SIGNAL(contentionChange(bool)), this,
         SLOT(slotServerContentionChange(bool))
        );

      connect
        (
         server_,
         SIGNAL(pauseChange(bool)), this,
         SLOT(slotServerPauseChange(bool))
        );
    }

    setTooltip();
  }

  BandwidthGraph::~BandwidthGraph()
  {
    // Empty.
  }

    void
  BandwidthGraph::setTooltip()
  {
    QToolTip::add(this, i18n( "%1 on port %2" )
            .arg( server_->root() ).arg( server_->listenPort() ) );
  }

    QRect
  BandwidthGraph::contentsRect() const
  {
    return QRect(1, 1, width() - 2, height() - 2);
  }

    void
  BandwidthGraph::updateContents()
  {
    QRect r(contentsRect());

    uint w = r.width();
    uint h = r.height();
    
    buffer_.fill(this, 0, 0);

    QPainter p(&buffer_);

    p.drawPixmap( ( width()-bgPix_.width() )/2, 
            ( height()-bgPix_.height() )/2, bgPix_ );

    p.setPen(colorGroup().dark());

    for (uint i = 0; i < history_.size(); i++)
    {
      ulong l = history_[i];

      if (0 != l)
      {
        uint barLength =
          static_cast<uint>(l / float(max_) * h);

        p.drawLine(i + 1, h, i + 1, h - barLength);
      }
    }

    drawOverlays(p);

    update();
  }

    void
  BandwidthGraph::paintEvent(QPaintEvent * e)
  {
    bitBlt(this, e->rect().topLeft(), &buffer_, e->rect());
  }

    void
  BandwidthGraph::resizeEvent(QResizeEvent *)
  {
    buffer_.resize(size());

    if ( width() > 48 )
      bgPix_ = KGlobal::iconLoader()->loadIcon( "kpf", KIcon::Panel, 48 );
    else if ( width() > 32 )
      bgPix_ = KGlobal::iconLoader()->loadIcon( "kpf", KIcon::Panel, 32 );
    else if ( width() > 16 )
      bgPix_ = KGlobal::iconLoader()->loadIcon( "kpf", KIcon::Panel, 16 );
    else
      bgPix_.fill( this, QPoint( 0, 0 ) );

    KIconEffect::semiTransparent( bgPix_ );

    if (width() < 2)
    {
      // We have 0 space. Make history 0 size.
      history_ = QMemArray<ulong>();
      return;
    }

    uint w = width() - 2;

    if (w < history_.size())
    {
      QMemArray<ulong> newHistory(w);

      uint sizeDiff = history_.size() - w;

      for (uint i = sizeDiff; i < history_.size(); i++)
        newHistory[i - sizeDiff] = history_[i];

      history_ = newHistory;
    }
    else if (w > history_.size())
    {
      QMemArray<ulong> newHistory(w);

      uint sizeDiff = w - history_.size();

      for (uint i = 0; i < sizeDiff; i++)
      {
        newHistory[i] = 0L;
      }

      for (uint i = 0; i < history_.size(); i++)
        newHistory[sizeDiff + i] = history_[i];

      history_ = newHistory;
    }
    
    updateContents();
  }

    void
  BandwidthGraph::slotOutput(ulong l)
  {
    QRect r(contentsRect());

    uint w = r.width();
    uint h = r.height();

    if (0 == w || 0 == h)
      return;

    ulong oldMax = max_;

    max_ = 0L;

    if (history_.size() != w)
      return;

    for (uint i = 1; i < w; i++)
    {
      history_[i - 1] = history_[i];
      max_ = max(history_[i], max_);
    }

    history_[w - 1] = l;
    max_ = max(l, max_);

    if (max_ != oldMax)
      emit(maximumChanged(max_));

    updateContents();
  }

    void
  BandwidthGraph::drawOverlays(QPainter & p)
  {
    if (NoOverlays == overlaySelect_)
      return;

    if (!overlayPixmap_.isNull())
    {
      p.drawPixmap(3, 3, overlayPixmap_);
    }

    if (width() < 32 || height() < 32)
      return;

    if (overlayPixmap_.isNull())
    {
      QString maxString;

      QString bs(i18n("%1 b/s"));
      QString kbs(i18n("%1 kb/s"));
      QString mbs(i18n("%1 Mb/s"));

      if (max_ > 1024)
        if (max_ > 1024 * 1024)
          maxString = mbs.arg(max_ / (1024 * 1024));
        else
          maxString = kbs.arg(max_ / 1024);
      else if ( max_ > 0 )
        maxString = bs.arg(max_);
      else
        maxString = i18n( "Idle" );

      p.setPen(Qt::white);

      p.drawText
        (
         4,
         4 + fontMetrics().ascent(),
         maxString
        );

      p.setPen(Qt::black);

      p.drawText
        (
         3,
         3 + fontMetrics().ascent(),
         maxString
        );
    }
  }

    QSize
  BandwidthGraph::sizeHint() const
  {
    return QSize(32, 32);
  }

    QSize
  BandwidthGraph::minimumSizeHint() const
  {
    return QSize(12, 12);
  }

    WebServer *
  BandwidthGraph::server()
  {
    return server_;
  }

    void
  BandwidthGraph::slotServerContentionChange(bool)
  {
    updateOverlayPixmap();
  }

    void
  BandwidthGraph::slotServerPauseChange(bool)
  {
    updateOverlayPixmap();
  }

    void
  BandwidthGraph::updateOverlayPixmap()
  {
   if (server_->paused())
    {
      overlayPixmap_ = SmallIcon("player_pause");
    }
    else
    {
      if (server_->portContention())
      {
        overlayPixmap_ = SmallIcon("connect_creating");
      }
      else
      {
        overlayPixmap_ = QPixmap();
      }
    }
  }

} // End namespace KPF

#include "BandwidthGraph.moc"
// vim:ts=2:sw=2:tw=78:et
