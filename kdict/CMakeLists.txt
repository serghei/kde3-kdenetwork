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

# FIXME KDE_CXXFLAGS = $(USE_THREADS)

add_subdirectory( applet )
add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES kdict.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kdictui.rc DESTINATION ${DATA_INSTALL_DIR}/kdict )


##### kdict (kdeinit) ###########################

kde_add_kdeinit_executable( kdict AUTOMOC
  SOURCES
    dcopinterface.skel main.cpp actions.cpp dict.cpp options.cpp
    queryview.cpp toplevel.cpp sets.cpp matchview.cpp application.cpp
  LINK khtml-shared pthread
)
