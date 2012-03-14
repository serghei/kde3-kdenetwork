/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#ifndef KWIRELESS_WIDGET_H
#define KWIRELESS_WIDGET_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qmutex.h>
#include <qstringlist.h>

class DeviceInfo
{
public:
    DeviceInfo(QString _device=QString::null, QString _essid=QString::null,
	       QString _encr=QString::null,
               float _quality=0, float _signal=0, float _noise=0,
               int _bitrate=0);
    QString bitrateString();
    float quality();
    QString qualityString();
    float signal();
    QString signalString();
    float noise();
    QString noiseString();
    const QString& device();
    const QString& essid();
    bool usesEncryption();
    QString encrString();
protected:
    QString m_device; // the device name (e.g., eth1)
    QString m_essid; // the network name

    // all this values are coefficients (values between 0 and 1):
    float m_quality; // link quality level
    float m_noise; // the noise level
    float m_signal; // the signal level
    // all these values are absolut, e.g., 11000000 = 11MBit/sec:
    int m_bitrate; // bit rate
    QString m_encr;
};

/** This wigdet displays the information about one wireless device.
    It is supposed to "look good" at about any resolution (to enable
    it to be used in the panel). */

class KWireLessWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
	Horizontal,
	Vertical
    };
    static KWireLessWidget* makeWireLessWidget(QWidget *parent = 0,
                                               const char *name = 0);
protected:
    KWireLessWidget(QWidget *parent=0, const char* name=0);
public:
    ~KWireLessWidget();
    void setMode(Mode);
    // These methods return values according to the mode:
    int preferredHeight();
    int preferredWidth();
    int instances();
protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *e);
protected:
    Mode mode;
    int frameWidth;
    int qualityBarWidth;
    int signalBarWidth;
    int noiseBarWidth;
    // some static device information
    // a list of DeviceInfo objects, this needs to be filled by the
    // poll method:
    static QPtrList<DeviceInfo> deviceInfo;
    static QTimer *timer; // the poll timer
    static QMutex mutex;
    static int m_instances;
signals:
    void updateDeviceInfo(QPtrList<DeviceInfo> *);
protected slots:
    /** poll() is system dependent and needs to be implemented by
	deriving classes.
        poll() has to put a DeviceInfo object into deviceInfo for each
        wireless network device on the system. It uses mutex to
        serialize access to this objects.
        poll() will be called by a timer. If you cannot implement a
        KWireLessWidget for your OS this way, please contact me
        (mirko@kde.org).
    */
    virtual void poll() = 0;
};

#endif // KWIRELESS_WIDGET_H
