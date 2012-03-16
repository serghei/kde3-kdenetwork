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

add_subdirectory( images )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${SQLITE_INCLUDE_DIRS}
)


##### other data ################################

install( FILES kopete_statistics.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES statisticsui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_statistics )


##### kopete_statistics (module) ################

kde_add_kpart( kopete_statistics AUTOMOC
  SOURCES
    statisticsplugin.cpp statisticsdb.cpp statisticsdialog.cpp
    statisticswidget.ui statisticscontact.cpp statisticsdcopiface.skel
  LINK kopete-shared khtml-shared ${SQLITE_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
