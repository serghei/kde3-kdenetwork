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

if( WITH_GSM )

  include( CheckIncludeFileCXX )
  check_include_file_cxx( gsmlib/gsm_util.h HAVE_GSMLIB_GSM_UTIL_H )

  if( HAVE_GSMLIB_GSM_UTIL_H )
    set( INCLUDE_SMSGSM 1 )
    find_library( GSM_LIBRARY gsmme )
  endif( )

  if( NOT (HAVE_GSMLIB_GSM_UTIL_H AND GSM_LIBRARY) )
    kde_message_fatal( "gsmlib is required, but was not found on your system" )
  endif( )

endif( )

configure_file( config.h.cmake config.h @ONLY )
