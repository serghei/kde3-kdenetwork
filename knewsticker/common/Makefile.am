INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libknewstickercommon.la

noinst_HEADERS = configiface.h configaccess.h newsiconmgr.h xmlnewsaccess.h \
  newsengine.h

METASOURCES = AUTO

libknewstickercommon_la_SOURCES = newsiconmgr.cpp xmlnewsaccess.cpp \
  configaccess.cpp newsiconmgr.skel newsengine.cpp
libknewstickercommon_la_LIBADD = $(LIB_KIO) $(LIB_KDECORE)
libknewstickercommon_la_LDFLAGS = $(all_libraries) -no-undefined
