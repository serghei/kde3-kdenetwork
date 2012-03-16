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
  ${CMAKE_CURRENT_SOURCE_DIR}/../libgroupwise
  ${CMAKE_CURRENT_SOURCE_DIR}/../libgroupwise/qca/src
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetegroupwiseui (static) ################

kde_add_library( kopetegroupwiseui STATIC_PIC AUTOMOC
  SOURCES
    gwaccountpreferences.ui gwaddcontactpage.cpp gwaddui.ui
    gweditaccountwidget.cpp gwreceiveinvitationdialog.cpp
    gwshowinvitation.ui gwcontactpropswidget.ui gwcontactproperties.cpp
    gwprivacy.ui gwprivacydialog.cpp gwsearch.cpp gwcustomstatuswidget.ui
    gwcustomstatusedit.ui gwcontactsearch.ui gwchatsearchwidget.ui
    gwchatsearchdialog.cpp gwchatpropswidget.ui gwchatpropsdialog.cpp
)
