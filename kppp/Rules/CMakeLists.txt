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

kde_auto_add_subdirectories( )

install( FILES TEMPLATE DESTINATION ${DATA_INSTALL_DIR}/kppp/Rules )
install( PROGRAMS checkrules DESTINATION ${DATA_INSTALL_DIR} )
