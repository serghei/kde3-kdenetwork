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
  ${CMAKE_SOURCE_DIR}/kopete/kopete/addaccountwizard
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_accountconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kcm_kopete_accountconfig (module) #########

kde_add_kpart( kcm_kopete_accountconfig AUTOMOC
  SOURCES kopeteaccountconfigbase.ui kopeteaccountconfig.cpp
  LINK kopeteaddaccountwizard-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
