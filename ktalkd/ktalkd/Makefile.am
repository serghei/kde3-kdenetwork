## -*- makefile -*-
# Ktalkd - Makefile.am

SUBDIRS 	=	machines

bin_PROGRAMS	=	ktalkd
EXTRA_HEADERS	=	readcfg++.h
INCLUDES	=	$(all_includes)
ktalkd_DEPS	=	machines/libmach.a

#for extra warnings during compilation :
#KDE_CXXFLAGS =	-ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE

###########################################################

# Config file location
TALKD_CONF	=	$(kde_confdir)/ktalkdrc
TALKD_CONF_NAME	=	ktalkdrc
AM_CPPFLAGS	=      -DHAVE_KDE
ktalkd_SOURCES	=	print.c repairs.c \
	announce.cpp process.cpp readcfg++.cpp table.cpp talkd.cpp \
	find_user.cpp threads.cpp options.cpp unixsock.cpp
ktalkd_LDFLAGS	=       $(all_libraries) $(KDE_RPATH)
ktalkd_LDADD	=       machines/libmach.a $(LIBBSD) $(LIB_KDECORE) $(LIBSOCKET)

###########################################################


EXTRA_DIST	=	.talkdrc talkd.conf ktalkdrc

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
	$(mkinstalldirs) $(DESTDIR)$(kde_sounddir)
	$(INSTALL_DATA) $(srcdir)/ktalkd.wav $(DESTDIR)$(kde_sounddir)
	@echo "**************************************************************************"
	@echo
	@if [ -f $(DESTDIR)$(TALKD_CONF) ]; then \
	  echo "Please check $(TALKD_CONF) to be up-to-date."; \
	else \
	  $(INSTALL_DATA) $(srcdir)/$(TALKD_CONF_NAME) $(DESTDIR)$(TALKD_CONF); \
	fi
