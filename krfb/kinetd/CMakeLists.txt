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
  ${CMAKE_CURRENT_SOURCE_DIR}/../srvloc
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kinetdmodule.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES kinetd.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/kinetd )


##### kded_kinetd (module) ######################

kde_add_kpart( kded_kinetd AUTOMOC
  SOURCES kinetd.cpp kinetd.skel
  LINK
    srvloc-static kdnssd-shared kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
