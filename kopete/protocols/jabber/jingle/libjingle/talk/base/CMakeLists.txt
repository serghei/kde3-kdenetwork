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
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### cricketbase (static) ######################

kde_add_library( cricketbase STATIC_PIC
  SOURCES
    socketaddress.cc jtime.cc asyncudpsocket.cc messagequeue.cc
    thread.cc physicalsocketserver.cc bytebuffer.cc asyncpacketsocket.cc
    network.cc asynctcpsocket.cc socketadapters.cc md5c.c base64.cc
    task.cc taskrunner.cc host.cc socketaddresspair.cc
)
