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

# NOTE some files was renamed:
# kopetemessagemanager.h -> kopetechatsession.h
# kopetemessagemanagerfactory.h -> kopetechatsessionmanager.h

include( ConfigureChecks.cmake )

add_subdirectory( libkopete )
add_subdirectory( kopete )
add_subdirectory( protocols )
add_subdirectory( plugins )
add_subdirectory( icons )
add_subdirectory( sounds )
add_subdirectory( styles )
