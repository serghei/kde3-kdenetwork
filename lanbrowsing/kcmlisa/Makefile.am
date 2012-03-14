kde_module_LTLIBRARIES = kcm_lanbrowser.la

kcm_lanbrowser_la_SOURCES = main.cpp setupwizard.cpp kcmlisa.cpp kcmreslisa.cpp kcmkiolan.cpp portsettingsbar.cpp findnic.cpp

kcm_lanbrowser_la_LDFLAGS  = $(all_libraries) $(KDE_PLUGIN) -module
kcm_lanbrowser_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = 

kcm_lanbrowser_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_lanbrowser_la_SOURCES) -o $(podir)/kcmlanbrowser.pot

apps_DATA = kcmkiolan.desktop  kcmlisa.desktop  kcmreslisa.desktop
appsdir = $(kde_appsdir)/.hidden
