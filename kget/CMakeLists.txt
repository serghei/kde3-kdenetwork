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

add_subdirectory( pics )
add_subdirectory( icons )
add_subdirectory( sounds )
add_subdirectory( kget_plug_in )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES kget.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES x-kgetlist.desktop DESTINATION ${MIME_INSTALL_DIR}/application )
install( FILES kget_download.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES kgetui.rc eventsrc DESTINATION ${DATA_INSTALL_DIR}/kget )


##### kget (executable) #########################

kde_add_executable( kget AUTOMOC
  SOURCES
    getfilejob.cpp slaveevent.cpp slave.cpp transferlist.cpp
    transfer.cpp settings.cpp logwindow.cpp kmainwidget.cpp kfileio.cpp
    droptarget.cpp docking.cpp dlgconnectionbase.ui dlgautomationbase.ui
    dlglimitsbase.ui dlgadvancedbase.ui dlgdirectoriesbase.ui
    dlgsystembase.ui dlgSystem.cpp dlgPreferences.cpp dlgLimits.cpp
    dlgIndividual.cpp dlgDirectories.cpp dlgConnection.cpp
    dlgAutomation.cpp dlgAdvanced.cpp main.cpp dockindividual.cpp
    kget_iface.skel safedelete.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
