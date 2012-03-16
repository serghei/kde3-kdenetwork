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

add_subdirectory( ui )
add_subdirectory( icons )
add_subdirectory( libiris )
add_subdirectory( kioslave )

if( WITH_JINGLE )
  add_subdirectory( jingle )
  set( SUPPORT_JINGLE 1 CACHE INTERNAL "" FORCE )
  set( JINGLE_LIBRARIES
    kopetejabberjingle-static cricketsessionphone-static cricketxmllite-static
    cricketp2pclient-static cricketxmpp-static cricketp2pbase-static cricketbase-static
    mediastreamer-static ortp-static
    ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${GMODULE2_LIBRARIES}
    ${EXPAT_LIBRARY} ${SPEEX_LIBRARIES} pthread
  )
endif( )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_BINARY_DIR}/jingle
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/jabber
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-im
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/qca/src
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/util
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/network
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/jingle
  ${CMAKE_CURRENT_SOURCE_DIR}/../../libkopete
  ${CMAKE_CURRENT_SOURCE_DIR}/../../libkopete/ui
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_jabber.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES jabberchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber )


##### jabberclient (static) #####################

kde_add_library( jabberclient STATIC_PIC AUTOMOC
  SOURCES
    jabberclient.cpp jabberconnector.cpp jabberbytestream.cpp
)


##### kopete_jabber (module) ####################

kde_add_kpart( kopete_jabber AUTOMOC
  SOURCES
    jabberprotocol.cpp jabberaccount.cpp jabberresource.cpp
    jabberresourcepool.cpp jabberbasecontact.cpp jabbercontact.cpp
    jabbergroupcontact.cpp jabbergroupmembercontact.cpp
    jabbercontactpool.cpp jabberformtranslator.cpp jabberformlineedit.cpp
    jabberchatsession.cpp jabbergroupchatmanager.cpp
    jabberfiletransfer.cpp jabbercapabilitiesmanager.cpp
    jabbertransport.cpp jabberbookmarks.cpp
  LINK
    jabberclient-static kopetejabberui-static
    iris_jabber-static iris_xmpp_core-static iris_xmpp_im-static iris-static
    qca-static cutestuff_network-static cutestuff_util-static ${JINGLE_LIBRARIES}
    kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
