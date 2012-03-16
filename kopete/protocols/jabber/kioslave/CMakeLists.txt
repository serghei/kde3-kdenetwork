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
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/jabber
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im
  ${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/util
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES jabberdisco.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_jabberdisco (module) ##################

kde_add_kpart( kio_jabberdisco AUTOMOC
  SOURCES jabberdisco.cpp
  LINK
    jabberclient-static
    iris_xmpp_core-static iris_xmpp_im-static iris_jabber-static iris-static
    qca-static cutestuff_network-static cutestuff_util-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
