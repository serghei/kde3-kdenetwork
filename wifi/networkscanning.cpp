/***************************************************************************
                          networkscanning.cpp  -  description
                             -------------------
    begin                : Sam Apr 24 11:44:20 CEST 2005
    copyright            : (C) 2005 by Stefan Winter
    email                : swinter@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <qstring.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtable.h>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <ktempfile.h>

#include "networkscanning.h"
#include "interface_wireless.h"

NetworkScanning::NetworkScanning (Interface_wireless * dev, QWidget * parent, const char * name ) : QWidget ( parent, name ) {
  device = dev;
  networkScan();
}

void
NetworkScanning::networkScan ()
{
  networks = device->get_available_networks ();

  if ( networks->numRows() > 0 ) {

      networks->setColumnReadOnly( 0, true);
      networks->setColumnReadOnly( 1, true);
      networks->setColumnReadOnly( 2, true);
      networks->setColumnReadOnly( 3, false);
      for ( int i = 0; i < networks->numRows(); i++) {
	if ( networks->text( i, 3 ) == i18n( "off" )) networks->setRowReadOnly( i, true );
      }

      networks->setSelectionMode(QTable::SingleRow);
      connect(networks,SIGNAL(selectionChanged()),this,SLOT(checkWEP()));

      this->setCaption( i18n( "Scan Results" ) );

      QGridLayout* networkSelectionLayout = new QGridLayout ( this, 2, 3, 0, 5);
      switchNet = new QPushButton( i18n( "Switch to Network..." ), this );
      switchNet->setEnabled(false);
      QPushButton* close = new QPushButton( i18n( "Close" ), this );

      networks->reparent( this, QPoint( 0, 0 ) );
      networks->setLeftMargin( 0 );
      networks->verticalHeader()->hide();

      connect ( close, SIGNAL( clicked() ), this, SLOT( hide() ) );
      connect ( switchNet, SIGNAL( clicked() ), this, SLOT( switchToNetwork() ) );
      connect ( networks, SIGNAL( currentChanged(int,int)), this, SLOT( checkSettings(int,int)));
      connect ( networks, SIGNAL( valueChanged(int,int)), this, SLOT( checkSettings(int,int)));

      networkSelectionLayout->addMultiCellWidget( networks, 0, 0, 0, 2 );
      networkSelectionLayout->addWidget( switchNet, 1, 0 );
      networkSelectionLayout->addWidget( close, 1, 2 );

      this->show();

    } else
    {
      KMessageBox::sorry(0,i18n("The scan is complete, but no networks have been found."),i18n("No Network Available"));
    }
}

void NetworkScanning::checkSettings(int row, int)
{
	if ((networks->text(row,0)!=i18n("(hidden cell)")) && (checkWEP()!=INVALID)) switchNet->setEnabled(true);
	else switchNet->setEnabled(false);
}

WEP_KEY
NetworkScanning::checkWEP()
{
  kdDebug() << "In checkWEP()\n";
  if (  (networks->text( networks->currentRow() , 3 ) == i18n( "off" )) ||
	(networks->text( networks->currentRow() , 3 ) == "" ) ) return NONE;
  if (  (networks->text( networks->currentRow() , 3 ).length()== 5 ) || 
	(networks->text( networks->currentRow() , 3 ).length()== 13 ) ) return VALID_STRING;
  if (  (networks->text( networks->currentRow() , 3 ).length()== 10 ) ||
        (networks->text( networks->currentRow() , 3 ).length()== 26 ) ) return VALID_HEX;
  return INVALID;
}

void
NetworkScanning::switchToNetwork()
{

  WEP_KEY encryption = checkWEP();

  if (encryption == INVALID) {
    KMessageBox::sorry(0,i18n( "Aborting network switching due to invalid WEP key specification." ), i18n( "Invalid WEP Key" ));
    return;
  }

  QString cmdline;

  KTempFile* tempfile = new KTempFile( QString::null, QString::null, 0700 );
  QString tempfilename = tempfile->name();

  cmdline = (QString)"ifconfig %1 down\n";
  cmdline = cmdline.arg( device->get_interface_name() );
  write( tempfile->handle(), cmdline.ascii(), strlen( cmdline.ascii() ) );

  cmdline = (QString)"iwconfig %1 essid %2 mode %3 enc %4\n";
  cmdline = cmdline.arg( device->get_interface_name() );
  cmdline = cmdline.arg( KProcess::quote( networks->text( networks->currentRow(), 0 ) ) );

  QString modetemp;
  if (networks->text( networks->currentRow(), 1 ) == i18n("Managed") ) modetemp = "Managed"; else modetemp = "Ad-Hoc"; 
  cmdline = cmdline.arg( modetemp );

  if ( encryption != NONE ) {
    cmdline = cmdline.arg( (encryption == VALID_STRING ? "s:" : "" ) + KProcess::quote( networks->text( networks->currentRow(), 3 ) ) );
  } else {
    cmdline = cmdline.arg("off");
  }
  write( tempfile->handle(), cmdline.ascii(), strlen( cmdline.ascii() ) );

  cmdline = (QString)"ifconfig %1 up\n";
  cmdline = cmdline.arg( device->get_interface_name() );
  write( tempfile->handle(), cmdline.ascii(), strlen( cmdline.ascii() ) );

  delete tempfile; // autoDeletion off, so the file remains on disk

  KProcess switchProc;
  switchProc << "kdesu" << tempfilename;
  switchProc.start( KProcess::Block );

  remove(tempfilename.ascii());

}


#include "networkscanning.moc"
