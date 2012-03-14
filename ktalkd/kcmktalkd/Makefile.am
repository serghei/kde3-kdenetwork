xdg_apps_DATA = kcmktalkd.desktop
KDE_ICON  = ktalkd

INCLUDES = $(all_includes)
METASOURCES=AUTO

kde_module_LTLIBRARIES = kcm_ktalkd.la

kcm_ktalkd_la_SOURCES = main.cpp soundpage.cpp answmachpage.cpp \
	forwmachpage.cpp
kcm_ktalkd_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kcm_ktalkd_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

#for extra warnings during compilation :
# AMDEFS		=	-ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE
