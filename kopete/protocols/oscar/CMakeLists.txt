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

add_subdirectory( liboscar )
add_subdirectory( aim )
add_subdirectory( icq )
add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/liboscar
  ${CMAKE_SOURCE_DIR}/kopete/libkopete
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kopete_oscar (shared) #####################

kde_add_library( kopete_oscar SHARED AUTOMOC
  SOURCES
    oscaraccount.cpp oscarcontact.cpp oscarmyselfcontact.cpp
    oscarencodingselectionbase.ui oscarencodingselectiondialog.cpp
    oscarlistcontactsbase.ui oscarlistnonservercontacts.cpp
    oscarvisibilitybase.ui oscarvisibilitydialog.cpp
    oscarversionupdater.cpp
  VERSION 2.0.0
  LINK oscar-static kopete-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
