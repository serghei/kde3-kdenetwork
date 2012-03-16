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
  ${CMAKE_CURRENT_SOURCE_DIR}/../xmpp-im
  ${CMAKE_CURRENT_SOURCE_DIR}/../xmpp-core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cutestuff/util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cutestuff/network
  ${CMAKE_CURRENT_SOURCE_DIR}/../../qca/src
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### iris_jabber (static) ######################

add_custom_command( OUTPUT s5b.moc
  COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/s5b.cpp -o s5b.moc.cpp
  COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/s5b.h -o s5b.moc.h
  COMMAND sed -i -e '/^\#include/d' s5b.moc.cpp
  COMMAND cat s5b.moc.h s5b.moc.cpp > s5b.moc
  DEPENDS s5b.cpp )

set_source_files_properties( s5b.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/s5b.moc )

kde_add_library( iris_jabber STATIC_PIC AUTOMOC
  SOURCES
    filetransfer.cpp s5b.cpp xmpp_ibb.cpp xmpp_jidlink.cpp all_mocs.cpp
)
