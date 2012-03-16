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
  -DLINUX
  -DPOSIX
  -DINTERNAL_BUILD
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../..
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### cricketp2pclient (static) #################

kde_add_library( cricketp2pclient STATIC_PIC
  SOURCES
    sessionclient.cc basicportallocator.cc socketmonitor.cc
)
