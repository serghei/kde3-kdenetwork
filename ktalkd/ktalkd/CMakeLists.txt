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

add_subdirectory( machines )

add_definitions( -DHAVE_KDE )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ktalkd.wav DESTINATION ${SOUND_INSTALL_DIR} )
install( FILES ktalkdrc DESTINATION ${CONFIG_INSTALL_DIR} )


##### ktalkd (executable) #######################

kde_add_executable( ktalkd
  SOURCES
    print.c repairs.c announce.cpp process.cpp readcfg++.cpp table.cpp
    talkd.cpp find_user.cpp threads.cpp options.cpp unixsock.cpp
  LINK mach-static kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
