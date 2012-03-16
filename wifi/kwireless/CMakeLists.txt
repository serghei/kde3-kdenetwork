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

install( FILES kwireless.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### libkwireless (module) #####################

kde_add_kpart( libkwireless AUTOMOC
  SOURCES
    kwireless.cpp kwirelesswidget.cpp linuxwirelesswidget.cpp
    propertytablebase.ui propertytable.cpp
  LINK kdeui-shared ${IW_LIBRARY}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
