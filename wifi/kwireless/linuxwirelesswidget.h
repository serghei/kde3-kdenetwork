/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#ifndef LINUXWIRELESSWIDGET_H
#define LINUXWIRELESSWIDGET_H

#include "kwirelesswidget.h"

class LinuxWireLessWidget : public KWireLessWidget
{
    Q_OBJECT
public:
    LinuxWireLessWidget(QWidget *parent=0, const char* name=0);
    ~LinuxWireLessWidget();
protected:
    /** poll() is supposed to read the information from
       /proc/net/wireless (Linux) and put it into the data members. */
    void poll();
    static int socketFD; // the socket file descriptor to talk to the
			 // driver
    static QStringList deviceNames;
    static int devEnumHandler(int	skfd,
                              char *	ifname,
                              char *	args[],
                              int	count);
    /** Store previously found number of wireless devices. */
    int m_number;
};

#endif
