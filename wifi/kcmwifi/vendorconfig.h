/*
  Copyright (C) 2005 Stefan Winter <swinter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef VENDORCONFIG_H
#define VENDORCONFIG_H

class KCMWifi;

class VendorConfig {
	public:
	VendorConfig( KCMWifi* caller);
	void initAll ();
	void initSuSE_92plus ();
	void initDebian ();
	protected:
	KCMWifi* m_caller;
	int m_totalConfigs;
};

#endif // VENDORCONFIG_H
