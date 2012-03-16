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

add_definitions( -DKDE_NO_COMPAT )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/private
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    addressbooklinkwidget.h kopetefileconfirmdialog.h
    kopetepasswordwidget.h kopeteview.h addcontactpage.h
    kopeteviewplugin.h editaccountwidget.h kopetecontactaction.h
    kopetestdaction.h userinfodialog.h addressbookselectordialog.h
    addressbookselectorwidget.h kopetelistview.h accountselector.h
    kopetelistviewitem.h kopetelistviewsearchline.h
    ${CMAKE_CURRENT_BINARY_DIR}/fileconfirmbase.h
    ${CMAKE_CURRENT_BINARY_DIR}/kopeteawaydialogbase.h
    ${CMAKE_CURRENT_BINARY_DIR}/kopetepasswordwidgetbase.h
    ${CMAKE_CURRENT_BINARY_DIR}/kopetepassworddialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/ui )


##### kopeteui (static) #########################

kde_add_library( kopeteui STATIC_PIC AUTOMOC
  SOURCES
    kopetecontactaction.cpp addcontactpage.cpp editaccountwidget.cpp
    kopetepassworddialog.ui kopetestdaction.cpp kopeteawaydialogbase.ui
    kopetefileconfirmdialog.cpp fileconfirmbase.ui
    userinfodialog.cpp kopeteview.cpp kopetepasswordwidgetbase.ui
    kopetepasswordwidget.cpp accountselector.cpp kopeteviewplugin.cpp
    addresseeitem.cpp addressbookselectorwidget_base.ui
    addressbookselectordialog.cpp addressbookselectorwidget.cpp
    metacontactselectorwidget_base.ui metacontactselectorwidget.cpp
    kopetelistview.cpp kopetelistviewitem.cpp kopetelistviewsearchline.cpp
    contactaddednotifywidget.ui contactaddednotifydialog.cpp
    addressbooklinkwidget_base.ui addressbooklinkwidget.cpp
)
