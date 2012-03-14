## Makefile.am for kdict

KDE_CXXFLAGS = $(USE_THREADS)

SUBDIRS = applet pics

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kdict.la

# set the include path for X, qt and KDE
AM_CPPFLAGS         = $(all_includes)

kdict_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -module $(KDE_PLUGIN)
kdict_la_LIBADD  = $(LIB_KFILE) $(LIB_KHTML) $(LIBPTHREAD) $(LIBRESOLV)
kdict_la_SOURCES = dcopinterface.skel main.cpp actions.cpp dict.cpp options.cpp \
                   queryview.cpp toplevel.cpp sets.cpp matchview.cpp application.cpp

# these are the headers for your project
noinst_HEADERS = actions.h dict.h options.h queryview.h toplevel.h sets.h matchview.h application.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdict.pot

KDE_ICON = AUTO

# this is where the kdelnk file will go
xdg_apps_DATA = kdict.desktop

rcdir = $(kde_datadir)/kdict
rc_DATA = kdictui.rc
