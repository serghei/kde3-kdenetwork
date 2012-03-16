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
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../qca/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cutestuff/util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cutestuff/network
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### iris_xmpp_core (static) ########################

add_custom_command( OUTPUT securestream.moc
  COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/securestream.cpp -o securestream.moc.cpp
  COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/securestream.h -o securestream.moc.h
  COMMAND sed -i -e '/^\#include/d' securestream.moc.cpp
  COMMAND cat securestream.moc.h securestream.moc.cpp > securestream.moc
  DEPENDS securestream.cpp )

set_source_files_properties( securestream.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/securestream.moc )

kde_add_library( iris_xmpp_core STATIC_PIC
  SOURCES
    connector.cpp jid.cpp securestream.cpp tlshandler.cpp hash.cpp
    protocol.cpp stream.cpp xmlprotocol.cpp parser.cpp simplesasl.cpp
)
