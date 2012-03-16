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
  ${CMAKE_CURRENT_BINARY_DIR}/../../../libkopete/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/jabber
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/qca/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../libkopete
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../libkopete/ui
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopetejabberui (static) ###################

kde_add_library( kopetejabberui STATIC_PIC AUTOMOC
  SOURCES
    dlgsendraw.ui dlgjabbersendraw.cpp dlgaddcontact.ui
    jabberaddcontactpage.cpp dlgvcard.ui dlgjabbervcard.cpp
    dlgjabberservices.cpp dlgregister.ui dlgjabberregister.cpp
    dlgbrowse.ui dlgjabberbrowse.cpp dlgjabbereditaccountwidget.ui
    jabbereditaccountwidget.cpp dlgjabberregisteraccount.ui
    jabberregisteraccount.cpp dlgjabberchooseserver.ui
    jabberchooseserver.cpp dlgchangepassword.ui
    dlgjabberchangepassword.cpp empty.cpp dlgchatroomslist.ui
    dlgjabberchatroomslist.cpp dlgchatjoin.ui dlgjabberchatjoin.cpp
    dlgservices.ui
)
