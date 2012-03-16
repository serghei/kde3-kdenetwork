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

add_definitions( -UQT_NO_ASCII_CAST )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES servers.txt servers.ini DESTINATION ${DATA_INSTALL_DIR}/ksirc )


##### ksopenksirc (static) ######################

kde_add_library( ksopenksirc STATIC_PIC AUTOMOC
  SOURCES
    open_ksirc.cpp open_ksircData.ui serverFileParser.cpp enter_combo.cpp
)
