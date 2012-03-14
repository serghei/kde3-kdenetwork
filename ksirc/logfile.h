/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/
#ifndef __logfile_h__
#define __logfile_h__

#include <qobject.h>

class QFile;

class LogFile : public QObject
{
public:
    LogFile( const QString &channel, const QString &server );
    virtual ~LogFile();

    void open();

    void closeLog();

    void log( const QString &message );

protected:
    virtual void timerEvent( QTimerEvent * );

private:
    QString m_channel;
    QString m_server;

    QFile *m_file;

    int m_flushTimerId;

    QString makeLogFileName( const QString &channel, const QString &server, int suffix = -1 );
};

#endif
