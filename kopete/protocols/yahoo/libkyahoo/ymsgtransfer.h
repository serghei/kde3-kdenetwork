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

#ifndef YMSG_TRANSFER_H
#define YMSG_TRANSFER_H

#include "transfer.h"

#include "yahootypes.h"

#include <qpair.h>
#include <qvaluelist.h>

class YMSGTransferPrivate;

typedef QPair< int, QCString > Param;
typedef QValueList< Param > ParamList;

/**
@author Duncan Mac-Vicar Prett
*/
class YMSGTransfer : public Transfer
{
public:
	YMSGTransfer(Yahoo::Service service);
	YMSGTransfer(Yahoo::Service service, Yahoo::Status status);
	YMSGTransfer();
	~YMSGTransfer();


	TransferType type();

	//! Get the validity of the transfer object
	bool isValid() const;
	Yahoo::Service service() const;
	void setService(Yahoo::Service service);
	Yahoo::Status status() const;
	void setStatus(Yahoo::Status status);
	unsigned int id() const;
	void setId(unsigned int id);
	int packetLength() const;
	void setPacketLength(int len);


	ParamList paramList() const;
	QCString firstParam( int index ) const;
	QCString nthParam( int index, int occurrence ) const;
	QCString nthParamSeparated( int index, int occurrence, int separator ) const;
	int paramCount( int index ) const;


	void setParam(int index, const QCString &data);
	void setParam(int index, int data);
	QByteArray serialize() const;

	int length() const;
private:
	YMSGTransferPrivate* d;
};

#endif
