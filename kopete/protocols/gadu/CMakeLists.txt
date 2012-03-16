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
  ${GADU_INCLUDE_DIRS}
)


##### other data ################################

install( FILES kopete_gadu.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kopete_gadu (module) ######################

kde_add_kpart( kopete_gadu AUTOMOC
  SOURCES
    gaduaway.cpp gadueditcontact.cpp gaducommands.cpp
    gadueditaccount.cpp gadusession.cpp gaducontact.cpp
    gaduaddcontactpage.cpp gaduprotocol.cpp gaduaccount.cpp
    gadupubdir.cpp gaduregisteraccount.cpp gaducontactlist.cpp
    gadurichtextformat.cpp gadudccserver.cpp gadudcctransaction.cpp
    gadudcc.cpp
  LINK gaduui-static kopete-shared ${GADU_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
