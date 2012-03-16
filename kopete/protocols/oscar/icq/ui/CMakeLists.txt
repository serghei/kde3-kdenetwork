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
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../liboscar
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopeteicqui (static) ######################

kde_add_library( kopeteicqui STATIC_PIC AUTOMOC
  SOURCES
    icqadd.ui icqeditaccountui.ui icqeditaccountwidget.cpp
    icqgeneralinfo.ui icqotherinfowidget.ui icqworkinfowidget.ui
    icqinterestinfowidget.ui icquserinfowidget.cpp icqauthreplyui.ui
    icqauthreplydialog.cpp icqaddcontactpage.cpp icqsearchbase.ui
    icqsearchdialog.cpp
)
