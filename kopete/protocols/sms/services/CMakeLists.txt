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

# FIXME KDE_CXXFLAGS = $(USE_EXCEPTIONS)

include( ConfigureChecks.cmake )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetesmsservices (static) ################

kde_add_library( kopetesmsservices STATIC_PIC AUTOMOC
  SOURCES
    smssend.cpp smssendprefs.ui smssendprovider.cpp smsclient.cpp
    smsclientprefs.ui gsmlib.cpp gsmlibprefs.ui kopete_unix_serial.cpp
  LINK ${GSM_LIBRARY}
)
