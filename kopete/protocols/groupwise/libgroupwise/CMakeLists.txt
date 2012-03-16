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

add_subdirectory( qca )
add_subdirectory( tasks )

# add_definitions(
#   -DUSE_TLSHANDLER
# )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/qca/src
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### groupwise (static) ########################

kde_add_library( groupwise STATIC_PIC AUTOMOC
  SOURCES
    bytestream.cpp chatroommanager.cpp client.cpp connector.cpp
    coreprotocol.cpp eventprotocol.cpp eventtransfer.cpp
    gwclientstream.cpp gwerror.cpp gwfield.cpp gwglobal.cpp
    inputprotocolbase.cpp privacymanager.cpp qcatlshandler.cpp
    request.cpp requestfactory.cpp response.cpp responseprotocol.cpp
    rtf.cc safedelete.cpp securestream.cpp stream.cpp task.cpp
    tlshandler.cpp transfer.cpp transferbase.cpp userdetailsmanager.cpp
    usertransfer.cpp
)


##### gwtest (static) ###########################

kde_add_library( gwtest STATIC_PIC AUTOMOC
  SOURCES
    coreprotocol.cpp eventtransfer.cpp gwfield.cpp request.cpp
    requestfactory.cpp transfer.cpp usertransfer.cpp client.cpp task.cpp
    safedelete.cpp gwclientstream.cpp qcatlshandler.cpp stream.cpp
    tlshandler.cpp response.cpp connector.cpp securestream.cpp
    bytestream.cpp
)
