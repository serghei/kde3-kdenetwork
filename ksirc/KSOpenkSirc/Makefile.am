KDE_CXXFLAGS = $(USE_RTTI) $(USE_EXCEPTIONS) -UQT_NO_ASCII_CAST

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

####### This part is very pws specific
# you can add here more. This one gets installed
noinst_LTLIBRARIES = libksopenksirc.la

# Which sources should be compiled for dialog

libksopenksirc_la_SOURCES = \
	open_ksirc.cpp\
	open_ksircData.ui\
	serverFileParser.cpp\
        enter_combo.cpp

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = open_ksirc.h open_ksircData.h serverDataType.h serverFileParser.h enter_combo.h

# just to make sure, automake makes them
METASOURCES = AUTO 

misc_DATA = servers.txt servers.ini
miscdir = $(kde_datadir)/ksirc

