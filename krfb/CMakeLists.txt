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

include( ConfigureChecks.cmake )

add_subdirectory( srvloc )
add_subdirectory( kinetd )
add_subdirectory( libvncserver )
add_subdirectory( krfb )
add_subdirectory( kcm_krfb )
add_subdirectory( krfb_httpd )
