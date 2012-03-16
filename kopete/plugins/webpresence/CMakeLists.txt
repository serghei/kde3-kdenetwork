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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kopete_webpresence.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    kopete_webpresence_config.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )

install( FILES
    webpresence_html.xsl webpresence_html_images.xsl
    webpresence_xhtml.xsl webpresence_xhtml_images.xsl
  DESTINATION ${DATA_INSTALL_DIR}/kopete/webpresence )


##### kopete_webpresence (module) ###############

kde_add_kpart( kopete_webpresence AUTOMOC
  SOURCES
    webpresenceplugin.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_webpresence (module) ###########

kde_add_kpart( kcm_kopete_webpresence AUTOMOC
  SOURCES
    webpresencepreferences.cpp webpresenceprefs.ui
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
