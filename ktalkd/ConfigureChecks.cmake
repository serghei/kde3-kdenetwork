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

include( CheckFunctionExists )

check_include_file( sys/types.h HAVE_SYS_TYPES_H )
check_include_file( sys/stat.h HAVE_SYS_STAT_H )
check_include_file( sys/file.h HAVE_SYS_FILE_H )
check_include_file( unistd.h HAVE_UNISTD_H )
check_include_file( utmp.h HAVE_UTMP_H )

check_function_exists( vsnprintf HAVE_VSNPRINTF )
