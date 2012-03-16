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

# FIXME no support for XSS and XRENDER yet

add_subdirectory( private )
add_subdirectory( ui )
add_subdirectory( avdevice )

add_definitions( -DKDE_NO_COMPAT )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/private
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### headers ###################################

install( FILES
    kopeteaccount.h kopeteaccountmanager.h kopeteawayaction.h
    kopeteawaydialog.h kopeteaway.h kopetecommandhandler.h
    kopetecontact.h kopetecontactlistelement.h kopetecontactlist.h
    kopetecontactproperty.h kopeteeventpresentation.h
    kopete_export.h kopeteglobal.h kopetegroup.h kopetemessageevent.h
    kopetemessage.h kopetemessagehandlerchain.h kopetemessagehandler.h
    kopetechatsession.h kopetechatsessionmanager.h kopetemetacontact.h
    kopetemimetypehandler.h kopeteonlinestatus.h kopeteonlinestatusmanager.h
    kopetepasswordedaccount.h kopetepassword.h kopeteplugin.h
    kopeteprotocol.h kopetesimplemessagehandler.h kopetetask.h
    kopetetransfermanager.h kopeteuiglobal.h kabcpersistence.h
    managedconnectionaccount.h kopetenotifydataobject.h
    kopeteversion.h kopeteprefs.h kopetepicture.h webcamwidget.h
    kopetepluginmanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kopete )


##### other data ################################

install( FILES kopete.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kopetecommandui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete )
install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### kopete (shared) ###########################

kde_add_library( kopete SHARED AUTOMOC
  SOURCES
    knotification.cpp connectionmanager.cpp kopeteonlinestatus.cpp
    kopeteonlinestatusmanager.cpp kopeteprotocol.cpp
    kopetecontact.cpp kopetepluginmanager.cpp kopeteplugin.cpp
    kopetemessage.cpp kopetechatsession.cpp kopetechatsessionmanager.cpp
    kopetecontactlist.cpp kopetemetacontact.cpp kopeteawaydialog.cpp
    kopetetransfermanager.cpp kopetegroup.cpp kcautoconfigmodule.cpp
    kopeteaccountmanager.cpp kopeteaccount.cpp kopetecontactlistelement.cpp
    kopetecommandhandler.cpp kopeteaway.cpp kopeteawayaction.cpp
    kautoconfig.cpp kopetewalletmanager.cpp kopetecontactproperty.cpp
    kopetepassword.cpp kopeteglobal.cpp kopeteuiglobal.cpp
    kopetepasswordedaccount.cpp kopetemimetypehandler.cpp
    kopetetask.cpp kopetemimesourcefactory.cpp kopeteeventpresentation.cpp
    kopetenotifyevent.cpp kopetenotifydataobject.cpp kopeteblacklister.cpp
    kopetemessageevent.cpp kopetemessagehandler.cpp kopetemessagehandlerchain.cpp
    kopetesimplemessagehandler.cpp kopeteproperties.cpp kabcpersistence.cpp
    connectionmanager.skel clientiface.stub managedconnectionaccount.cpp
    networkstatuscommon.h kopeteconfig.kcfgc kopeteutils.cpp
    kopeteprefs.cpp kopetepicture.cpp webcamwidget.cpp
  VERSION 1.0.0
  EMBED kopeteui-static
  LINK kopeteprivate-static kabc-shared kutils-shared khtml-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
