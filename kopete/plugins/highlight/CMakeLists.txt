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

add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_highlight.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_highlight_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )


##### kopete_highlight (module) #################

kde_add_kpart( kopete_highlight AUTOMOC
  SOURCES
    highlightplugin.cpp highlightconfig.cpp filter.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_highlight (module) #############

kde_add_kpart( kcm_kopete_highlight AUTOMOC
  SOURCES
    highlightprefsbase.ui highlightpreferences.cpp
    filter.cpp highlightconfig.cpp
  LINK kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
