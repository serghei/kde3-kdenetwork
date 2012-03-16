#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

# FIXME KDE_CXXFLAGS = $(USE_RTTI) $(USE_EXCEPTIONS)

add_subdirectory( KSTicker )
add_subdirectory( KSProgress )
add_subdirectory( KSPrefs )
add_subdirectory( KSOpenkSirc )
add_subdirectory( img )
add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/KSOpenkSirc
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( PROGRAMS
    dsirc
  DESTINATION ${BIN_INSTALL_DIR} )

install( FILES
    ksirc.pl filters.pl autodcc.pl sirc.help.gz relnotes eventsrc
  DESTINATION ${DATA_INSTALL_DIR}/ksirc )

install( FILES
    ksircrc
  DESTINATION ${CONFIG_INSTALL_DIR} )

install( FILES
    ksirc.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### ksirc (kdeinit) ###########################

kde_add_kdeinit_executable( ksirc AUTOMOC
  SOURCES
    toplevel.cpp charSelector.cpp chanButtons.cpp ksirc.cpp alistbox.cpp
    ahistlineedit.cpp iocontroller.cpp NewWindowDialog.cpp
    messageReceiver.cpp ioBroadcast.cpp ksircprocess.cpp
    servercontroller.cpp FilterRuleWidget.ui FilterRuleEditor.cpp
    ioDiscard.cpp ioDCC.cpp ioLAG.cpp ioNotify.cpp ssfeprompt.cpp
    ssfepromptdata.cpp usercontrolmenu.cpp baserules.cpp chanparser.cpp
    objFinder.cpp displayMgrSDI.cpp displayMgrMDI.cpp mditoplevel.cpp
    ksopts.cpp topic.cpp ksview.cpp logfile.cpp kstextview.cpp
    colorpicker.cpp ksparser.cpp dccManagerbase.ui dccManager.cpp
    dccToplevel.cpp dockservercontroller.cpp nickColourMaker.cpp
    dccNewbase.ui dccNew.cpp
  LINK
    ksprefs-static ksprogress-static ksopenksirc-static ksticker-static
    kio-shared
)
