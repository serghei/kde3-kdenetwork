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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### knewstickercommon (static) ################

kde_add_library( knewstickercommon STATIC_PIC AUTOMOC
  SOURCES
    newsiconmgr.cpp xmlnewsaccess.cpp configaccess.cpp newsiconmgr.skel
    newsengine.cpp
)
