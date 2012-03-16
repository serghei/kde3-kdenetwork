// krdp
#define VERSION "@VERSION@"

// NOTE:
// this definition is safe for modern Linux systems
// for other OSes can be necessary to check
#define STDC_HEADERS 1

// kopete/libkopete
#cmakedefine HAVE_V4L 1
#cmakedefine HAVE_LIBV4L1_VIDEODEV_H 1

// kopete/protocols/yahoo
#cmakedefine HAVE_STRINGS_H 1

// kopete/protocols/yahoo, kppp
#cmakedefine HAVE_STRING_H 1

// kopete/protocols/jabber
#cmakedefine SUPPORT_JINGLE 1
#cmakedefine HAVE_GLIB 1
#cmakedefine HAVE_SPEEX 1

// kppp
#cmakedefine HAVE_SYS_PARAM_H 1
#cmakedefine HAVE_NET_IF_PPP_H 1

// ktalkd
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_FILE_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_UTMP_H 1
#cmakedefine HAVE_VSNPRINTF 1
#define SCANDIR_NEEDS_CONST 1
// only for Linux
#define PROC_FIND_USER 1
#define UTMP_AND_PROC_FIND_USER 1
#define ALL_PROCESSES_AND_PROC_FIND_USER 1

// wifi
#cmakedefine WITHOUT_ARTS 1

#ifdef __cplusplus
extern "C" {
unsigned long strlcpy(char*, const char*, unsigned long);
unsigned long strlcat(char*, const char*, unsigned long);
}
#endif
