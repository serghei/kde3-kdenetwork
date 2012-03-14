SUBDIRS = common kntsrcfilepropsdlg knewstickerstub
INCLUDES = -I$(top_srcdir)/knewsticker/common $(all_includes)

kde_module_LTLIBRARIES = knewsticker_panelapplet.la

noinst_HEADERS = newsscroller.h knewsticker.h

METASOURCES = AUTO

knewsticker_panelapplet_la_SOURCES = newsscroller.cpp \
	knewsticker.cpp knewsticker.skel \
	knewstickerconfig.cpp knewstickerconfigwidget.ui \
	newssourcedlgimpl.cpp newssourcedlg.ui
knewsticker_panelapplet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
knewsticker_panelapplet_la_LIBADD = common/libknewstickercommon.la \
	$(LIB_KIO) $(LIB_KDEUI)

appdatadir = $(kde_datadir)/knewsticker
appdata_DATA = eventsrc

updatedir = $(kde_datadir)/kconf_update
update_DATA = knewsticker.upd
update_SCRIPTS = knt-0.1-0.2.pl

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = knewsticker.desktop

xdg_apps_DATA = knewsticker-standalone.desktop

KDE_ICON = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.ui */*.ui >> rc.cpp
	$(XGETTEXT) *.cpp *.h */*.cpp */*.h -o $(podir)/knewsticker.pot
