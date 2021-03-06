/*
    Kopete Oscar Protocol
    aimlogintask.h - Handles logging into to the AIM service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "ownuserinfotask.h"
#include <qcstring.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"
#include "userdetails.h"
#include "ssimanager.h"


using namespace Oscar;

OwnUserInfoTask::OwnUserInfoTask( Task* parent ) : Task( parent )
{
}


OwnUserInfoTask::~OwnUserInfoTask()
{
}


bool OwnUserInfoTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	else
	{
		if ( st->snacService() == 0x01 &&
		     ( st->snacSubtype() == 0x0F || st->snacSubtype() == 0x21 ) )
			return true;
		else
			return false;
	}
	
}

bool OwnUserInfoTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if ( !st )
			return false;
		
		Buffer* b = transfer->buffer();
		if ( st->snacSubtype() == 0x0F )
		{
			UserDetails ud;
			ud.fill( b );
			m_details = ud;
			emit gotInfo();
			setSuccess( 0, QString::null );
			return true;
		}
		else
		{
			bool needUpload = false;
			WORD infoType = b->getWord();
			if ( infoType == 0x0000 || infoType == 0x0001 )
			{
				BYTE flags = b->getByte();
				if ( flags == 0x41 )  //we need to do a buddy upload when bit 8 = 1
					needUpload = true;
				
				QByteArray qba;
				if ( b->length() != 0 )
				{ //buffer might be empty if flags bit 8 = 1
					BYTE checksumLength = b->getByte();
					qba.duplicate( b->getBlock( checksumLength ) );
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Self icon checksum: " << qba << endl;
				}
				
				if ( needUpload )
				{
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Buddy icon upload requested" << endl;
					emit buddyIconUploadRequested();
				}
				else
				{
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "no item for hash found" << endl;
				}
			}
			
			if ( infoType == 0x0002 )
			{
				QString availableMsg( b->getBSTR() );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "self available message: " << endl;
			}
			
			setSuccess( 0, QString::null );
			return true;
		}
		
	}

	return false;
}

void OwnUserInfoTask::onGo()
{
	//Send SNAC( 0x01, 0x0E )
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x000E, 0x0000, client()->snacSequence() };
	Buffer *b = new Buffer(); //empty snac data
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

UserDetails OwnUserInfoTask::getInfo() const
{
	return m_details;
}

//kate: tab-width 4; indent-mode csands;

#include "ownuserinfotask.moc"
