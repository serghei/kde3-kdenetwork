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

add_subdirectory( pixmaps )
add_subdirectory( icons )
add_subdirectory( logview )
add_subdirectory( Rules )
add_subdirectory( DB )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES Kppp.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kppp (executable) #########################

kde_add_executable( kppp SETUID AUTOMOC
  SOURCES
    kpppwidget.cpp general.cpp accounts.cpp connect.cpp conwindow.cpp
    debug.cpp edit.cpp iplined.cpp main.cpp modem.cpp modemcmds.cpp
    pppdargs.cpp pppdata.cpp scriptedit.cpp pwentry.cpp modeminfo.cpp
    pppstatdlg.cpp pppstats.cpp miniterm.cpp accounting.cpp
    acctselect.cpp ruleset.cpp docking.cpp runtests.cpp loginterm.cpp
    ppplog.cpp newwidget.cpp requester.cpp opener.cpp modemdb.cpp
    utils.cpp providerdb.cpp modems.cpp KPPPIface.skel
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
