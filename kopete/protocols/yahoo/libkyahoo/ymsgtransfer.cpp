/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <string>

#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "kdebug.h"

#include <qdatastream.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>


using namespace Yahoo;

class YMSGTransferPrivate
{
public:
	int yflag;
	int version;
	int packetLength;
	Yahoo::Service service;
	Yahoo::Status status;
	unsigned int id;
	ParamList data;
	bool valid;
};

YMSGTransfer::YMSGTransfer()
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->id = 0;
	d-> status = Yahoo::StatusAvailable;
}

YMSGTransfer::YMSGTransfer(Yahoo::Service service)
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->service = service;
	d->id = 0;
	d->status = Yahoo::StatusAvailable;
}

YMSGTransfer::YMSGTransfer(Yahoo::Service service, Yahoo::Status status)
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->service = service;
	d->id = 0;
	d->status = status;
}

YMSGTransfer::~YMSGTransfer()
{
	delete d;
}

Transfer::TransferType YMSGTransfer::type()
{
	return Transfer::YMSGTransfer;
}

bool YMSGTransfer::isValid() const
{
	return d->valid;
}

Yahoo::Service YMSGTransfer::service() const
{
	return d->service;
}

void YMSGTransfer::setService(Yahoo::Service service)
{
	d->service = service;
}

Yahoo::Status YMSGTransfer::status() const
{
	return d->status;
}

void YMSGTransfer::setStatus(Yahoo::Status status)
{
	d->status = status;
}

unsigned int YMSGTransfer::id() const
{
	return d->id;
}

void YMSGTransfer::setId(unsigned int id)
{
	d->id = id;
}

int YMSGTransfer::packetLength() const
{
	return d->packetLength;
}

void YMSGTransfer::setPacketLength(int len)
{
	d->packetLength = len;
}

ParamList YMSGTransfer::paramList() const
{
	return d->data;
}

int YMSGTransfer::paramCount( int index ) const
{
	int cnt = 0;
	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
		if( (*it).first == index )
			cnt++;
	}
	return cnt;
}


QCString YMSGTransfer::nthParam( int index, int occurrence ) const
{
	int cnt = 0;
	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
		if( (*it).first == index && cnt++ == occurrence)
			return (*it).second;
	}
	return QCString();
}

QCString YMSGTransfer::nthParamSeparated( int index, int occurrence, int separator ) const
{

	int cnt = -1;
	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
		if( (*it).first == separator )
			cnt++;
		if( (*it).first == index && cnt == occurrence)
			return (*it).second;
	}
	return QCString();
}

QCString YMSGTransfer::firstParam( int index ) const
{
	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
		if( (*it).first == index )
			return (*it).second;
	}
	return QCString();
}

void YMSGTransfer::setParam(int index, const QCString &data)
{
	d->data.append( Param( index, data ) );
}

void YMSGTransfer::setParam( int index, int data )
{
	d->data.append( Param( index, QString::number( data ).local8Bit() ) );
}

int YMSGTransfer::length() const
{
	int len = 0;
	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
		len += QString::number( (*it).first ).length();
		len += 2;
		len += (*it).second.length();
		len += 2;
	}
	return len;
}


QByteArray YMSGTransfer::serialize() const
{
	/*
	<------- 4B -------><------- 4B -------><---2B--->
	+-------------------+-------------------+---------+
	|   Y   M   S   G   |      version      | pkt_len |
	+---------+---------+---------+---------+---------+
	| service |      status       |    session_id     |
	+---------+-------------------+-------------------+
	|                                                 |
	:                    D A T A                      :
	/                   0 - 65535*                   |
	+-------------------------------------------------+
	*/

	int pos = 0;
	QStringList::ConstIterator listIt = 0;
	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );

	stream << (Q_INT8)'Y' << (Q_INT8)'M' << (Q_INT8)'S' << (Q_INT8)'G';
	if( d->service == Yahoo::ServicePictureUpload )
		stream << (Q_INT16)0x0f00;
	else
		stream << (Q_INT16)0x000f;
	stream << (Q_INT16)0x0000;
	if( d->service == Yahoo::ServicePictureUpload ||
		d->service == Yahoo::ServiceFileTransfer )
		stream << (Q_INT16)(length()+4);
	else
		stream << (Q_INT16)length();
	stream << (Q_INT16)d->service;
	stream << (Q_INT32)d->status;
	stream << (Q_INT32)d->id;
 	for (ParamList::ConstIterator it = d->data.constBegin(); it !=  d->data.constEnd(); ++it)
	{
 		kdDebug(YAHOO_RAW_DEBUG) << " Serializing key " << (*it).first << " value " << (*it).second << endl;
		stream.writeRawBytes ( QString::number( (*it).first ).local8Bit(), QString::number( (*it).first ).length() );
		stream << (Q_INT8)0xc0 << (Q_INT8)0x80;
		stream.writeRawBytes( (*it).second, (*it).second.length() );
		stream << (Q_INT8)0xc0 << (Q_INT8)0x80;
	}
	kdDebug(YAHOO_RAW_DEBUG) << " pos=" << pos << " (packet size)" << buffer << endl;
	return buffer;
}

