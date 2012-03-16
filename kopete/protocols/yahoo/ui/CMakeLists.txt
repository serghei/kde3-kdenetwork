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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopeteyahooui (static) ####################

kde_add_library( kopeteyahooui STATIC_PIC AUTOMOC
  SOURCES
    yahooadd.ui yahooeditaccountbase.ui yahooinvitelistbase.ui
    yahooinvitelistimpl.cpp empty.cpp yahooverifyaccountbase.ui
    yahoostealthsetting.ui yahoowebcamdialog.cpp yahoogeneralinfowidget.ui
    yahoouserinfodialog.cpp yahooworkinfowidget.ui
    yahoootherinfowidget.ui
)
