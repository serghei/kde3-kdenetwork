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
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/private
  ${CMAKE_SOURCE_DIR}/kopete/libkopete/ui
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${CMAKE_SOURCE_DIR}/kopete/kopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kopeterichtexteditpartfull.rc DESTINATION ${DATA_INSTALL_DIR}/kopeterichtexteditpart )
install( FILES kopetechatwindow.rc kopeteemailwindow.rc DESTINATION ${DATA_INSTALL_DIR}/kopete )
install( FILES chatwindow.desktop emailwindow.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### libkrichtexteditpart (module) #############

kde_add_kpart( libkrichtexteditpart AUTOMOC
  SOURCES krichtexteditpart.cpp
  LINK kopete-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kopete_chatwindow (module) ################

kde_add_kpart( kopete_chatwindow AUTOMOC
  SOURCES
    chatview.cpp kopetechatwindow.cpp chatmemberslistwidget.cpp
  LINK kopeteui-static kopetechatwindow-static kopete-shared khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kopete_emailwindow (module) ###############

kde_add_kpart( kopete_emailwindow AUTOMOC
  SOURCES
    kopeteemailwindow.cpp
  LINK kopeteui-static kopetechatwindow-static kopete-shared khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kopetechatwindow (static) #################

kde_add_library( kopetechatwindow STATIC_PIC AUTOMOC
  SOURCES
    chatmessagepart.cpp emoticonselector.cpp kopeteemoticonaction.cpp
    chattexteditpart.cpp krichtexteditpart.cpp kopetechatwindowstyle.cpp
    kopetechatwindowstylemanager.cpp
)
