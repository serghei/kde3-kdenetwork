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

add_subdirectory( emoticons )

install( FILES
    server.xpm channel.xpm channels.xpm ksirc_a.xpm ksirc_b.xpm
    arrow.png greenpin.png madsmiley.png smiley.png sadsmiley.png
    blueball.png star.png bluepin.png info.png info1.png mini-run.png
    ksirc.png sdi.png mdi.png dcc.png kick.png mode.png quit.png X.png
    dccget.png minus.png notice.png servinfo.png action.png dccsend.png
    ominus.png topic.png bminus.png elipsis.png oplus.png bplus.png
    error.png part.png ctcpping.png join.png plus.png
  DESTINATION ${DATA_INSTALL_DIR}/ksirc/pics )
