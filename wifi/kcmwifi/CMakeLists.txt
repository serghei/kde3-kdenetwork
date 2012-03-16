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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmwifi.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_wifi (module) #########################

kde_add_kpart( kcm_wifi AUTOMOC
  SOURCES
    wificonfig.cpp kcmwifi.cpp configcrypto.ui ifconfigpagebase.ui
    configpower.ui mainconfigbase.ui mainconfig.cpp ifconfigpage.cpp
    vendorconfig.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
