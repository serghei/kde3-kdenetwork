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

add_definitions( -DKDE_NO_COMPAT )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopeteprivate (static) ####################

kde_add_library( kopeteprivate STATIC_PIC AUTOMOC
  SOURCES
    kopeteemoticons.cpp kopetecommand.cpp
    kopeteviewmanager.cpp kopeteutils_private.cpp
)
