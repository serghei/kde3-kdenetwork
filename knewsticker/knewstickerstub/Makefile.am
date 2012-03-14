INCLUDES = -I$(top_srcdir)/knewsticker/common $(all_includes)

bin_PROGRAMS = knewstickerstub

servicedir = $(kde_appsdir)/.hidden
service_DATA = knewstickerstub.desktop

METASOURCES = AUTO

knewstickerstub_SOURCES = knewstickerstub.cpp
knewstickerstub_LDADD = $(LIB_KIO) ../common/libknewstickercommon.la
knewstickerstub_LDFLAGS = $(all_libraries) $(KDE_RPATH)

