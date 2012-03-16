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

pkg_search_module( MEANWHILE meanwhile )
if( NOT MEANWHILE_FOUND )
  kde_message_fatal( "meanwhile is required, but was not found on your system" )
endif( )
