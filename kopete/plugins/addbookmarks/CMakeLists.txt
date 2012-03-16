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

install( FILES kopete_addbookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_addbookmarks_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )


##### kopete_addbookmarks (module) ##############

kde_add_kpart( kopete_addbookmarks AUTOMOC
  SOURCES
    addbookmarksplugin.cpp addbookmarksprefssettings.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_addbookmarks (module) ##########

kde_add_kpart( kcm_kopete_addbookmarks AUTOMOC
  SOURCES
    addbookmarkspreferences.cpp addbookmarksprefsui.ui
    addbookmarksprefssettings.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
