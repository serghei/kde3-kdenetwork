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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../liboscar
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kopete_aim.desktop aim.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_aim (module) #######################

kde_add_kpart( kopete_aim AUTOMOC
  SOURCES
    aimprotocol.cpp aimaccount.cpp aimcontact.cpp aimuserinfo.cpp
    aimjoinchat.cpp aimchatsession.cpp
  LINK
    kopeteaimui-static kopete_oscar-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
