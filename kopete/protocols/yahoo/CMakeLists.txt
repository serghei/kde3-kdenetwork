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

add_subdirectory( libkyahoo )
add_subdirectory( ui )
add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libkyahoo
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES yahooconferenceui.rc yahoochatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_yahoo )
install( FILES kopete_yahoo.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_yahoo (module) #####################

kde_add_kpart( kopete_yahoo AUTOMOC
  SOURCES
    yahooprotocol.cpp yahooeditaccount.cpp yahooaddcontact.cpp
    yahooaccount.cpp yahoocontact.cpp yahooconferencemessagemanager.cpp
    yahoochatsession.cpp yahooverifyaccount.cpp yahoowebcam.cpp
  LINK kyahoo-static kopeteyahooui-static kopeteui-static kopete-shared kopete_videodevice-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
