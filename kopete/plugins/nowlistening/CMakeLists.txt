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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_nowlistening.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_nowlistening_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )
install( FILES nowlisteningconfig.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES nowlisteningui.rc nowlisteningchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete )


##### kopete_nowlistening (module) ##############

kde_add_kpart( kopete_nowlistening AUTOMOC
  SOURCES
    nowlisteningconfig.kcfgc nowlisteningplugin.cpp nlkscd.cpp
    nlnoatun.cpp nlxmms.cpp nowlisteningguiclient.cpp nljuk.cpp
    nlamarok.cpp nlkaffeine.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_kopete_nowlistening (module) ##########

kde_add_kpart( kcm_kopete_nowlistening AUTOMOC
  SOURCES
    nowlisteningprefs.ui nowlisteningpreferences.cpp
    nowlisteningconfig.kcfgc
  LINK kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
