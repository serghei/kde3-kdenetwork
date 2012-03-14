/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#include <qtooltip.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <unistd.h> // for ::close
#include "linuxwirelesswidget.h"
#include "config.h"

extern "C" {
#include <iwlib.h>
// #include <linux/wireless.h>
}

int LinuxWireLessWidget::socketFD;
QStringList LinuxWireLessWidget::deviceNames;

LinuxWireLessWidget::LinuxWireLessWidget(QWidget *parent, const char* name)
    : KWireLessWidget(parent, name),
      m_number  (-1)
{
}

LinuxWireLessWidget::~LinuxWireLessWidget()
{
    if(instances() == 1)
    {   // I am the only one
        if(socketFD != 0)
        {
            kdDebug() << "KWireLessWidget dtor: closing FD, over and out."
                      << endl;
            ::close(socketFD); // prevent from using QWidget::close
            socketFD = 0;
        }
    }
}


void LinuxWireLessWidget::poll()
{
    // BE AWARE: do not call this method, only the first instance is
    // supposed to do so, and it has been taken care of!
    // this will repoll the device names every ReEnumCountDownStart
    // timer ticks:
    const int ReEnumCountDownStart = 30;
    static int ReEnumCountDown;
    struct wireless_info info;
    struct wireless_config config;
    struct iwreq wrq;
    bool updateNeeded = false;
    bool updateToolTip = false;

    // get a socket file descriptor:
    if(socketFD == 0)
        {
            socketFD = iw_sockets_open();
        }
    if(socketFD == 0)
    {
        kdDebug() << "KWireLessWidget ctor: opening socket file descriptor failed."
                  << endl;
        return;
    }
    // ----- query the available devices:
    mutex.lock(); // querying the device names is protected:
    if(ReEnumCountDown-- == 0)
    {
        ReEnumCountDown = ReEnumCountDownStart;
        updateToolTip = true;

        // query device information:
        deviceNames.clear();
        iw_enum_devices(socketFD, devEnumHandler, 0, 0);

        if (m_number != (int) deviceNames.count())
        {
            m_number = deviceNames.count();
            kdDebug() << "KWireLessWidget::poll: found "
                      << deviceNames.count() << " wireless "
                      << ((deviceNames.count() == 1) ? "device" : "devices")
                      << endl;
        }
    }

    // -----  get the device information:
    QStringList::Iterator it;

    deviceInfo.clear();
    for(it=deviceNames.begin(); it!=deviceNames.end(); ++it)
    {
        // I think it is OK to use this cast, since the QT docs say the
        // returned pointer is valid until the value of the string is
        // changed, which will not happen here (horrible cast, anyway
        // ...):
        char *device_c_str = (char*)(*it).latin1();

        if(iw_get_basic_config(socketFD, device_c_str, &config) == -1)
	{
            kdDebug() << "KWireLessWidget::poll: device "
                      << *it << " does not seem to be a wireless device"
                      << endl;
	} else {
            // WORK_TO_DO: decide whether updates are needed or not
            // create a DeviceInfo object and fill it:
            QString dev, essid, encr;
            float quality=0, signal=0, noise=0;
            int bitrate;

            dev = *it;
            // get the bitrate:
            if(iw_get_ext(socketFD, device_c_str, SIOCGIWRATE, &wrq) >=0)
	    {
                info.has_bitrate = 1;
                memcpy(&(info.bitrate), &(wrq.u.bitrate), sizeof(iwparam));
	    }
            bitrate = info.bitrate.value;
            // get the ranges (needed to translate the absolute values
            // reported by the driver):
            if(iw_get_range_info(socketFD, device_c_str, &(info.range)) >= 0)
            {
                info.has_range = 1;
            }

            // get the device statistics:
#ifdef HAVE_IW_27 
	    if(iw_get_stats(socketFD, device_c_str, &(info.stats), &(info.range), info.has_range)>= 0)
#else
	    if(iw_get_stats(socketFD, device_c_str, &(info.stats)) >= 0)
#endif
	    {
                info.has_stats = 1;
                // get the link quality (logic has been taken from
                // wireless-tools, iwlib.c):

		int rqn = info.range.max_qual.noise;
		int rql = info.range.max_qual.level;
		int rmq = info.range.max_qual.qual;

		// in case the quality levels are zero, do not allow division by zero, and
		// instead set the max. possible quality range to 255 (max of "unsigned char")
		if (!rqn) rqn = 255;
		if (!rql) rql = 255;
		if (!rmq) rmq = 255;

                bool isAbs; // true if values are relative tp a peak value,
                            // otherwise values are dBm
                isAbs = (float)info.stats.qual.level > (int)info.range.max_qual.level;
                if(isAbs)
                {
                    noise  = 1 + ((float)info.stats.qual.noise - 0x100) / rqn;
                    signal = 1 + ((float)info.stats.qual.level - 0x100) / rql;
                } else {
                    noise  = ((float)info.stats.qual.noise) / rqn;
                    signal = ((float)info.stats.qual.level) / rql;
                }

                quality = ((float)info.stats.qual.qual) / rmq;

                updateNeeded = true;
	    }
	    if(config.has_essid)
		essid = config.essid;
	    else
                essid = i18n("<unknown>");

            if (config.has_key && config.key_size>0) {
                encr = i18n("enabled");
            } else {
		// non-root users don't get information about encryption status
		encr = (getuid() == 0) ? i18n("disabled") : i18n("no information");
	    }

            DeviceInfo *device = new DeviceInfo(dev, essid, encr, quality, signal,
                                                noise, bitrate);
            deviceInfo.append(device);
        }
    }
    mutex.unlock();
    if(updateNeeded)
    {
        emit(updateDeviceInfo(&deviceInfo));
        repaint(false);
    }
    if(updateToolTip)
    {
        DeviceInfo *info;
        QString text;

        for(info=deviceInfo.first(); info; info=deviceInfo.next())
        {
	    if (!text.isEmpty())
		text.append('\n');
            text += i18n("%1: Link Quality %2, Bitrate: %3")
				.arg(info->device())
				.arg(info->qualityString())
				.arg(info->bitrateString());
        }
        QToolTip::add(this, text);

        updateToolTip = false;
    }

    timer->start(330, true); // single shot
}

// static:
int LinuxWireLessWidget::devEnumHandler(int	skfd, char * ifname, char **, int)
{
    struct wireless_config config;

    if(iw_get_basic_config(skfd, ifname, &config) != -1)
    {
        deviceNames.append(ifname);
    }
    return 0;
}


#include "linuxwirelesswidget.moc"
