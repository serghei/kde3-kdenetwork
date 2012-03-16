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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_autoreplace.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_autoreplace_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )


##### kopete_autoreplace (module) ###############

kde_add_kpart( kopete_autoreplace AUTOMOC
  SOURCES
    autoreplaceplugin.cpp autoreplaceconfig.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_autoreplace (module) ###########

kde_add_kpart( kcm_kopete_autoreplace AUTOMOC
  SOURCES
    autoreplacepreferences.cpp autoreplaceconfig.cpp autoreplaceprefs.ui
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
