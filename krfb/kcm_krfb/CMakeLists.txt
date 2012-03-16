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
  ${CMAKE_CURRENT_BINARY_DIR}/../krfb
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmkrfb.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_krfb (module) #########################

kde_add_kpart( kcm_krfb AUTOMOC
  SOURCES configurationwidget.ui kcm_krfb.cpp
  LINK krfbconfig-static srvloc-static kdnssd-shared kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
