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
  ${CMAKE_CURRENT_SOURCE_DIR}/../jabber
  ${CMAKE_CURRENT_SOURCE_DIR}/../xmpp-core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cutestuff/util
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### iris_xmpp_im (static) #####################

add_custom_command( OUTPUT types.moc
  COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/types.cpp -o types.moc
  COMMAND sed -i -e '/^\#include \"/d' types.moc
  DEPENDS types.cpp )

set_source_files_properties( types.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/types.moc )

kde_moc( MOCS xmpp_tasks.h )

kde_add_library( iris_xmpp_im STATIC_PIC
  SOURCES
    client.cpp types.cpp xmpp_tasks.cpp xmpp_vcard.cpp xmpp_xmlcommon.cpp ${MOCS}
)
