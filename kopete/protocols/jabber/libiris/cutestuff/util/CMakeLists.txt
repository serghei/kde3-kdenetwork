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


##### cutestuff_util (static) ###################

kde_add_library( cutestuff_util STATIC_PIC AUTOMOC
  SOURCES
    base64.cpp bytestream.cpp qrandom.cpp safedelete.cpp sha1.cpp
    showtextdlg.cpp
)
