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

kde_import( librss )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/librss
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kntsrcfilepropsdlg.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### kntsrcfilepropsdlg (module) ###############

kde_add_kpart( kntsrcfilepropsdlg AUTOMOC
  SOURCES
    kntsrcfilepropsdlg.cpp kntsrcfilepropsdlgwidget.ui
  LINK knewstickercommon-static rss-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
