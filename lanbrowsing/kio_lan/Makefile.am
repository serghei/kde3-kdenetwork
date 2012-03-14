## Makefile.am of kdebase/kioslave/man

INCLUDES= $(all_includes)

####### Files

kde_module_LTLIBRARIES = kio_lan.la

kio_lan_la_SOURCES = kio_lan.cpp
kio_lan_la_LIBADD = $(LIB_KIO)
kio_lan_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries) $(KDE_RPATH)

noinst_HEADERS = kio_lan.h

kdelnk_DATA = lan.protocol rlan.protocol
kdelnkdir = $(kde_servicesdir)

remote_DATA = lan.desktop
remotedir = $(kde_datadir)/konqueror/dirtree/remote

remoteio_DATA = lan.desktop
remoteiodir = $(kde_datadir)/remoteview

konq_sidebartree_DATA = lisa.desktop
konq_sidebartreedir = $(kde_datadir)/konqsidebartng/virtual_folders/services

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_lan.pot

