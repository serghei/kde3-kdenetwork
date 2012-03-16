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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_cryptography.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_cryptography_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES cryptographyui.rc cryptographychatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_cryptography )


##### kopete_cryptography (module) ##############

kde_add_kpart( kopete_cryptography AUTOMOC
  SOURCES
    cryptographyplugin.cpp kgpginterface.cpp
    cryptographyguiclient.cpp cryptographyselectuserkey.cpp
    cryptographyuserkey_ui.ui popuppublic.cpp kgpgselkey.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_cryptography (module) ##########

kde_add_kpart( kcm_kopete_cryptography AUTOMOC
  SOURCES
    cryptographypreferences.cpp cryptographyprefsbase.ui
    kgpgselkey.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
