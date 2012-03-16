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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### rdp (static) ##############################

kde_add_library( rdp STATIC_PIC AUTOMOC
  SOURCES krdpview.cpp rdphostpref.cpp rdpprefs.ui
)
