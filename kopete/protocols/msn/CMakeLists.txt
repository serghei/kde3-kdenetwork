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
add_subdirectory( config )

if( WITH_WEBCAM )
  add_subdirectory( webcam )
  add_definitions( -DMSN_WEBCAM )
  set( WEBCAM_LIBRARIES mimicwrapper-static kopete_videodevice-shared ${GLIB2_LIBRARIES} )
endif( )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/webcam
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopete_msn.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES msnchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_msn )


##### kopete_msn (module) #######################

kde_add_kpart( kopete_msn AUTOMOC
  SOURCES
    webcam.cpp
  LINK kopete_msn_shared-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kopete_msn_shared (shared) ################

kde_add_library( kopete_msn_shared SHARED AUTOMOC
  SOURCES
    msnprotocol.cpp msnaccount.cpp msnaddcontactpage.cpp msncontact.cpp
    msnsocket.cpp msnchatsession.cpp msndebugrawcmddlg.cpp
    msnnotifysocket.cpp msnswitchboardsocket.cpp
    msnfiletransfersocket.cpp msninvitation.cpp sha1.cpp
    msnsecureloginhandler.cpp msnchallengehandler.cpp
    dispatcher.cpp p2p.cpp messageformatter.cpp incomingtransfer.cpp
    outgoingtransfer.cpp webcam.cpp
  VERSION 0.0.0
  LINK
    kopetemsnui-static ${WEBCAM_LIBRARIES} kopete-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
