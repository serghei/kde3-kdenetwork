/**
 *  Copyright 2002 David Faure <faure@kde.org>
 *
 *  Redistribution and use in source and binary forms, with or
 *    without modification, are permitted provided that the following
 *    conditions are met:
 *
 *    - Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *    PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 * (BSD License, from kdelibs/doc/common/bsd-license.html)
 */

#ifndef __INCLUD_H
#define __INCLUD_H "$Id"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include "prot_talkd.h"
#define NEW_NAME_SIZE NAME_SIZE
#define NEW_CTL_MSG CTL_MSG
#define NEW_CTL_RESPONSE CTL_RESPONSE
#include "otalkd.h"

#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifndef UT_LINESIZE
#define UT_LINESIZE 12
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_UTMP
#define _PATH_UTMP UTMP
#endif
#ifndef _PATH_DEV
#define _PATH_DEV "/dev/"
#endif
#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#endif /* __INCLUD_H */
