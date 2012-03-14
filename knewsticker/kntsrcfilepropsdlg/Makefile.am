INCLUDES = -I$(top_srcdir)/knewsticker/common -I$(top_srcdir)/librss $(all_includes)

kde_module_LTLIBRARIES = libkntsrcfilepropsdlg.la

libkntsrcfilepropsdlg_la_SOURCES = kntsrcfilepropsdlg.cpp \
	kntsrcfilepropsdlgwidget.ui
libkntsrcfilepropsdlg_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkntsrcfilepropsdlg_la_LIBADD = $(LIB_KIO) ../common/libknewstickercommon.la ../../librss/librss.la
libkntsrcfilepropsdlg_la_METASOURCES = AUTO

service_DATA = kntsrcfilepropsdlg.desktop
servicedir = $(kde_servicesdir)
