INCLUDES = $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = kpf_panelapplet.la kpfpropertiesdialog.la

kpf_panelapplet_la_SOURCES = \
  Utils.cpp \
  DirectoryLister.cpp \
  ByteRange.cpp \
  DirSelectWidget.cpp \
  PortValidator.cpp \
  Request.cpp \
  Response.cpp \
  Resource.cpp \
  RootValidator.cpp \
  Server.cpp \
  ServerPrivate.cpp \
  ServerSocket.cpp \
  WebServer.cpp \
  WebServer.skel \
  WebServer.stub \
  WebServerSocket.cpp \
  WebServerManager.cpp \
  WebServerManager.skel \
  SingleServerConfigDialog.cpp \
  System.cpp \
  ConfigDialogPage.cpp \
  ErrorMessageConfigDialog.cpp \
  ActiveMonitor.cpp \
  ActiveMonitorItem.cpp \
  ActiveMonitorWindow.cpp \
  BandwidthGraph.cpp \
  ServerWizard.cpp \
  AppletItem.cpp \
  Applet.cpp \
  Defaults.cpp \
  Help.cpp

kpf_panelapplet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

kpf_panelapplet_la_LIBADD = $(LIB_KIO) -lkdnssd

kpfpropertiesdialog_la_SOURCES = \
  PropertiesDialogPlugin.cpp \
  StartingKPFDialog.cpp \
  WebServer.stub \
  WebServerManager.stub \
  Defaults.cpp \
  Help.cpp

kpfpropertiesdialog_la_LIBADD = $(LIB_KIO) -lkdnssd

kpfpropertiesdialog_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

noinst_HEADERS = \
  Utils.h \
  Defaults.h \
  DirectoryLister.h \
  DirSelectWidget.h \
  ByteRange.h \
  PortValidator.h \
  Request.h \
  Response.h \
  Resource.h \
  RootValidator.h \
  Server.h \
  ServerPrivate.h \
  ServerSocket.h \
  WebServer.h \
  WebServerSocket.h \
  WebServerManager.h \
  SingleServerConfigDialog.h \
  ConfigDialogPage.h \
  PropertiesDialogPlugin.h \
  StartingKPFDialog.h \
  ErrorMessageConfigDialog.h \
  ActiveMonitor.h \
  ActiveMonitorItem.h \
  ActiveMonitorWindow.h \
  BandwidthGraph.h \
  ServerWizard.h \
  AppletItem.h \
  Applet.h \
  Help.h

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kpf.pot

