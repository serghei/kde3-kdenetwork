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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/private
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/kopete/chatwindow
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_appearanceconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kcm_kopete_appearanceconfig (module) ######

kde_add_kpart( kcm_kopete_appearanceconfig AUTOMOC
  SOURCES
    appearanceconfig.cpp appearanceconfig_chatwindow.ui
    appearanceconfig_colors.ui appearanceconfig_contactlist.ui
    appearanceconfig_emoticons.ui
    tooltipeditdialog.cpp
    tooltipeditwidget.ui
  LINK kopetechatwindow-static kopete-shared khtml-shared knewstuff-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
