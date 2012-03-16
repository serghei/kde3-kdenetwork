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
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../qca/src
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### groupwise_tasks (static) ##################

kde_add_library( groupwise_tasks STATIC_PIC AUTOMOC
  SOURCES
    requesttask.cpp eventtask.cpp logintask.cpp setstatustask.cpp
    statustask.cpp conferencetask.cpp createconferencetask.cpp
    sendmessagetask.cpp getdetailstask.cpp getstatustask.cpp
    typingtask.cpp connectiontask.cpp sendinvitetask.cpp
    joinconferencetask.cpp leaveconferencetask.cpp rejectinvitetask.cpp
    keepalivetask.cpp createcontacttask.cpp modifycontactlisttask.cpp
    createfoldertask.cpp movecontacttask.cpp updateitemtask.cpp
    createcontactinstancetask.cpp deleteitemtask.cpp updatefoldertask.cpp
    updatecontacttask.cpp pollsearchresultstask.cpp privacyitemtask.cpp
    needfoldertask.cpp searchchattask.cpp searchusertask.cpp
    searchusertask.h getchatsearchresultstask.cpp chatcountstask.cpp
    chatpropertiestask.cpp joinchattask.cpp
)
