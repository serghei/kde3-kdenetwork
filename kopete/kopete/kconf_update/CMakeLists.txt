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

add_definitions( -DKDE_NO_COMPAT )

include_directories(
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kopete-pluginloader.upd kopete-account-kconf_update.upd
    kopete-pluginloader2.upd kopete-jabberproxytype-kconf_update.upd
    kopete-jabberpriorityaddition-kconf_update.upd
    kopete-nameTracking.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( PROGRAMS
    kopete-pluginloader.pl kopete-jabberpriorityaddition-kconf_update.sh
    kopete-pluginloader2.sh kopete-jabberproxytype-kconf_update.sh
    kopete-account-0.10.pl kopete-account-kconf_update.sh
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### kopete_account_kconf_update (executable) ##

kde_add_executable( kopete_account_kconf_update
  SOURCES kopete-account-kconf_update.cpp
  LINK ${QT_LIBRARY}
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)


##### kopete_pluginloader2_kconf_update (executable)

kde_add_executable( kopete_pluginloader2_kconf_update
  SOURCES kopete-pluginloader2.cpp
  LINK ${QT_LIBRARY}
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)


##### kopete_nameTracking_kconf_update (executable)

kde_add_executable( kopete_nameTracking_kconf_update
  SOURCES kopete-nameTracking.cpp
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)
