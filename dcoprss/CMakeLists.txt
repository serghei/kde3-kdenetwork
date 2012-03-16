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

kde_import( librss )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES rssservice.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### rssservice (executable) ###################

kde_add_executable( rssservice AUTOMOC
  SOURCES
    main.cpp service.cpp query.cpp document.cpp article.cpp query.skel
    service.skel xmlrpciface.cpp cache.cpp
  LINK rss-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### rssclient (executable) ####################

kde_add_executable( rssclient
  SOURCES
    client.cpp
  LINK kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### feedbrowser (executable) ##################

kde_add_executable( feedbrowser AUTOMOC
  SOURCES
    feedbrowser.skel feedbrowser.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
