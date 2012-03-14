## -*- makefile -*-
# Ktalkd - Main Makefile.am

if KDE_INSTALLED
kde_SUBDIRS     = kcmktalkd ktalkdlg
endif
SUBDIRS 	=	ktalkd mail.local $(kde_SUBDIRS)

EXTRA_DIST	=	ChangeLog TODO ktalkd.lsm

install-data-local:
	@echo "**************************************************************************"
	@echo
	@echo "Don't forget to update /etc/inetd.conf :"
	@echo
	@echo "For example, on a linux system, if kde stays in "$(prefix)", set it to :"
	@echo "talk	dgram	udp	wait	root	/usr/sbin/tcpd	"$(bindir)"/ktalkd"
	@echo "ntalk	dgram	udp	wait	root	/usr/sbin/tcpd	"$(bindir)"/ktalkd"
	@echo	
	@echo "Alternatively, you can use the script post-install.sh, to do the job"
	@echo
	@echo "Anyway, you'll have to restart inetd after this."
	@echo "On most linux system, do a  'killall -HUP inetd'"
	@echo
	@echo "**************************************************************************"

messages:
	$(XGETTEXT) */*.cpp -o $(podir)/kcmktalkd.pot

