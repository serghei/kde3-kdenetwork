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

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( kdenetwork )

set( PACKAGE kdenetwork )
set( VERSION "3.6" )


##### features ##################################

option( WITH_ALL_FEATURES "Enable all possible features" OFF )
option( WITH_JINGLE "Enable jingle support (kopete)" ${WITH_ALL_FEATURES} )
option( WITH_SPEEX "Enable speex support (for jingle)" ${WITH_ALL_FEATURES} )
option( WITH_WEBCAM "Enable webcam support (kopete/msn)" ${WITH_ALL_FEATURES} )
option( WITH_GSM "Enable GSM SMS support (kopete/sms)" ${WITH_ALL_FEATURES} )
option( WITH_ARTS "Enable aRts support" ${WITH_ALL_FEATURES} )


##### kopete protocols ##########################

option( BUILD_KOPETE_PROTOCOL_ALL "Build all kopete protocols" OFF )
option( BUILD_KOPETE_PROTOCOL_TESTBED "Build kopete protocol testbed" ${BUILD_KOPETE_PROTOCOL_ALL} )
option( BUILD_KOPETE_PROTOCOL_GROUPWISE "Build kopete protocol groupwise" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_MSN "Build kopete protocol msn" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_IRC "Build kopete protocol irc" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_OSCAR "Build kopete protocol oscar" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_YAHOO "Build kopete protocol yahoo" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_WINPOPUP "Build kopete protocol winpopup" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_SMS "Build kopete protocol sms" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_JABBER "Build kopete protocol jabber" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_GADU "Build kopete protocol gadu" ${BUILD_KOPETE_PROTOCOL_ALL}  )
option( BUILD_KOPETE_PROTOCOL_MEANWHILE "Build kopete protocol meanwhile" ${BUILD_KOPETE_PROTOCOL_ALL}  )


##### kopete plugins ############################

option( BUILD_KOPETE_PLUGIN_ALL "Build all kopete plugins" OFF )
option( BUILD_KOPETE_PLUGIN_LATEX "Build latex kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_AUTOREPLACE "Build autoreplace kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_HISTORY "Build history kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_CONTACTNOTES "Build contactnotes kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_CRYPTOGRAPHY "Build cryptography kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_CONNECTIONSTATUS "Build connectionstatus kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_TRANSLATOR "Build translator kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_NOWLISTENING "Build nowlistening kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_WEBPRESENCE "Build webpresence kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_TEXTEFFECT "Build texteffect kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_HIGHLIGHT "Build highlight kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_ALIAS "Build alias kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_MOTIONAUTOAWAY "Build motionautoaway kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_NETMEETING "Build netmeeting kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_ADDBOOKMARKS "Build addbookmarks kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_STATISTICS "Build statistics kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )
option( BUILD_KOPETE_PLUGIN_SMPPPDCS "Build smpppdcs kopete plugin" ${BUILD_KOPETE_PLUGIN_ALL} )


##### user requested modules ####################

option( BUILD_ALL "Build all" OFF )
option( BUILD_DCOPRSS "Build dcoprss" ${BUILD_ALL} )
option( BUILD_DOC "Build doc" ${BUILD_ALL} )
option( BUILD_FILESHARING "Build filesharing" ${BUILD_ALL} )
option( BUILD_KDICT "Build kdict" ${BUILD_ALL} )
option( BUILD_KDNSSD "Build kdnssd" ${BUILD_ALL} )
option( BUILD_KFILE_PLUGINS "Build kfile-plugins" ${BUILD_ALL} )
option( BUILD_KGET "Build kget" ${BUILD_ALL} )
option( BUILD_KNEWSTICKER "Build knewsticker" ${BUILD_ALL} )
option( BUILD_KOPETE "Build kopete" ${BUILD_ALL} )
option( BUILD_KPF "Build kpf" ${BUILD_ALL} )
option( BUILD_KPPP "Build kppp" ${BUILD_ALL} )
option( BUILD_KRDC "Build krdc" ${BUILD_ALL} )
option( BUILD_KRFB "Build krfb" ${BUILD_ALL} )
option( BUILD_KSIRC "Build ksirc" ${BUILD_ALL} )
option( BUILD_KTALKD "Build ktalkd" ${BUILD_ALL} )
option( BUILD_LANBROWSING "Build lanbrowsing" ${BUILD_ALL} )
option( BUILD_LIBRSS "Build librss" ${BUILD_ALL} )
option( BUILD_WIFI "Build wifi" ${BUILD_ALL} )


##### configure checks ##########################

include( ConfigureChecks.cmake )


##### kdenetwork directories ####################

kde_conditional_add_subdirectory( BUILD_DCOPRSS dcoprss )
kde_conditional_add_subdirectory( BUILD_DOC doc )
kde_conditional_add_subdirectory( BUILD_FILESHARING filesharing )
kde_conditional_add_subdirectory( BUILD_KDICT kdict )
kde_conditional_add_subdirectory( BUILD_KDNSSD kdnssd )
kde_conditional_add_subdirectory( BUILD_KFILE_PLUGINS kfile-plugins )
kde_conditional_add_subdirectory( BUILD_KGET kget )
kde_conditional_add_subdirectory( BUILD_KNEWSTICKER knewsticker )
kde_conditional_add_subdirectory( BUILD_KOPETE kopete )
kde_conditional_add_subdirectory( BUILD_KPF kpf )
kde_conditional_add_subdirectory( BUILD_KPPP kppp )
kde_conditional_add_subdirectory( BUILD_KRDC krdc )
kde_conditional_add_subdirectory( BUILD_KRFB krfb )
kde_conditional_add_subdirectory( BUILD_KSIRC ksirc )
kde_conditional_add_subdirectory( BUILD_KTALKD ktalkd )
kde_conditional_add_subdirectory( BUILD_LANBROWSING lanbrowsing )
kde_conditional_add_subdirectory( BUILD_LIBRSS librss )
kde_conditional_add_subdirectory( BUILD_WIFI wifi )


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )
