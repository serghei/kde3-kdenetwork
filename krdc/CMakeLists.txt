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

add_subdirectory( vnc )
add_subdirectory( rdp )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( krdc )

install( FILES
    pointcursor.png pointcursormask.png pinup.png pindown.png
    iconify.png close.png
  DESTINATION ${DATA_INSTALL_DIR}/krdc/pics )

install( FILES
    _rfb._tcp
  DESTINATION ${DATA_INSTALL_DIR}/zeroconf )

install( FILES
    krdc.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    smb2rdc.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )

install( FILES
    vnc.protocol rdp.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### krdc (executable) #########################

kde_add_executable( krdc AUTOMOC
  SOURCES
    main.cpp krdc.cpp vidmode.cpp kfullscreenpanel.cpp hostprofiles.ui
    maindialogbase.ui keycapturewidget.ui kservicelocator.cpp
    keycapturedialog.cpp kremoteview.cpp smartptr.cpp hostpreferences.cpp
    preferencesdialog.cpp maindialogwidget.cpp maindialog.cpp
  LINK vnc-static rdp-static kdeui-shared kdnssd-shared kwalletclient-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
