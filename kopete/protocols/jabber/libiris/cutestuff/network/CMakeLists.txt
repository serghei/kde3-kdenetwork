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
  ${CMAKE_CURRENT_SOURCE_DIR}/../util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../qca/src
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### cutestuff_network (static) ################

kde_add_library( cutestuff_network STATIC_PIC AUTOMOC
  SOURCES
    bsocket.cpp httpconnect.cpp httppoll.cpp ndns.cpp servsock.cpp
    socks.cpp srvresolver.cpp
)
