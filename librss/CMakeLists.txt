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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### headers ###################################

install( FILES
    article.h document.h global.h image.h textinput.h loader.h librss.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/rss )


##### rss (shared) ##############################

kde_add_library( rss SHARED AUTOMOC
  SOURCES
    article.cpp document.cpp image.cpp textinput.cpp tools_p.cpp
    loader.cpp
  VERSION 1.0.0
  LINK kio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
