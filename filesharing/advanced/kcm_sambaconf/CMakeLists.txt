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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES kcmsambaconf.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### filesharesamba (static) ###################

kde_add_library( filesharesamba STATIC_PIC AUTOMOC
  SOURCES
    sambafile.cpp share.ui sharedlgimpl.cpp sambashare.cpp
    socketoptionsdlg.ui common.cpp userselectdlg.ui
    groupselectdlg.ui usertab.ui usertabimpl.cpp filemodedlg.ui
    filemodedlgimpl.cpp smbpasswdfile.cpp passwd.cpp hiddenfileview.cpp
    dictmanager.cpp qmultichecklistitem.cpp smbconfconfigwidget.cpp
    linuxpermissionchecker.cpp expertuserdlg.ui
)


##### kcm_kcmsambaconf (module) #################

kde_add_kpart( kcm_kcmsambaconf AUTOMOC
  SOURCES
    kcminterface.ui kcmprinterdlg.ui printerdlgimpl.cpp kcmsambaconf.cpp
    joindomaindlg.ui
  LINK filesharesamba-static kdeprint-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
