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


# check for aRts
if( WITH_ARTS )
  pkg_search_module( ARTS arts )
  if( NOT ARTS_FOUND )
    kde_message_fatal( "aRts is requested, but was not found on your system" )
  endif( )
else( )
  set( WITHOUT_ARTS 1 CACHE INTERNAL "" FORCE )
endif( )


# check for wireless tools
check_library_exists( iw iw_get_stats "" HAVE_IW )
if( HAVE_IW )
  check_include_file( iwlib.h HAVE_IWLIB_H )
  # we need at least wireless tools >= 27
  add_definitions( -DHAVE_IW_27 -DHAVE_IW_27pre19 )
endif( )
if( HAVE_IW AND HAVE_IWLIB_H )
  set( IW_LIBRARY iw )
else( )
  kde_message_fatal( "wifi support needs wireless tools >= 27, but was not found on your system" )
endif( )
