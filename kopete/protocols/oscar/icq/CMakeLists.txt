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
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../liboscar
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_icq.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES x-icq.desktop DESTINATION ${MIME_INSTALL_DIR}/application )


##### kopete_icq (module) #######################

kde_add_kpart( kopete_icq AUTOMOC
  SOURCES
    icqpresence.cpp icqaccount.cpp icqcontact.cpp icqprotocol.cpp
  LINK
    kopeteicqui-static kopete_oscar-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
