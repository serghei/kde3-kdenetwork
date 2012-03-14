## Makefile.am for kcmwifi

INCLUDES= $(all_includes)

kde_module_LTLIBRARIES = kcm_wifi.la
kcm_wifi_la_SOURCES = wificonfig.cpp kcmwifi.cpp configcrypto.ui \
                      ifconfigpagebase.ui configpower.ui mainconfigbase.ui \
                      mainconfig.cpp ifconfigpage.cpp vendorconfig.cpp
kcm_wifi_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_wifi_la_LIBADD = $(LIB_KIO)

METASOURCES = AUTO

noinst_HEADERS = 

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmwifi.pot

xdg_apps_DATA = kcmwifi.desktop
