#ifndef MYSTRING_H
#define MYSTRING_H

#include <string>
#include <string.h>
#include <strings.h>
#include <stdio.h>

class MyString: public std::string
{
   public:
      MyString(const char c) : std::string(1,c) {};
      MyString(const char* c)
         : std::string(c==0?"":c)
      {};
      MyString(const std::string& s) : std::string(s) {};
      MyString() : std::string() {};
      
      int isEmpty() const {return empty();};
      MyString mid(int pos, int length=-1) const {return substr(pos,length);};
      MyString left(int len) const {return substr(0,len);};
      MyString right(int len) const
      {
         if (len<1) return "";
         else if (len<int(size())) return substr(size()-len);
         else return (*this);
      };

      int contains(char c);
      //char & operator[] (unsigned int i)    {return ((string)(*this))[i];}
      //operator const char* () const     {return c_str();}
      const char* data() const     {return c_str();}
      /*void setNum(int value) {char c[15]; bzero(c,15); sprintf(c,"%d",value); (*this)=c;};
      void setNum(double value) {char c[25]; bzero(c,25); sprintf(c,"%g",value); (*this)=c;};
      void simplifyWhiteSpace();*/
};

//taken from Qt/QCString
MyString stripWhiteSpace(MyString str);

#endif

