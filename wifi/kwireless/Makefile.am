INCLUDES= $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkwireless.la

libkwireless_la_SOURCES = kwireless.cpp \
	kwirelesswidget.cpp \
	linuxwirelesswidget.cpp  \
	propertytablebase.ui propertytable.cpp 
libkwireless_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkwireless_la_LIBADD =  -liw $(LIB_KDEUI)

kwireless_DATA = kwireless.desktop
kwirelessdir = $(kde_datadir)/kicker/applets

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kwireless.pot
