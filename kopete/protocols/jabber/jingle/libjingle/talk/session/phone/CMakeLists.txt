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
  ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/ortp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/mediastreamer
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${GLIB2_INCLUDE_DIRS}
  ${SPEEX_INCLUDE_DIRS}
)


##### cricketsessionphone (static) ##############

kde_add_library( cricketsessionphone STATIC_PIC
  SOURCES
    audiomonitor.cc channelmanager.cc voicechannel.cc call.cc
    phonesessionclient.cc linphonemediaengine.cc
)
