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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kpf_panelapplet (module) ##################

kde_add_kpart( kpf_panelapplet AUTOMOC
  SOURCES
    Utils.cpp DirectoryLister.cpp ByteRange.cpp DirSelectWidget.cpp
    PortValidator.cpp Request.cpp Response.cpp Resource.cpp
    RootValidator.cpp Server.cpp ServerPrivate.cpp
    ServerSocket.cpp WebServer.cpp WebServer.skel WebServer.stub
    WebServerSocket.cpp WebServerManager.cpp WebServerManager.skel
    SingleServerConfigDialog.cpp System.cpp ConfigDialogPage.cpp
    ErrorMessageConfigDialog.cpp ActiveMonitor.cpp ActiveMonitorItem.cpp
    ActiveMonitorWindow.cpp BandwidthGraph.cpp ServerWizard.cpp
    AppletItem.cpp Applet.cpp Defaults.cpp Help.cpp
  LINK kdnssd-shared kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kpfpropertiesdialog (module) ##############

kde_add_kpart( kpfpropertiesdialog AUTOMOC
  SOURCES
    PropertiesDialogPlugin.cpp StartingKPFDialog.cpp WebServer.stub
    WebServerManager.stub Defaults.cpp Help.cpp
  LINK kdnssd-shared kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
