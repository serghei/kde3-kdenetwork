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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetewpui (static) #######################

kde_add_library( kopetewpui STATIC_PIC AUTOMOC
  SOURCES
    wpaddcontactbase.ui wpeditaccountbase.ui empty.cpp
    wpuserinfowidget.ui
)
