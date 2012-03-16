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
  ${CMAKE_CURRENT_SOURCE_DIR}/../srvloc
  ${CMAKE_CURRENT_SOURCE_DIR}/../libvncserver
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( krfb )
install( FILES krfb.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES eyes-open24.png eyes-closed24.png connection-side-image.png DESTINATION ${DATA_INSTALL_DIR}/krfb/pics )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/krfb )
install( FILES kinetd_krfb.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### krfb (executable) #########################

kde_add_executable( krfb AUTOMOC
  SOURCES
    rfbcontroller.cc xupdatescanner.cc main.cpp connectionwidget.ui
    krfbifaceimpl.cc krfbiface.skel trayicon.cpp connectiondialog.cc
  LINK
    krfbconfig-static vncserver-static srvloc-static kdeui-shared kio-shared ${XTST_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


##### krfbconfig (static) #######################

kde_add_library( krfbconfig STATIC_PIC AUTOMOC
  SOURCES
    configuration.cc configuration.skel invitedialog.cc invitation.cc
    manageinvitations.ui personalinvitewidget.ui invitewidget.ui
    personalinvitedialog.cc
)
