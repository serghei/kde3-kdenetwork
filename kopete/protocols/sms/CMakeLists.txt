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
add_subdirectory( services )
add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/services
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_sms.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_sms (module) #######################

kde_add_kpart( kopete_sms AUTOMOC
  SOURCES
    smsaddcontactpage.cpp smscontact.cpp smseditaccountwidget.cpp
    smsprotocol.cpp serviceloader.cpp smsservice.cpp
    smsuserpreferences.cpp smsaccount.cpp
  LINK
    kopetesmsui-static kopetesmsservices-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
