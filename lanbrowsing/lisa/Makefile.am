##
AM_CXXFALGS = -fno-rtti -fno-exceptions

#CXXFLAGS = -fno-rtti -fno-exceptions

bin_PROGRAMS = lisa reslisa
#
# Programs to compile, Manpages to install and Versions
#
#INCLUDES = $(all_includes)

lisa_SOURCES = \
addressvalidator.cpp \
netmanager.cpp \
netscanner.cpp \
ipaddress.cpp \
main.cpp \
mystring.cpp \
client.cpp \
configfile.cpp

reslisa_SOURCES = \
addressvalidator.cpp \
netmanager.cpp \
netscanner.cpp \
ipaddress.cpp \
strictmain.cpp \
mystring.cpp \
client.cpp \
configfile.cpp

lisa_LDADD = $(LIBSOCKET)
lisa_LDFLAGS = $(KDE_EXTRA_RPATH)

reslisa_LDADD = $(LIBSOCKET)
reslisa_LDFLAGS = $(KDE_EXTRA_RPATH)

misc_DATA = README
miscdir = $(kde_datadir)/lisa
EXTRA_DIST=$(misc_DATA)   

