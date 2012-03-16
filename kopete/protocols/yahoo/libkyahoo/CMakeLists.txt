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

include( ConfigureChecks.cmake )

if( HAVE_INTTYPES_H )
  add_definitions( -DHAVE_INTTYPES_H )
elseif( HAVE_STDINT_H )
  add_definitions( -DHAVE_STDINT_H )
endif()


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kyahoo (static) ###########################

kde_add_library( kyahoo STATIC_PIC AUTOMOC
  SOURCES
    client.cpp task.cpp connector.cpp inputprotocolbase.cpp
    ymsgprotocol.cpp ymsgtransfer.cpp transfer.cpp
    yahoobytestream.cpp bytestream.cpp yahooclientstream.cpp
    yahooconnector.cpp safedelete.cpp stream.cpp sha1.c
    md5.c crypt.c coreprotocol.cpp logintask.cpp libyahoo.c
    yahoo_fn.c listtask.cpp statusnotifiertask.cpp
    mailnotifiertask.cpp messagereceivertask.cpp
    sendnotifytask.cpp sendmessagetask.cpp logofftask.cpp
    changestatustask.cpp modifybuddytask.cpp picturenotifiertask.cpp
    requestpicturetask.cpp yahoobuddyiconloader.cpp
    stealthtask.cpp sendpicturetask.cpp webcamtask.cpp
    conferencetask.cpp sendauthresptask.cpp pingtask.cpp
    yabtask.cpp yabentry.cpp modifyyabtask.cpp chatsessiontask.cpp
    sendfiletask.cpp filetransfernotifiertask.cpp
    receivefiletask.cpp yahoochattask.cpp
)
