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
  ${GLIB2_INCLUDE_DIRS}
)


##### mimic (static) ############################

kde_add_library( mimic STATIC_PIC
  SOURCES
    mimic.c encode.c decode.c bitstring.c vlc_common.c vlc_encode.c
    vlc_decode.c fdct_quant.c idct_dequant.c colorspace.c deblock.c
)
