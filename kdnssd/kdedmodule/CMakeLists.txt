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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES dnssdwatcher.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_dnssdwatcher (module) ################

kde_add_kpart( kded_dnssdwatcher AUTOMOC
  SOURCES dnssdwatcher.cpp dnssdwatcher.skel watcher.cpp
  LINK kdnssd-shared kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
