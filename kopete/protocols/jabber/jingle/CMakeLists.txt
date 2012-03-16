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

add_subdirectory( libjingle )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libjingle
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/jabber
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/util
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetejabberjingle (static) ###############

kde_moc( MOCS voicecaller.h )

kde_add_library( kopetejabberjingle STATIC_PIC AUTOMOC
  SOURCES
    jinglevoicecaller.cpp jinglevoicesessiondialogbase.ui
    jinglevoicesessiondialog.cpp ${MOCS}
)
