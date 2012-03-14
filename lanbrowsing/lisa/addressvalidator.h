/*    addressvalidator.h
 *
 *    Copyright (c) 2000, Alexander Neundorf
 *    neundorf@kde.org
 *
 *    You may distribute under the terms of the GNU General Public
 *    License as specified in the COPYING file.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 */

#ifndef ADDRESSVALIDATOR_H
#define ADDRESSVALIDATOR_H

#include "lisadefines.h"
#include "mystring.h"
#include "configfile.h"

#define NO_SPEC 0
#define NETMASK_SPEC 1
#define EXACTADDR_SPEC 2
#define RANGE_SPEC 3
#define MULTIRANGE_SPEC 4

struct AddressSpec
{
   int address;
   int mask;
   int typeOfSpec;
};

class AddressValidator
{
   public:
      AddressValidator(const MyString& addressSpecs);
      AddressValidator();
      ~AddressValidator();
      void configure(Config& config);
      void setValidAddresses(MyString addressSpecs);
      void clearSpecs();
      int isValid(int addressNBO);
      MyString validAddresses() {return allowedHosts;};
   private:
      int localhostNet;
      int localhostMask;
      MyString allowedHosts;
      void addSpec(int type, int address, int mask=0);
      AddressSpec specs[MAX_SPECS];
};

#endif
