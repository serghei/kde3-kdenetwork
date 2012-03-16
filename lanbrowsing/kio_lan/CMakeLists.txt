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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    lan.protocol rlan.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    lan.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/dirtree/remote )

install( FILES
    lan.desktop
  DESTINATION ${DATA_INSTALL_DIR}/remoteview )

install( FILES
    lisa.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )


##### kio_lan (module) ##########################

kde_add_kpart( kio_lan
  SOURCES kio_lan.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
