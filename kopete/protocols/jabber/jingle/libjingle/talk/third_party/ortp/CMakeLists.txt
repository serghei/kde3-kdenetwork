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
  -D_ORTP_SOURCE
  -DG_LOG_DOMAIN="oRTP"
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${GLIB2_INCLUDE_DIRS}
)


##### ortp (static) #############################

kde_add_library( ortp STATIC_PIC
  SOURCES
    port_fct.c rtpmod.c rtpparse.c rtpsession.c jitterctl.c
    rtpsignaltable.c rtptimer.c posixtimer.c ortp.c scheduler.c
    avprofile.c sessionset.c telephonyevents.c payloadtype.c rtcp.c
    utils.c rtcpparse.c str_utils.c
)
