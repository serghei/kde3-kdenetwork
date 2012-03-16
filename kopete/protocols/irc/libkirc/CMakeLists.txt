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
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kirc (static) #############################

kde_add_library( kirc STATIC_PIC AUTOMOC
  SOURCES
    kircengine.cpp kircengine_commands.cpp kircengine_ctcp.cpp
    kircengine_numericreplies.cpp kircentity.cpp kircmessage.cpp
    kircmessageredirector.cpp kirctransfer.cpp kirctransferhandler.cpp
    kirctransferserver.cpp ksslsocket.cpp
)
