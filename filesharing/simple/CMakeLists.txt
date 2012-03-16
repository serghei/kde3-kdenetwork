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
  ${CMAKE_CURRENT_SOURCE_DIR}/../advanced/propsdlgplugin
  ${CMAKE_CURRENT_BINARY_DIR}/../advanced/propsdlgplugin
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES fileshare.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_fileshare (module) ####################

kde_add_kpart( kcm_fileshare AUTOMOC
  SOURCES
    fileshare.cpp controlcenter.ui groupconfiggui.ui groupconfigdlg.cpp
    krichtextlabel.cpp
  LINK
    propsdlgplugin_common-static filesharenfs-static filesharesamba-static
    kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
