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
add_subdirectory( libgroupwise )
add_subdirectory( ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/libgroupwise
  ${CMAKE_CURRENT_SOURCE_DIR}/libgroupwise/tasks
  ${CMAKE_CURRENT_SOURCE_DIR}/libgroupwise/qca/src
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_groupwise.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES gwchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_groupwise )


##### kopete_groupwise (module) #################

kde_add_kpart( kopete_groupwise AUTOMOC
  SOURCES
    gwprotocol.cpp gwcontact.cpp gwaccount.cpp gwbytestream.cpp
    gwconnector.cpp gwmessagemanager.cpp gwcontactlist.cpp
  LINK
    kopetegroupwiseui-static groupwise-static groupwise_tasks-static
    groupwise_qca-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
