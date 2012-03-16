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
  -DG_LOG_DOMAIN="MediaStreamer"
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../ortp
  ${CMAKE_BINARY_DIR}
  ${GLIB2_INCLUDE_DIRS}
  ${SPEEX_INCLUDE_DIRS}
)


##### mediastreamer (static) ####################

kde_add_library( mediastreamer STATIC_PIC
  SOURCES
    msfilter.c mscodec.c mssoundread.c mssoundwrite.c msbuffer.c
    msqueue.c msfifo.c ms.c mssync.c msnosync.c msread.c mswrite.c
    mscopy.c msosswrite.c msossread.c msringplayer.c msrtprecv.c
    msrtpsend.c msAlawenc.c msAlawdec.c msMUlawenc.c msMUlawdec.c
    mstimer.c msqdispatcher.c msfdispatcher.c sndcard.c osscard.c
    hpuxsndcard.c alsacard.c jackcard.c audiostream.c msspeexenc.c
    msspeexdec.c msilbcdec.c msilbcenc.c
)
