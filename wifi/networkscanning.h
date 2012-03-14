
/***************************************************************************
          networkscanning.h  -  header for the NetworkScanning class
                             -------------------
    begin                : Sam Apr  24 11:44:20 CEST 2005
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

#ifndef NETWORKSCANNING_H
#define NETWORKSCANNING_H

#include <qwidget.h>

class QTable;
class QPushButton;
class Interface_wireless;

enum WEP_KEY { NONE, VALID_HEX, VALID_STRING, INVALID };

class NetworkScanning : public QWidget

{

  Q_OBJECT 

  public:
  NetworkScanning (Interface_wireless * dev, QWidget * parent = 0, const char * name = 0 );

  protected:
  void networkScan ();
  QTable* networks;
  Interface_wireless * device;
  QPushButton* switchNet;

  protected slots:
  void checkSettings(int row, int);
  WEP_KEY checkWEP();
  void switchToNetwork();

};

#endif /* NETWORKSCANNING_H */
