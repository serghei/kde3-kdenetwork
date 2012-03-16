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

add_subdirectory( Incoming )
add_subdirectory( Outgoing )

install( FILES
    main.css Footer.html Header.html Status.html
  DESTINATION ${DATA_INSTALL_DIR}/kopete/styles/Retropete/Contents/Resources )
