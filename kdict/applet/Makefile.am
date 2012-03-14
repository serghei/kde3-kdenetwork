INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kdict_panelapplet.la

kdict_panelapplet_la_SOURCES = kdictapplet.cpp

METASOURCES = AUTO
noinst_HEADERS = kdictapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = kdictapplet.desktop

EXTRA_DIST = $(lnk_DATA)

kdict_panelapplet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kdict_panelapplet_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdictapplet.pot
