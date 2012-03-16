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
  ${QT_INCLUDE_DIR}
)


##### iris (static) #############################

kde_moc( SRCS im.h xmpp.h )

kde_add_library( iris STATIC_PIC
  SOURCES ${SRCS}
)
