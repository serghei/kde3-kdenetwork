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

install( FILES kinetd_krfb_httpd.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( PROGRAMS krfb_httpd DESTINATION ${BIN_INSTALL_DIR} )
