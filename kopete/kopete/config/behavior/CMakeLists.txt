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

install( FILES kopete_behaviorconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kcm_kopete_behaviorconfig (module) ########

kde_add_kpart( kcm_kopete_behaviorconfig AUTOMOC
  SOURCES
    kopeteawayconfigbase.ui behaviorconfig_chat.ui
    behaviorconfig_general.ui behaviorconfig_events.ui
    behaviorconfig.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
