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


##### other data ################################

install( FILES
    zeroconf.protocol invitation.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    _http._tcp _ftp._tcp _ldap._tcp _webdav._tcp _nfs._tcp _ssh._tcp
  DESTINATION ${DATA_INSTALL_DIR}/zeroconf )

install( FILES
    zeroconf.desktop
  DESTINATION ${DATA_INSTALL_DIR}/remoteview )


##### kio_zeroconf (module) #####################

kde_add_kpart( kio_zeroconf AUTOMOC
  SOURCES dnssd.cpp
  LINK kdnssd-shared kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
