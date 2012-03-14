/***************************************************************************
                          led.h  -  description
                             -------------------
    begin                : wo okt 8 2003
    copyright            : (C) 2003 by Stefan Winter
    email                : mail@stefan-winter.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASUSLED_H
#define ASUSLED_H

#include <fstream>

/**
  *@author Roeland Merks
  */

class Led
{
public:
  Led ();
  ~Led ();
  void On (void);
  void Off (void);
  bool state;

private:
    std::ofstream * led;
};

#endif
