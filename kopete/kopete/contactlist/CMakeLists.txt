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

# FIXME no XRENDER support yet

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kopete/libkopete/ui
  ${CMAKE_BINARY_DIR}/kopete/kopete/addcontactwizard
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/private
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/kopete
  ${CMAKE_SOURCE_DIR}/kopete/kopete/addcontactwizard
  ${CMAKE_SOURCE_DIR}/kopete/kopete/chatwindow
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetecontactlist (static) ################

kde_add_library( kopetecontactlist STATIC_PIC AUTOMOC
  SOURCES
    kopetemetacontactlvi.cpp kopetestatusgroupviewitem.cpp
    kopetegroupviewitem.cpp kopetecontactlistview.cpp
    kopetegvipropswidget.ui kopetemetalvipropswidget.ui
    kopetelviprops.cpp kopeteaddrbookexport.cpp
    kopeteaddrbookexportui.ui customnotifications.ui
    customnotificationprops.cpp kopetegrouplistaction.cpp
    kabcexport.cpp kabcexport_base.ui
)
