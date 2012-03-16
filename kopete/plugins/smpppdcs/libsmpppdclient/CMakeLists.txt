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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### smpppdclient (static) #####################

kde_add_library( smpppdclient STATIC_PIC
  SOURCES
    smpppdclient.cpp smpppdstate.cpp smpppdready.cpp smpppdunsettled.cpp
)
