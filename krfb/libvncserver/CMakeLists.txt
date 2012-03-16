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

add_definitions( -DHAVE_PTHREADS -DALLOW24BPP )


##### vncserver (static) ########################

kde_add_library( vncserver STATIC_PIC
  SOURCES
    main.c rfbserver.c sraRegion.c auth.c sockets.c stats.c corre.c
    hextile.c rre.c translate.c cutpaste.c zlib.c tight.c httpd.c
    cursor.c font.c draw.c selbox.c d3des.c vncauth.c cargs.c
)
