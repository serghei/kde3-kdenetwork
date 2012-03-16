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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopeteaddaccountwizard (static) ###########

kde_add_library( kopeteaddaccountwizard STATIC_PIC AUTOMOC
  SOURCES
    addaccountwizardpage1.ui addaccountwizardpage2.ui
    addaccountwizardpage3.ui addaccountwizard.cpp
)
