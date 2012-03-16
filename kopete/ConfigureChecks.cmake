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

# glib-2.0 (jabber, msn)
if( (BUILD_KOPETE_PROTOCOL_JABBER AND WITH_JINGLE) OR (BUILD_KOPETE_PROTOCOL_MSN AND WITH_WEBCAM) )
  include( FindPkgConfig )
  pkg_search_module( GLIB2 glib-2.0 )
  if( GLIB2_FOUND )
    set( HAVE_GLIB 1 CACHE INTERNAL "" FORCE )
  else( )
    kde_message_fatal( "glib-2.0 is required, but was not found on your system" )
  endif( )
endif( )


# video
check_include_file( linux/videodev.h HAVE_V4L )
if( NOT HAVE_V4L )
  check_include_file( libv4l1-videodev.h HAVE_LIBV4L1_VIDEODEV_H )
endif( )
if( NOT HAVE_V4L AND NOT HAVE_LIBV4L1_VIDEODEV_H )
  kde_message_fatal( "Your system doesn't support V4L1. `libv4l` is installed?" )
endif( )
