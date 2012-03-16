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

add_subdirectory( ui )
add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${MEANWHILE_INCLUDE_DIRS}
)


##### other data ################################

install( FILES kopete_meanwhile.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### new_target0 (module) #######################

kde_add_kpart( new_target0 AUTOMOC
  SOURCES
    meanwhileprotocol.cpp meanwhileaddcontactpage.cpp
    meanwhileeditaccountwidget.cpp meanwhileaccount.cpp
    meanwhilecontact.cpp meanwhilesession.cpp meanwhileplugin.cpp
  LINK kopetemeanwhileui-static kopete-shared ${MEANWHILE_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
