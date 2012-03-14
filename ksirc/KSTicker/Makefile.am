INCLUDES= $(all_includes)

noinst_LTLIBRARIES = libksticker.la

libksticker_la_SOURCES = ksticker.cpp speeddialog.cpp speeddialogData.cpp \
	kspainter.cpp

noinst_HEADERS = ksticker.h speeddialog.h speeddialogData.h \
	kspainter.h

METASOURCES = AUTO


check_PROGRAMS = ksttest

ksttest_SOURCES = ksttest.cpp
ksttest_LDADD = libksticker.la ../ksopts.lo ../nickColourMaker.lo $(LIB_KDEUI) 
ksttest_LDFLAGS = $(all_libraries) $(KDE_RPATH)


