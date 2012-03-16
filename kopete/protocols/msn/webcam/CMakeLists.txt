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

add_subdirectory( libmimic )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${GLIB2_INCLUDE_DIRS}
)


##### mimicwrapper (static) #####################

kde_add_library( mimicwrapper STATIC_PIC AUTOMOC
  SOURCES mimicwrapper.cpp msnwebcamdialog.cpp
  LINK mimic-static
)
