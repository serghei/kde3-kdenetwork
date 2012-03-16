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

if( NOT BUILD_KOPETE_PROTOCOL_MSN )
  kde_message_fatal( "netmeeting plugin needs msn protocol.\n Add -DBUILD_KOPETE_PROTOCOL_MSN=ON to cmake flags." )
endif( )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/protocols/msn
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_netmeeting.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_netmeeting_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES netmeetingchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_netmeeting )


##### kopete_netmeeting (module) ################

kde_add_kpart( kopete_netmeeting AUTOMOC
  SOURCES
    netmeetingplugin.cpp netmeetinginvitation.cpp
    netmeetingguiclient.cpp
  LINK kopete_msn_shared-shared kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_netmeeting (module) ############

kde_add_kpart( kcm_kopete_netmeeting AUTOMOC
  SOURCES
    netmeetingprefs_ui.ui netmeetingpreferences.cpp
  LINK kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
