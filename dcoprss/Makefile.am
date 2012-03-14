bin_PROGRAMS = rssservice rssclient feedbrowser
INCLUDES = -I$(top_srcdir) $(all_includes)

rssservice_LDFLAGS = $(KDE_RPATH) $(all_libraries)
rssservice_LDADD = $(LIB_KIO) ../librss/librss.la
rssservice_SOURCES = main.cpp service.cpp query.cpp document.cpp article.cpp query.skel service.skel xmlrpciface.cpp cache.cpp


# client stuff
rssclient_LDFLAGS = $(KDE_RPATH) $(all_libraries)
rssclient_LDADD   = $(LIB_KDECORE)
rssclient_SOURCES = client.cpp

feedbrowser_LDFLAGS = $(KDE_RPATH) $(all_libraries)
feedbrowser_LDADD = $(LIB_KDEUI)
feedbrowser_SOURCES = feedbrowser.skel feedbrowser.cpp

noinst_HEADERS = service.h query.h xmlrpciface.h cache.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/dcoprss.pot


service_DATA = rssservice.desktop
servicedir = $(kde_servicesdir)

