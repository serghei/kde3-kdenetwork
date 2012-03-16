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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_translator.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_translator_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES translatorui.rc translatorchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_translator )


##### kopete_translator (module) ################

kde_add_kpart( kopete_translator AUTOMOC
  SOURCES
    translatordialog.cpp translatorguiclient.cpp
    translatorlanguages.cpp translatorplugin.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_translator (module) ############

kde_add_kpart( kcm_kopete_translator AUTOMOC
  SOURCES
    translatorprefsbase.ui translatorprefs.cpp
    translatorlanguages.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
