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
  ${CMAKE_BINARY_DIR}
)


##### mail.local (executable) ###################

kde_add_executable( mail.local
  SOURCES mail.local.c
  DESTINATION ${BIN_INSTALL_DIR}
)
