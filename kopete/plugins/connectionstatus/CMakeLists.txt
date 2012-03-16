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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_connectionstatus.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_connectionstatus (module) ##########

kde_add_kpart( kopete_connectionstatus AUTOMOC
  SOURCES connectionstatusplugin.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
