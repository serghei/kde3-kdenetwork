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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )

install( FILES
    kget_plug_in.rc kget_plug_in.desktop
  DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins )


##### khtml_kget (module) #######################

kde_add_kpart( khtml_kget AUTOMOC
  SOURCES kget_plug_in.cpp kget_linkview.cpp links.cpp
  LINK khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
