INCLUDES = $(all_includes)

lib_LTLIBRARIES = librss.la
librss_la_METASOURCES = AUTO
librss_la_SOURCES = article.cpp document.cpp image.cpp textinput.cpp \
  tools_p.cpp loader.cpp
librss_la_LDFLAGS = -no-undefined -version-info 1:0 $(all_libraries)
librss_la_LIBADD = $(LIB_KIO)

librssincludedir = $(includedir)/rss
librssinclude_HEADERS = article.h document.h global.h image.h textinput.h \
  loader.h librss.h

check_PROGRAMS = testlibrss
testlibrss_SOURCES = testlibrss.cpp
testlibrss_LDFLAGS = $(all_libraries)
testlibrss_LDADD = librss.la
