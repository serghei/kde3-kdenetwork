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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_latex.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_latex_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES kopete_latexconvert.sh DESTINATION ${BIN_INSTALL_DIR} )
install( FILES latexconfig.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES latexchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_latex )


##### kopete_latex (module) #####################

kde_add_kpart( kopete_latex AUTOMOC
  SOURCES
    latexplugin.cpp latexconfig.kcfgc latexguiclient.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_latex (module) #######################

kde_add_kpart( kcm_kopete_latex AUTOMOC
  SOURCES
    latexprefsbase.ui latexpreferences.cpp latexconfig.kcfgc
  LINK kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
