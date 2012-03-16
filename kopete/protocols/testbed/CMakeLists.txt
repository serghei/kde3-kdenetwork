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
add_subdirectory( ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_testbed.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_testbed (module) ###################

kde_add_kpart( kopete_testbed AUTOMOC
  SOURCES
    testbedprotocol.cpp testbedcontact.cpp testbedaccount.cpp
    testbedaddcontactpage.cpp testbedaddui.ui
    testbededitaccountwidget.cpp testbedaccountpreferences.ui
    testbedfakeserver.cpp testbedincomingmessage.cpp
  LINK kopetetestbedui-static kopete-shared kopete_videodevice-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
