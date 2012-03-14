/*    addressvalidator.cpp
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

#include "addressvalidator.h"
#include "mystring.h"

#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

#ifdef LISA_DEBUG
#undef LISA_DEBUG
#endif
#define LISA_DEBUG 0
#define dcerr if (LISA_DEBUG==1) std::cerr<<"AddressValidator::"

AddressValidator::AddressValidator(const MyString& addressSpecs)
//this is 127.0.0.0
:localhostNet(htonl(0x7f000000))
//with mask 255.255.255.0
,localhostMask(htonl(0xffffff00))
{
   clearSpecs();
   MyString tmp=addressSpecs;
   setValidAddresses(tmp);
}

AddressValidator::AddressValidator()
   //this is 127.0.0.0
   :localhostNet(htonl(0x7f000000))
   //with mask 255.255.255.0
   ,localhostMask(htonl(0xffffff00))
{
   clearSpecs();
}

AddressValidator::~AddressValidator()
{}

void AddressValidator::configure(Config& config)
{
   MyString tmp=stripWhiteSpace(config.getEntry("AllowedAddresses",""));
   tmp=tmp+";";
   setValidAddresses(tmp);
   dcerr<<"configure(): "<<tmp<<std::endl;
}


void AddressValidator::setValidAddresses(MyString addressSpecs)
{
   dcerr<<"setValidAddresses"<<std::endl;
   allowedHosts=addressSpecs;
   MyString nextPart;
   while (!addressSpecs.isEmpty())
   {
      dcerr<<"setValidAddresses: "<<addressSpecs<<std::endl;
      int pos=addressSpecs.find(";");
      nextPart=addressSpecs.left(pos);
      addressSpecs=addressSpecs.mid(pos+1);
      dcerr<<"setValidAddresses: nextPart: "<<nextPart<<std::endl;
      if ((nextPart.contains('.')==3) && (nextPart.contains('-')==0))
      {
         addSpec(EXACTADDR_SPEC,inet_addr(nextPart.data()));
         dcerr<<"setValidAddresses: exact addr: "
	 <<std::ios::hex<<inet_addr(nextPart.data())<<std::ios::dec<<std::endl;
      }
      else if ((nextPart.contains('-')==1) && (nextPart.contains('.')==6))
      {
         int p2=nextPart.find("-");
         MyString from=nextPart.left(p2);
         MyString to=nextPart.mid(p2+1);
         addSpec(RANGE_SPEC,ntohl(inet_addr(from.data())),ntohl(inet_addr(to.data())));
      }
      else if ((nextPart.contains('-')==4) && (nextPart.contains('.')==3))
      {
         unsigned int i1=0;
         unsigned int i2=0;
         int p2=0;
         MyString rest=nextPart+'.';
         MyString digit;

         for (int i=0; i<4; i++)
         {
            p2=rest.find("-");
            digit=rest.left(p2);
            i1=i1<<8;
            i1+=atoi(digit.data());
            rest=rest.mid(p2+1);
            p2=rest.find(".");
            digit=rest.left(p2);
            i2=i2<<8;
            i2+=atoi(digit.data());
            rest=rest.mid(p2+1);
         };
         addSpec(MULTIRANGE_SPEC,i1,i2);
      }
      else
      {
         pos=nextPart.find('/');
         MyString netStr=nextPart.left(pos);
         MyString maskStr=nextPart.mid(pos+1);
         int mask=inet_addr(maskStr.data());
         int net= (inet_addr(netStr.data()) & mask);
         dcerr<<"setValidAddresses: net/mask: "
	 <<std::ios::hex<<net<<"/"<<mask<<std::ios::dec<<std::endl;
         addSpec(NETMASK_SPEC,net,mask);
      }
   }
}

void AddressValidator::clearSpecs()
{
   allowedHosts="";
   for (int i=0; i<MAX_SPECS; i++)
   {
      specs[i].address=0;
      specs[i].mask=0;
      specs[i].typeOfSpec=NO_SPEC;
   }
}

void AddressValidator::addSpec(int type, int address, int mask)
{
   for (int i=0; i<MAX_SPECS; i++)
   {
      if (specs[i].typeOfSpec==NO_SPEC)
      {
         specs[i].address=address;
         specs[i].mask=mask;
         specs[i].typeOfSpec=type;
         return;
      }
   }
}

int AddressValidator::isValid(int addressNBO)
{
   dcerr<<"isValid: "
      <<std::ios::hex<<addressNBO<<std::ios::dec<<std::endl;
   //localhost is always allowed
   dcerr<<"isValid() local net: "<<
   std::ios::hex<<localhostNet<<" mask: "<<localhostMask<<" AND: "<<(addressNBO &
   localhostMask)<<std::ios::dec<<std::endl;
   if ((addressNBO & localhostMask) == localhostNet)
      return 1;
      
   for (int i=0; i<MAX_SPECS; i++)
   {
      if (specs[i].typeOfSpec==NO_SPEC)
      {
         //since the specifications are always entered from the beginning
         //of the array, we already passed the last one if we get here
         //so we can return now "it is invalid !" ;-)
         return 0;
         //continue;
      }
      else if (specs[i].typeOfSpec==EXACTADDR_SPEC)
      {
         dcerr<<"isValid: comparing "
	 <<std::ios::hex<<specs[i].address<<std::ios::dec<<std::endl;
         if (addressNBO==specs[i].address)
         {
            dcerr<<"isValid: exact address"<<std::endl;
            return 1; // this one is allowed to :-)
         }
      }
      else if (specs[i].typeOfSpec==NETMASK_SPEC)
      {
         dcerr<<"isValid: ANDing "<<
	 std::ios::hex<<(addressNBO & specs[i].mask)<<" "<<
    specs[i].address<<std::ios::dec<<std::endl;
         if ((addressNBO & specs[i].mask) == specs[i].address)
         {
            dcerr<<"isValid: net/mask"<<std::endl;
            return 1;
         }
      }
      else if (specs[i].typeOfSpec==RANGE_SPEC)
      {
         if ((ntohl(addressNBO)>=specs[i].address) && (ntohl(addressNBO)<=specs[i].mask))
         {
            dcerr<<"isValid: range"<<std::endl;
            return 1;
         }
      }
      else if (specs[i].typeOfSpec==MULTIRANGE_SPEC)
      {
         unsigned int addr=ntohl(addressNBO);
         dcerr<<"isValid ntohl="<<hex<<addr<<" addr: "<<specs[i].address<<" ma: "<<specs[i].mask<<dec<<std::endl;
         unsigned int mask=0x000000ff;
         int failure=0;
         for (int j=0; j<4; j++)
         {
            dcerr<<"isValid "<<hex<<"mask="<<mask<<" addr="<<(addr&mask)<<" addr="<<(specs[i].address & mask)<<" ma="<<(specs[i].mask&mask)<<std::endl;
            if (((addr & mask) < (specs[i].address & mask))
               || ((addr & mask) > (specs[i].mask & mask)))
            {
               failure=1;
               break;
            }
            mask=mask<<8;
         }
         dcerr<<"isValid: multirange"<<std::endl;
         if (!failure)
            return 1;
      }
   }
   //if ((addressNBO==htonl(0x0a040801)) || (addressNBO==htonl(0xc0a80001))) return 0;
   dcerr<<"isValid: invalid address"<<std::endl;
   return 0;
}

