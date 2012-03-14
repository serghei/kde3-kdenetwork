# Yow, Emacs, this is a -*- makefile -*-

xdg_apps_DATA = kwifimanager.desktop

EXTRA_DIST = $(xdg_apps_DATA)
INCLUDES = $(all_includes)

SUBDIRS = kcmwifi kwireless pixmaps locations

bin_PROGRAMS = kwifimanager

kwifimanager_SOURCES = 	interface_wireless.cpp \
			interface_wireless_wirelessextensions.cpp \
			interface_dcop.skel \
			networkscanning.cpp \
			kwifimanager.cpp \
			locator.cpp \
			main.cpp \
			picture.cpp \
			speed.cpp \
			statistics.cpp \
			status.cpp \
			strength.cpp \
			asusled.cpp

kwifimanager_LDFLAGS = $(all_libraries)

if include_ARTS
ARTS_LIBS = -lartsflow -lmcop
endif

kwifimanager_LDADD = $(ARTS_LIBS) -liw $(LIB_KDEUI)

noinst_HEADERS = 	interface_wireless.h \
			interface_wireless_wirelessextensions.h \
			interface_dcop.h \
			networkscanning.h \
			kwifimanager.h \
			locator.h \
			picture.h \
			speed.h \
			statistics.h \
			status.h \
			strength.h \
			asusled.h

METASOURCES = AUTO

KDE_ICON = AUTO

appdir=$(kde_datadir)/kwifimanager
app_DATA = kwifimanagerui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kwifimanager.pot

