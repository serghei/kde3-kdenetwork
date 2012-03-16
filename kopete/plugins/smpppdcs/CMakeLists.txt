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
add_subdirectory( libsmpppdclient )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libsmpppdclient
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_smpppdcs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_smpppdcs_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES smpppdcs.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kopete_smpppdcs (module) ##################

kde_add_kpart( kopete_smpppdcs AUTOMOC
  SOURCES
    kinternetiface.stub smpppdcsplugin.cpp onlineinquiry.cpp
    smpppdcsiface.skel detectordcop.cpp detectorsmpppd.cpp
    detectornetstat.cpp detectornetworkstatus.cpp smpppdcsconfig.kcfgc
  LINK smpppdclient-static kopete-shared crypto
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_smpppdcs (module) ##############

kde_add_kpart( kcm_kopete_smpppdcs AUTOMOC
  SOURCES
    smpppdcsprefs.ui smpppdcspreferences.cpp smpppdsearcher.cpp
    smpppdcsprefsimpl.cpp smpppdlocationui.ui smpppdlocationwidget.cpp
    smpppdcsconfig.kcfgc
  LINK  smpppdclient-static kopete-shared crypto
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
