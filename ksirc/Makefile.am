
KDE_CXXFLAGS = $(USE_RTTI) $(USE_EXCEPTIONS)# -UQT_NO_ASCII_CAST

# set the include path for X, qt and KDE
INCLUDES= -IKSOpenkSirc -IKSPrefs $(all_includes)

# claim, which subdirectories you want to install
SUBDIRS = KSTicker KSProgress KSPrefs KSOpenkSirc img icons

METASOURCES = AUTO

check_PROGRAMS = testview dccMgrTest

testview_SOURCES = kstextviewtest.cpp
testview_LDADD = kstextview.lo dccNew.lo dccNewbase.lo alistbox.lo objFinder.lo ksopts.lo nickColourMaker.lo $(LIB_KUTILS) $(LIB_KDEUI)
testview_LDFLAGS = $(all_libraries) $(KDE_RPATH)

dccMgrTest_SOURCES = dccMgrTest.cpp
dccMgrTest_LDADD = dccManager.lo dccManagerbase.lo dccNew.lo dccNewbase.lo alistbox.lo objFinder.lo ksopts.lo nickColourMaker.lo $(LIB_KUTILS) $(LIB_KDEUI)
dccMgrTest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = ksirc.la

noinst_PROGRAMS = getdate

# Which sources should be compiled for ksirc.

ksirc_la_SOURCES = \
	toplevel.cpp \
	charSelector.cpp \
	chanButtons.cpp \
	ksirc.cpp \
	alistbox.cpp \
	ahistlineedit.cpp \
	iocontroller.cpp \
	NewWindowDialog.cpp \
	messageReceiver.cpp \
	ioBroadcast.cpp \
	ksircprocess.cpp \
	servercontroller.cpp \
	FilterRuleWidget.ui FilterRuleEditor.cpp \
	ioDiscard.cpp \
	ioDCC.cpp \
	ioLAG.cpp \
	ioNotify.cpp \
	ssfeprompt.cpp ssfepromptdata.cpp \
	usercontrolmenu.cpp \
	baserules.cpp \
	chanparser.cpp \
	objFinder.cpp \
	displayMgrSDI.cpp \
	displayMgrMDI.cpp \
	mditoplevel.cpp \
	ksopts.cpp \
	topic.cpp \
	ksview.cpp \
	logfile.cpp \
        kstextview.cpp \
        colorpicker.cpp \
        ksparser.cpp \
        dccManagerbase.ui \
        dccManager.cpp \
        dccToplevel.cpp \
        dockservercontroller.cpp \
        nickColourMaker.cpp \
        dccNewbase.ui \
        dccNew.cpp

# the library search path
ksirc_la_LDFLAGS = $(all_libraries) -module -avoid-version
ksirc_la_COMPILE_FIRST = cdate.h

all_ksirc_libraries = \
	./KSPrefs/libksprefs.la \
	./KSProgress/libksprogress.la  \
	./KSOpenkSirc/libksopenksirc.la \
        ./KSTicker/libksticker.la \
	$(LIB_KIO) $(LIBDL) 

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ksirc_la_LIBADD   = $(all_ksirc_libraries)

kdoc:
	kdoc -ddoc ksirc *.h

getdate_SOURCES = getdate.c

cdate.h: getdate
	./getdate > cdate.h

# this option you can leave out. Just, if you use "make dist", you need it
#
# DSIRC is on the bottom so make dist includes it!!!!!!!!!
#
noinst_HEADERS = \
	FilterRuleEditor.h \
	control_message.h \
	FilterRuleWidget.h \
	messageReceiver.h \
	ioBroadcast.h \
	ioDCC.h \
	usercontrolmenu.h \
	ioDiscard.h \
	ahistlineedit.h \
	iocontroller.h \
	servercontroller.h \
	alistbox.h \
	chanparser.h \
	toplevel.h \
	ksircprocess.h \
	cdate.h \
	ioLAG.h \
	ioNotify.h \
	ssfeprompt.h \
	ssfepromptdata.h \
	objFinder.h \
	baserules.h \
	displayMgr.h \
	displayMgrSDI.h \
	NewWindowDialog.h \
    ksopts.h \
	ksview.h \
	boundscheckingarray.h \
	ksparser.h \
	dccManager.h \
        dccToplevel.h \
        dockservercontroller.h \
        dccNew.h

messages: rc.cpp
	$(EXTRACTRC) KSOpenkSirc/*.ui >> rc.cpp
	$(EXTRACTRC) KSPrefs/*.ui >> rc.cpp
	$(XGETTEXT) *.cpp */*.cpp -o $(podir)/ksirc.pot

bin_SCRIPTS = dsirc

xdg_apps_DATA = ksirc.desktop

misc_DATA = ksirc.pl filters.pl autodcc.pl sirc.help.gz relnotes eventsrc
miscdir = $(kde_datadir)/ksirc

conf_DATA = ksircrc
confdir = $(kde_confdir)
