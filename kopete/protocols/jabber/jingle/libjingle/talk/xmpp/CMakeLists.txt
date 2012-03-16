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

add_definitions(
  -DPOSIX
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDE_DIR}
)


##### cricketxmpp (static) ######################

kde_add_library( cricketxmpp STATIC_PIC
  SOURCES
    constants.cc jid.cc saslmechanism.cc xmppclient.cc xmppengineimpl.cc
    xmppengineimpl_iq.cc xmpplogintask.cc xmppstanzaparser.cc xmpptask.cc
)
