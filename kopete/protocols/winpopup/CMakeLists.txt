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

add_subdirectory( ui )
add_subdirectory( icons )
add_subdirectory( libwinpopup )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/libwinpopup
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kopete_wp.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( PROGRAMS
    winpopup-send.sh winpopup-install.sh
  DESTINATION ${BIN_INSTALL_DIR} )


##### kopete_wp (module) ########################

kde_add_kpart( kopete_wp AUTOMOC
  SOURCES
    wpprotocol.cpp wpcontact.cpp wpaddcontact.cpp wpeditaccount.cpp
    wpaccount.cpp wpuserinfo.cpp
  LINK
    kopetewpui-static winpopup-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
