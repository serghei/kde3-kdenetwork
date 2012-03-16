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
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    knewstickerstub.desktop
  DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### knewstickerstub (executable) ##############

kde_add_executable( knewstickerstub AUTOMOC
  SOURCES knewstickerstub.cpp
  LINK knewstickercommon-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
