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
  ${CMAKE_CURRENT_BINARY_DIR}/../kcm_sambaconf
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    fileshare_propsdlgplugin.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### propsdlgplugin_common (static) ############

kde_add_library( propsdlgplugin_common STATIC_PIC AUTOMOC
  SOURCES propertiespage.cpp propertiespagegui.ui
)


##### fileshare_propsdlgplugin (module) #########

kde_add_kpart( fileshare_propsdlgplugin AUTOMOC
  SOURCES propsdlgshareplugin.cpp
  LINK
    propsdlgplugin_common-static filesharenfs-static
    filesharesamba-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
