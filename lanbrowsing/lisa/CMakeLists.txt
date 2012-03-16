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

add_definitions( -fno-rtti -fno-exceptions )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES README DESTINATION ${DATA_INSTALL_DIR}/lisa )


##### lisa (executable) #########################

kde_add_executable( lisa
  SOURCES main.cpp
  LINK lisa-static
  DESTINATION ${BIN_INSTALL_DIR}
)


##### reslisa (executable) ######################

kde_add_executable( reslisa
  SOURCES strictmain.cpp
  LINK lisa-static
  DESTINATION ${BIN_INSTALL_DIR}
)


##### lisa (static) #############################

kde_add_library( lisa STATIC
  SOURCES
    addressvalidator.cpp netmanager.cpp netscanner.cpp ipaddress.cpp
    mystring.cpp client.cpp configfile.cpp
)
