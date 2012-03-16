#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include( CheckLibraryExists )
include( FindPkgConfig )


# gthread-2.0
kde_search_module( GTHREAD2 gthread-2.0 )

# gmodule-2.0
kde_search_module( GMODULE2 gmodule-2.0 )

# speex
if( WITH_SPEEX )
  pkg_search_module( SPEEX speex )
  if( SPEEX_FOUND )
    set( HAVE_SPEEX 1 CACHE INTERNAL "" FORCE )
    if( NOT SPEEX_INCLUDE_DIRS )
      set( SPEEX_INCLUDE_DIRS "/usr/include/speex" )
    endif( )
  else( )
    kde_message_fatal( "speex is required, but was not found on your system" )
  endif( )
endif( )

# expat
check_include_file( expat.h HAVE_EXPAT_H )
if( HAVE_EXPAT_H )
  check_library_exists( expat XML_ParserCreate "" HAVE_EXPAT )
endif( )
if( HAVE_EXPAT_H AND HAVE_EXPAT )
  set( EXPAT_LIBRARY expat CACHE INTERNAL "" FORCE )
else( )
  kde_message_fatal( "expat is required, but was not found on your system" )
endif( )
