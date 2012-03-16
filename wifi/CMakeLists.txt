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

include( ConfigureChecks.cmake )

add_subdirectory( kcmwifi )
add_subdirectory( kwireless )
add_subdirectory( pixmaps )
add_subdirectory( locations )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)

if( WITH_ARTS )
  set( ARTS_LIBRARIES artsflow;mcop )
endif( )


##### other data ################################

kde_install_icons( )
install( FILES kwifimanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/kwifimanager )
install( FILES kwifimanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kwifimanager (executable) #################

kde_add_executable( kwifimanager AUTOMOC
  SOURCES
    interface_wireless.cpp interface_wireless_wirelessextensions.cpp
    interface_dcop.skel networkscanning.cpp kwifimanager.cpp locator.cpp
    main.cpp picture.cpp speed.cpp statistics.cpp status.cpp strength.cpp
    asusled.cpp
  LINK kdeui-shared ${ARTS_LIBRARIES} ${IW_LIBRARY}
  DESTINATION ${BIN_INSTALL_DIR}
)
