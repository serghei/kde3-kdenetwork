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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kppplogview.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kppplogview (executable) ##################

kde_add_executable( kppplogview AUTOMOC
  SOURCES
    main.cpp loginfo.cpp log.cpp monthly.cpp export.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
