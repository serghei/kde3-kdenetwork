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

add_definitions(
  -DPOSIX
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${QT_INCLUDE_DIR}
)


##### cricketxmllite (static) ###################

kde_add_library( cricketxmllite STATIC_PIC
  SOURCES
    qname.cc xmlbuilder.cc xmlconstants.cc xmlelement.cc xmlnsstack.cc
    xmlparser.cc xmlprinter.cc
)
