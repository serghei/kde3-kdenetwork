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

add_definitions( -Dksize_t=socklen_t )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDE_DIR}
)


##### mach (static) #############################

kde_add_library( mach STATIC_PIC
  SOURCES answmach.cpp forwmach.cpp talkconn.cpp
)
