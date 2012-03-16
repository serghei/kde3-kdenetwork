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

add_definitions(
  -DPOSIX
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../..
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### cricketp2pbase (static) ###################

kde_add_library( cricketp2pbase STATIC_PIC
  SOURCES
    stun.cc port.cc udpport.cc tcpport.cc helpers.cc sessionmanager.cc
    session.cc p2psocket.cc relayport.cc stunrequest.cc stunport.cc
    socketmanager.cc
)


##### relayserver (executable) ##################

kde_add_executable( relayserver
  SOURCES
    relayserver.cc relayserver_main.cc
  LINK
    cricketbase-static cricketp2pbase-static pthread
  DESTINATION ${BIN_INSTALL_DIR}
)


##### stunserver (executable) ###################

kde_add_executable( stunserver
  SOURCES
    stunserver.cc stunserver_main.cc
  LINK
    cricketbase-static cricketp2pbase-static pthread
  DESTINATION ${BIN_INSTALL_DIR}
)
