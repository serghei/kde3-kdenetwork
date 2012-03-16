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

# FIXME no support for GLLIB yet

add_definitions( -DKDE_NO_COMPAT )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kvideoio (static) #########################

kde_add_library( kvideoio STATIC_PIC AUTOMOC
  SOURCES kxv.cpp qvideo.cpp qvideostream.cpp
)


##### kopete_videodevice (shared) ###############

kde_add_library( kopete_videodevice SHARED AUTOMOC
  SOURCES
    videocontrol.cpp videodevice.cpp videodevicemodelpool.cpp
    videodevicepool.cpp videoinput.cpp bayer.cpp
    sonix_compress.cpp
  VERSION 1.0.0
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
