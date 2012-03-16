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

add_subdirectory( konqui )

install( FILES
    Side_blue_moon.css Side_blue_without_transparency.css
    Side_green_without_trans.css Side_blue.css
    Side_blue_moon_without_transparency.css Side_green.css
    Side_green_without_transparency.css
  DESTINATION ${DATA_INSTALL_DIR}/kopete/styles/Konqi/Contents/Resources/Variants )
