/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#include <qtimer.h>
#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>
#include <qpointarray.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <klocale.h>

#include "kwirelesswidget.h"
#include "propertytable.h"
#include "linuxwirelesswidget.h"

extern "C" {
#include <math.h>
}

QPtrList<DeviceInfo> KWireLessWidget::deviceInfo;
QMutex KWireLessWidget::mutex;
QTimer *KWireLessWidget::timer;
int KWireLessWidget::m_instances;

DeviceInfo::DeviceInfo(QString _device, QString _essid, QString _encr,
                       float _quality, float _signal, float _noise,
                       int _bitrate)
    : m_device(_device),
      m_essid(_essid),
      m_quality(_quality),
      m_noise(_noise),
      m_signal(_signal),
      m_bitrate(_bitrate),
      m_encr(_encr)
{
}

float DeviceInfo::quality()
{
    return m_quality;
}

QString DeviceInfo::qualityString()
{
    return i18n("%1%").arg(QString::number(m_quality*100, 'f', 0));
}

float DeviceInfo::signal()
{
    return m_signal;
}

QString DeviceInfo::signalString()
{
    return i18n("%1%").arg(QString::number(m_signal*100, 'f', 0));
}

float DeviceInfo::noise()
{
    return m_noise ;
}

QString DeviceInfo::noiseString()
{
    return i18n("%1%").arg(QString::number(m_noise*100, 'f', 0));
}

const QString& DeviceInfo::device()
{
    return m_device;
}

const QString& DeviceInfo::essid()
{
    return m_essid;
}


QString DeviceInfo::bitrateString()
{
    QString bitrate;
    QTextOStream b(&bitrate);
    b.precision(2);

    switch((int)log10(m_bitrate))
    {
    case 0:
    case 1:
    case 2:
        b << m_bitrate << " bit/s";
        break;
    case 3:
    case 4:
    case 5:
        b << m_bitrate/1000 << " kbit/s";
        break;
    case 6:
    case 7:
    case 8:
        b << m_bitrate/1000000 << " Mbit/s";
        break;
    case 9:
    case 10:
    case 11:
        b << m_bitrate/1000000000 << " Gbit/s";
        break;
    default:
        b << m_bitrate << " bit/s (!)";
    };
    return bitrate;
}

bool DeviceInfo::usesEncryption()
{
    return !m_encr.isEmpty();
}

QString DeviceInfo::encrString()
{
    if(m_encr.isEmpty())
    {
        return i18n("unknown");
    } else {
        // we rely on the information provided (that is, it has to
        // translated already!):
        return m_encr;
    }
}

KWireLessWidget::KWireLessWidget(QWidget *parent, const char* name)
    : QWidget(parent, name),
      mode(Horizontal),
      frameWidth(1),
      qualityBarWidth(6),
      signalBarWidth(3),
      noiseBarWidth(3)
{
    ++m_instances;
    deviceInfo.setAutoDelete(true);
    // set up the poll timer:
    if(timer == 0)
    {   // this way, only the poll() method of the first instance will
        // ever be called (intended behaviour):
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), SLOT(poll()));
        // single shot, will be restartet at the end of poll ()
        timer->start(100, true);
    }
}

KWireLessWidget::~KWireLessWidget()
{
    --m_instances;
}

int KWireLessWidget::preferredHeight()
{
    return preferredWidth();
}

int KWireLessWidget::preferredWidth()
{
    return 4*frameWidth + qualityBarWidth + signalBarWidth + noiseBarWidth;
}

int KWireLessWidget::instances()
{
    return m_instances;
}

void KWireLessWidget::setMode(KWireLessWidget::Mode _mode)
{
    mode = _mode;
}

void KWireLessWidget::mousePressEvent(QMouseEvent *e)
{
    if( e->button() == QMouseEvent::LeftButton)
    {
        PropertiesDialog dialog(this);
        connect(this, SIGNAL(updateDeviceInfo(QPtrList<DeviceInfo> *)),
                &dialog, SLOT(update(QPtrList<DeviceInfo> *)));
        dialog.exec();
    }
}

