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

add_definitions( ${KDE_PLUGIN} )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_history.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES historyui.rc  historychatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_history )
install( FILES kopete_history_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES historyconfig.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kopete_history (module) ###################

kde_add_kpart( kopete_history AUTOMOC
  SOURCES
    historyplugin.cpp historydialog.cpp historyviewer.ui
    historylogger.cpp converter.cpp historyguiclient.cpp
    historyconfig.kcfgc
  LINK kopete-shared khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_history (module) ###############

kde_add_kpart( kcm_kopete_history AUTOMOC
  SOURCES
    historyprefsui.ui historypreferences.cpp
    historyconfig.kcfgc
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
