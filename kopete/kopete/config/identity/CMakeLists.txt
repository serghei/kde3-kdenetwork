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
  ${CMAKE_BINARY_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_identityconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopeteidentityconfigpreferences.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kcm_kopete_identityconfig (module) ########

kde_add_kpart( kcm_kopete_identityconfig AUTOMOC
  SOURCES
    kopeteidentityconfigbase.ui kopeteidentityconfig.cpp
    globalidentitiesmanager.cpp kopeteidentityconfigpreferences.kcfgc
  LINK kopeteui-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
