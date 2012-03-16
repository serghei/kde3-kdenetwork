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
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### ksprefs (static) ##########################

kde_add_library( ksprefs STATIC_PIC AUTOMOC
  SOURCES
    ksprefs.cpp page_colorsbase.ui page_generalbase.ui
    page_rmbmenubase.ui page_servchanbase.ui page_startupbase.ui
    page_colors.cpp page_general.cpp page_rmbmenu.cpp page_servchan.cpp
    page_startup.cpp page_font.cpp page_autoconnectbase.ui
    page_autoconnect.cpp page_irccolorsbase.ui page_irccolors.cpp
    page_looknfeelbase.ui page_looknfeel.cpp page_shortcutsbase.ui
    page_shortcuts.cpp
)
