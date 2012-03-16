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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### oscar (static) ############################

kde_add_library( oscar STATIC_PIC AUTOMOC
  SOURCES
    oscarutils.cpp client.cpp task.cpp connector.cpp
    inputprotocolbase.cpp coreprotocol.cpp flapprotocol.cpp
    snacprotocol.cpp transfer.cpp rtf.cc bytestream.cpp
    oscarclientstream.cpp safedelete.cpp stream.cpp oscarconnector.cpp
    oscarbytestream.cpp buffer.cpp md5.c logintask.cpp aimlogintask.cpp
    icqlogintask.cpp closeconnectiontask.cpp rateclassmanager.cpp
    serverversionstask.cpp rateinfotask.cpp errortask.cpp
    locationrightstask.cpp profiletask.cpp blmlimitstask.cpp
    servicesetuptask.cpp icbmparamstask.cpp ssimanager.cpp rateclass.cpp
    rateclass.h prmparamstask.cpp ssiparamstask.cpp ssilisttask.cpp
    ssiactivatetask.cpp clientreadytask.cpp senddcinfotask.cpp
    sendidletimetask.cpp ownuserinfotask.cpp connection.cpp
    onlinenotifiertask.cpp userdetails.cpp ssimodifytask.cpp
    oscartypeclasses.cpp oscarmessage.cpp messagereceivertask.cpp
    sendmessagetask.cpp icqtask.cpp offlinemessagestask.cpp
    ssiauthtask.cpp userinfotask.cpp icquserinfo.cpp icquserinfotask.cpp
    usersearchtask.cpp warningtask.cpp changevisibilitytask.cpp
    typingnotifytask.cpp buddyicontask.cpp serverredirecttask.cpp
    oscarsettings.cpp chatnavservicetask.cpp connectionhandler.cpp
    chatservicetask.cpp
)
