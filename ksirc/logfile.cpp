/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/

#include "logfile.h"

#include <assert.h>

#include <qdir.h>

#include <kstandarddirs.h>

LogFile::LogFile( const QString &channel, const QString &server )
    : m_channel( channel ), m_server( server ), m_file( new QFile() ),
      m_flushTimerId( -1 )
{
}

LogFile::~LogFile()
{
    closeLog();
    delete m_file;
}

void LogFile::open()
{
    int suffix = 1;

    m_file->setName( makeLogFileName( m_channel, m_server ) );

    while ( !m_file->open( IO_WriteOnly | IO_Append ) && suffix < 16000 ) // arbitrary limit ;)
    {
        m_file->setName( makeLogFileName( m_channel, m_server, suffix ) );
        suffix++;
    }

    assert( m_file->isOpen() == true );

    log( QString::fromLatin1( "### Log session started at " )
         + QDateTime::currentDateTime().toString()
         + QString::fromLatin1( "###\n" ) );
}

void LogFile::closeLog()
{
    log( QString::fromLatin1( "### Log session terminated at " )
         + QDateTime::currentDateTime().toString()
         + QString::fromLatin1( "###\n" ) );

    if ( m_flushTimerId != -1 )
        killTimer( m_flushTimerId );

    m_file->close();
}

void LogFile::log( const QString &message )
{
    m_file->writeBlock( message.local8Bit(), message.length() );

    if ( m_flushTimerId == -1 )
        m_flushTimerId = startTimer( 60000 ); // flush each minute
}

void LogFile::timerEvent( QTimerEvent * )
{
    if ( m_file )
        m_file->flush();

    killTimer( m_flushTimerId );
    m_flushTimerId = -1;
}

QString LogFile::makeLogFileName( const QString &channel, const QString &server, int suffix )
{
    QString res = channel + '_';

    QDate dt = QDate::currentDate();
    QString dateStr;
    dateStr.sprintf( "%.4d_%.2d_%.2d_", dt.year(), dt.month(), dt.day() );
    res += dateStr;

    res += server;

    res += ".log";

    if ( suffix > -1 )
        res += '.' + QString::number( suffix );

    return locateLocal( "appdata", "logs/" + res );
}
