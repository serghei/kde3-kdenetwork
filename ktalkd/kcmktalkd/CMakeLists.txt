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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( ktalkd )
install( FILES kcmktalkd.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_ktalkd (module) #######################

kde_add_kpart( kcm_ktalkd AUTOMOC
  SOURCES
    main.cpp soundpage.cpp answmachpage.cpp forwmachpage.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
