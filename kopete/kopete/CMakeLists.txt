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

add_subdirectory( addaccountwizard )
add_subdirectory( addcontactwizard )
add_subdirectory( contactlist )
add_subdirectory( chatwindow )
add_subdirectory( config )
add_subdirectory( kconf_update )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_BINARY_DIR}/kopete/libkopete
  ${CMAKE_BINARY_DIR}/kopete/kopete/addcontactwizard
  ${CMAKE_BINARY_DIR}/kopete/kopete/contactlist
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/private
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/avdevice
  ${CMAKE_SOURCE_DIR}/kopete/kopete/addaccountwizard
  ${CMAKE_SOURCE_DIR}/kopete/kopete/addcontactwizard
  ${CMAKE_SOURCE_DIR}/kopete/kopete/contactlist
  ${CMAKE_SOURCE_DIR}/kopete/kopete/config/plugins
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES kopete.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kopeteui.rc eventsrc DESTINATION ${DATA_INSTALL_DIR}/kopete )
install( FILES x-kopete-emoticons.desktop DESTINATION ${MIME_INSTALL_DIR}/application )


##### kopete (executable) #######################

kde_add_executable( kopete AUTOMOC
  SOURCES
    main.cpp kopeteapplication.cpp kopeteiface.cpp
    kopeteiface.skel systemtray.cpp kopeteballoon.cpp
    kopetewindow.cpp kopeteaccountstatusbaricon.cpp
    kimifaceimpl.cpp kimiface.skel groupkabcselectorwidget.ui
    kopeteeditglobalidentitywidget.cpp
  LINK
    kopetecontactlist-static kopeteaddaccountwizard-static
    kopeteaddcontactwizard-static kopeteui-static
    kopetepluginconfig-static kopete-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
