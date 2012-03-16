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

# find kdelibs
# FIXME should be more flexibile
set( CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/cmake" )
if( NOT EXISTS "${CMAKE_MODULE_PATH}/FindKDE3.cmake" )
  message( FATAL_ERROR
      "\n"
      "####################################################################\n "
      "${CMAKE_MODULE_PATH}/FindKDE3.cmake does not exists.\n "
      "Please pass corect CMAKE_INSTALL_PREFIX to cmake.\n"
      "####################################################################\n" )
endif( )
include( FindKDE3 )


if( BUILD_KOPETE OR BUILD_KPPP )
  include( CheckIncludeFile )
  check_include_file( string.h HAVE_STRING_H )
endif( )
