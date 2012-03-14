/*    configfile.cpp
 *
 *    Copyright (c) 1998, 2000, Alexander Neundorf
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

#include "configfile.h"

#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LISA_DEBUG
#undef LISA_DEBUG
#endif
#define LISA_DEBUG 0

#ifdef dcerr
#undef dcerr
#endif

#define dcerr if (LISA_DEBUG==1) std::cerr<<"Config::"


#define CFGBUFSIZE 16*1024

Config::Config(const MyString& name/*,String path*/)
{
   char buff[CFGBUFSIZE],c;
/*   String s,empty="#############################################################################################################################################################";
   String home=getenv("HOME");

   if (!home.empty()) home+=String("/")+name;
   if (fexists(home)==0)
   {
      home=path+"/"+name;
      if (fexists(home)==0) 
      {
         home=name;
         if (fexists(home)==0) return;
      };
   };*/
   std::ifstream inf(name.data());
   if (!inf)
   {
      std::cout<<"could not open file "<<name<<std::endl;
      return;
   }
   dcerr<<"Config(): opened file "<<name<<std::endl;
   //read the file
   char key[CFGBUFSIZE], value[CFGBUFSIZE];
   do
	{
      char* buffStart=buff;
		//inf.getline(buff,16*1024,'\n');
      int bufSize(CFGBUFSIZE);
      int lineBroken(0);
      do
      {
         lineBroken=0;
         inf.get(buffStart,bufSize,'\n');
         inf.get(c);
         int l=strlen(buffStart);
         if (l==0)
            break;
         if (buffStart[l-1]=='\\')
         {
            buffStart=buffStart+l-1;
            bufSize=bufSize+1-l;
            lineBroken=1;
         }
      } while ((lineBroken) && (!inf.eof()));
      //make it ignore comments
      char *theChar=strchr(buff,'#');
      if (theChar!=0)
         *theChar='\0';
      //now divide the line into key and value
      theChar=strchr(buff,'=');
      if (theChar!=0)
      {
         *theChar='\0';
         key[0]='\0';
         sscanf(buff,"%8000s",key);
         //do we have something valid ?
         if (key[0]!='\0')
         {
            //the char behind the = should be at least the terminating \0
            // so I can be sure to access valid memory here, IMO
            value[0]='\0';

            strncpy(value,theChar+1,CFGBUFSIZE);
            if (value[0]!='\0')
            {
               //here we can be sure that the list will only contain
               //strings which are at least one char long
               dcerr<<"Config(): adding "<<key<<std::endl;
               m_entries[key]=value;
            }
         }
      }
	}
   while (!inf.eof());
}

MyString Config::getEntry(const char *key, const char* defaultValue)
{
   if ((key==0) || (key[0]=='\0'))
      return defaultValue;
   if (m_entries.find(key)==m_entries.end())
      return defaultValue;
   return m_entries[key];
}

int Config::getEntry(const char *key, int defaultValue)
{
   char def[100];
   sprintf(def,"%d",defaultValue);
   MyString tmp=stripWhiteSpace(getEntry(key,def));
   int i(0);
   int ok=sscanf(tmp.c_str(),"%d",&i);
   if (ok==1) return i;
   return defaultValue;
}