void KWireLessWidget::paintEvent(QPaintEvent*)
{
    // WORK_TO_DO: paint other devices, too:
    // for quick repaints, we need a buffered painter!
    int w, h, space;
    const int bevel = qualityBarWidth/2;
    QPainter painter(this);
    QPointArray points;
    QColor color, brush;

    // do stuff that does not rely on device being valid (e.g.,
    // non-nil):
    space = 3 * frameWidth + qualityBarWidth + signalBarWidth + noiseBarWidth+1;

    if(mode == Horizontal)
    {
        h = space;
        w = width();
    } else {
        // Vertical
        h = height();
        w = space;
    }

    painter.setPen(QPen(Qt::black, frameWidth));
    painter.setBrush(KGlobalSettings::baseColor());
    painter.drawRect(0, 0, w, h);

    int x = qualityBarWidth + frameWidth;
    mode == Vertical
          ? painter.drawLine(x, frameWidth, x, h-frameWidth)
          : painter.drawLine(frameWidth, x, w-frameWidth, x);
    x += signalBarWidth + frameWidth;
    mode == Vertical
          ? painter.drawLine(x, frameWidth, x, h-frameWidth)
          : painter.drawLine(frameWidth, x, w-frameWidth, x);

    DeviceInfo *device = deviceInfo.getFirst();
    if(device != 0)
    {
        // draw the quality bar:
        switch((int)(device->quality()*3))
        {
        case 0:
            color = Qt::darkRed;
            brush = Qt::red;
            break;
        case 1:
            color = Qt::darkYellow;
            brush = Qt::yellow;
            break;
        default:
            color = Qt::darkGreen;
            brush = Qt::green;
        };
        if(mode == Vertical)
        {
            int position = (int)(((float)(h-2*frameWidth))*(1-device->quality())+0.5);

            points.putPoints
                (0, 4,
                 frameWidth, h - 2*frameWidth,
                 frameWidth, position + bevel/2,
                 qualityBarWidth, QMAX(position - bevel/2, frameWidth),
                 qualityBarWidth, h - 2*frameWidth);
        } else {
            int position = (int)(((float)(w-2*frameWidth))*device->quality()+0.5);

            points.putPoints
                (0, 4,
                 frameWidth, frameWidth,
                 QMIN(position - bevel/2, w-frameWidth), frameWidth,
                 QMIN(position + bevel/2, w-frameWidth), frameWidth+qualityBarWidth-1,
                 frameWidth, frameWidth+qualityBarWidth-1);
        }
        painter.setPen(QPen(color));
        painter.setBrush(brush);
        painter.drawPolygon(points);

        // draw the signal bar:
        painter.setPen(QPen(Qt::darkRed));
        painter.setBrush(Qt::red);

        if(mode == Vertical)
        {
            int x = 2 * frameWidth + qualityBarWidth;
            int h1 = (int)(((float)(h-2*frameWidth))*(1-device->signal())+0.5);

            painter.drawRect(x, h1, signalBarWidth, h - frameWidth - h1);
        } else {
            int x = frameWidth;
            int y = 2 * frameWidth + qualityBarWidth;
            int w1 = (int)(((float)(w-2*frameWidth))*device->signal()+0.5);

            painter.drawRect(x, y, w1, signalBarWidth);
        }
        // draw the noise bar:
        painter.setPen(QPen(Qt::gray));
        painter.setBrush(Qt::lightGray);

        if(mode == Vertical)
        {
            int x = 3 * frameWidth + qualityBarWidth + signalBarWidth;
            int h1 = (int)(((float)(h-2*frameWidth))*(1-device->noise())+0.5);

            painter.drawRect(x, h1, signalBarWidth, h - frameWidth - h1);
        } else {
            int x = frameWidth;
            int y = 3 * frameWidth + qualityBarWidth + signalBarWidth;
            int w1 = (int)(((float)(w-2*frameWidth))*device->noise()+0.5);

            painter.drawRect(x, y, w1, signalBarWidth);
        }
    }
}

KWireLessWidget* KWireLessWidget::makeWireLessWidget(QWidget *parent,
                                                     const char *name)
{
#if 1
    // defined linuxwirelesswidget
    return new LinuxWireLessWidget(parent, name);
#else
#error KWireLess is not available for your OS
#endif
    // just to shut the compiler up:
    parent = parent; name = name; return 0;
}

#include "kwirelesswidget.moc"
