#include "mystring.h"

#include <ctype.h>

//this one is taken from Qt/QCString

MyString stripWhiteSpace(MyString str)
{
   if ( str.isEmpty() )				// nothing to do
      return "";

   char const *s = str.data();
   MyString result = s;
   int reslen = result.length();
   if ( !isspace(s[0]) && !isspace(s[reslen-1]) )
      return result;				// returns a copy

   s = result.data();
   int start = 0;
   int end = reslen - 1;
   while ( isspace(s[start]) )			// skip white space from start
      start++;
   if ( s[start] == '\0' )
   {			// only white space
      result.resize( 1 );
      return "";
   }

   while ( end && isspace(s[end]) )		// skip white space from end
      end--;

   end -= start - 1;

   result=str.mid(start,end);
   //memmove( result.data(), &s[start], end );
   //result.resize( end + 1 );
   return result;
}

//mainly taken from qcstring
int MyString::contains(char c)
{
   int count = 0;
   char const *d = c_str();
   if ( d==0 )
      return 0;
   while ( *d )
      if ( *d++ == c )
         count++;
   return count;
}

