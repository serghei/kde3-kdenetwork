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

add_subdirectory( common )
add_subdirectory( kntsrcfilepropsdlg )
add_subdirectory( knewstickerstub )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES knewsticker.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS knt-0.1-0.2.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/knewsticker )
install( FILES knewsticker.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES knewsticker-standalone.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### knewsticker_panelapplet (module) ##########

kde_add_kpart( knewsticker_panelapplet AUTOMOC
  SOURCES
    newsscroller.cpp knewsticker.cpp knewsticker.skel
    knewstickerconfig.cpp knewstickerconfigwidget.ui
    newssourcedlgimpl.cpp newssourcedlg.ui
  LINK knewstickercommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
