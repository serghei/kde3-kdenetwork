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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/avdevice
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( DESTINATION ${DATA_INSTALL_DIR}/kopete/icons )
install( FILES kopete_avdeviceconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kcm_kopete_avdeviceconfig (module) ########

kde_add_kpart( kcm_kopete_avdeviceconfig AUTOMOC
  SOURCES
    avdeviceconfig.cpp avdeviceconfig_videoconfig.ui
  LINK kopete_videodevice-shared kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
