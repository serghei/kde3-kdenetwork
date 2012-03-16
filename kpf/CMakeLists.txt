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

add_subdirectory( src )


##### other data ################################

kde_install_icons( kpf )

install( FILES
    kpfapplet.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )

install( FILES
    kpfpropertiesdialogplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )
