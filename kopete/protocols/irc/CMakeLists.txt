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

add_subdirectory( icons )
add_subdirectory( libkirc )
add_subdirectory( ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/libkirc
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kopete_irc.desktop irc.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    ircnetworks.xml ircchatui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kopete )


##### kopete_irc (module) #######################

kde_add_kpart( kopete_irc AUTOMOC
  SOURCES
    ircaccount.cpp ircaddcontactpage.cpp ircchannelcontact.cpp
    irccontact.cpp ircguiclient.cpp ircprotocol.cpp ircservercontact.cpp
    ircsignalhandler.cpp irctransferhandler.cpp ircusercontact.cpp
    irccontactmanager.cpp kcodecaction.cpp ksparser.cpp
  LINK
    kopeteircui-static kirc-static kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
