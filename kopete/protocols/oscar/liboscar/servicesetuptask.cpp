/*
	Kopete Oscar Protocol
	servicesetuptask.cpp - Set up the services for the BOS connection
	
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
#include "servicesetuptask.h"

#include <kdebug.h>
#include "blmlimitstask.h"
#include "connection.h"
#include "clientreadytask.h"
#include "icbmparamstask.h"
#include "locationrightstask.h"
#include "ownuserinfotask.h"
#include "prmparamstask.h"
#include "profiletask.h"
#include "senddcinfotask.h"
#include "sendidletimetask.h"
#include "ssiactivatetask.h"
#include "ssilisttask.h"
#include "ssimanager.h"
#include "ssiparamstask.h"
#include "transfer.h"

ServiceSetupTask::ServiceSetupTask( Task* parent )
	: Task( parent )
{
	m_finishedTaskCount = 0;
	m_locRightsTask = new LocationRightsTask( parent );
	m_profileTask = new ProfileTask( parent );
	m_blmLimitsTask = new BLMLimitsTask( parent );
	m_icbmTask = new ICBMParamsTask( parent );
	m_prmTask = new PRMParamsTask( parent );
	m_ssiParamTask = new SSIParamsTask( parent );
	m_ssiListTask = new SSIListTask( parent );
	m_ssiActivateTask = new SSIActivateTask( parent );

	QObject::connect( m_ssiListTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_ssiParamTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_prmTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_icbmTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_blmLimitsTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_profileTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_locRightsTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
	QObject::connect( m_ssiActivateTask, SIGNAL( finished() ), this, SLOT( childTaskFinished() ) );
}


ServiceSetupTask::~ServiceSetupTask()
{
	delete m_locRightsTask;
	delete m_profileTask;
	delete m_blmLimitsTask;
	delete m_icbmTask;
	//delete m_prmTask;
	//delete m_ssiParamTask;
	delete m_ssiListTask;
}


bool ServiceSetupTask::forMe( const Transfer* transfer ) const
{
	Q_UNUSED( transfer );
	return false;
}

bool ServiceSetupTask::take( Transfer* transfer )
{
	Q_UNUSED( transfer );
	return false;
}

void ServiceSetupTask::childTaskFinished()
{
	m_finishedTaskCount++;
	
// 	kdDebug( OSCAR_RAW_DEBUG ) << "Finished count is " << m_finishedTaskCount << endl;
	
	if ( m_finishedTaskCount == 7 )
	{
		if ( client()->ssiManager()->listComplete() )
			m_ssiActivateTask->go( true );

		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Sending DC info and client ready" << endl;
		SendIdleTimeTask* sitt = new SendIdleTimeTask( client()->rootTask() );
		QValueList<int> familyList;
		familyList.append( 0x0001 );
		familyList.append( 0x0002 );
		familyList.append( 0x0003 );
		familyList.append( 0x0004 );
		familyList.append( 0x0006 );
		familyList.append( 0x0008 );
		familyList.append( 0x0009 );
		familyList.append( 0x000A );
		familyList.append( 0x0013 );
		ClientReadyTask* crt = new ClientReadyTask( client()->rootTask() );
		crt->setFamilies( familyList );
		sitt->go( true );
		crt->go( true ); //autodelete
	}
	
	if ( m_finishedTaskCount == 8 )
	{
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Service setup finished" << endl;
		setSuccess( 0, QString::null );	
	}
}


void ServiceSetupTask::onGo()
{
	m_locRightsTask->go();
	m_profileTask->go();
	m_blmLimitsTask->go();
	m_icbmTask->go();
	m_prmTask->go( true );
	m_ssiParamTask->go( true );
	m_ssiListTask->go();
}

//kate: tab-width 4; indent-mode csands;

#include "servicesetuptask.moc"
