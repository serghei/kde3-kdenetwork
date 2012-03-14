/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
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

/* debug print routines */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <config.h>
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#include <errno.h>
#include "prot_talkd.h"
#include "proto.h"

#ifndef _PATH_VAR_LOG
#define _PATH_VAR_LOG "/var/log/"
#endif

#define	NTYPES 4
static const char *types[NTYPES] = { 
  "LEAVE_INVITE", 
  "LOOK_UP", 
  "DELETE", 
  "ANNOUNCE" 
};

#define NANSWERS 9
static const char *answers[NANSWERS] = { 
  "SUCCESS", 
  "NOT_HERE", 
  "FAILED", 
  "MACHINE_UNKNOWN", 
  "PERMISSION_DENIED", 
  "UNKNOWN_REQUEST", 
  "BADVERSION", 
  "BADADDR", 
  "BADCTLADDR" 
};

static int logging, badpackets;
static int logfd, packfd;

void
set_debug(int l, int b)
{
	const char *file;
	logging = l;
	badpackets = b;
	if (logging) {
		file = _PATH_VAR_LOG "talkd.log";
		logfd = open(file, O_WRONLY|O_APPEND);
		if (logfd<0) {
			syslog(LOG_WARNING, "%s: %s", file, strerror(errno));
			logging = 0;
		}
	}
	if (badpackets) {
		file = _PATH_VAR_LOG "talkd.packets";
		packfd = open(file, O_WRONLY|O_APPEND);
		if (packfd<0) {
			syslog(LOG_WARNING, "%s: %s", file, strerror(errno));
			badpackets = 0;
		}
	}
}

/****************** ktalkd addition ************/

/* print_addr is a debug print routine for sockaddr_in structures.
 * Call with a structure in network byte order.
 * @param cp a string to identify the log output
 * @param addr the address to read
 * This code is obviously NOT IPv6 ready :)
 */
static void print_addr(const char *cp, struct sockaddr_in * addr)
{
	int a0,a1,a2,a3;
	unsigned int s_add = addr->sin_addr.s_addr;
	char buf[1024];
	a0 = s_add % 256L;
	s_add /= 256L;
	a1 = s_add % 256L;
	s_add /= 256L;
	a2 = s_add % 256L;
	s_add /= 256L;
	a3 = s_add % 256L;
	snprintf(buf, sizeof(buf), "%s: addr = %d.%d.%d.%d port = %o, family = %o",
		 cp, a0, a1, a2, a3, ntohs(addr->sin_port), ntohs(addr->sin_family));
	write(logfd, buf, strlen(buf));
}

/**********************************************/

static const char *
print_type(int type)
{
	static char rv[80];
	if (type > NTYPES) {
		snprintf(rv, sizeof(rv), "type %d", type);
		return rv;
	} 
	return types[type];
}

static const char *
print_answer(int answer)
{
	static char rv[80];
	if (answer > NANSWERS) {
		snprintf(rv, sizeof(rv), "answer %d", answer);
		return rv;
	} 
	return answers[answer];
}

void
print_request(const char *cp, const CTL_MSG *mp)
{
	char lu[NAME_SIZE+1], ru[NAME_SIZE+1], tt[TTY_SIZE+1];
	char buf[1024];
	const char *tp;
	if (!logging) return;

	tp = print_type(mp->type);
	strncpy(lu, mp->l_name, sizeof(lu));
	strncpy(ru, mp->r_name, sizeof(ru));
	strncpy(tt, mp->r_tty, sizeof(tt));
	lu[sizeof(lu)-1]=0;
	ru[sizeof(ru)-1]=0;
	tt[sizeof(tt)-1]=0;
	
	snprintf(buf, sizeof(buf), 
		 "%s: %s: id %u, l_user %s, r_user %s, r_tty %s\n",
		 cp, tp, mp->id_num, lu, ru, tt);
	write(logfd, buf, strlen(buf));
        print_addr("    addr", (struct sockaddr_in *)&mp->addr);
        print_addr("    ctl_addr", (struct sockaddr_in *)&mp->ctl_addr);
}

void
print_response(const char *cp, const CTL_RESPONSE *rp)
{
	char buf[1024];
	const char *tp, *ap;
	if (!logging) return;

	tp = print_type(rp->type);
	ap = print_answer(rp->answer);
	
	snprintf(buf, sizeof(buf), 
		 "%s: %s <-- %s, id %d\n",
		 cp, tp, ap, ntohl(rp->id_num));
	write(logfd, buf, strlen(buf));
        if ((rp->type == LOOK_UP) && (rp->answer == SUCCESS))
            print_addr("    resp addr", (struct sockaddr_in *)&rp->addr);
}

void
ktalk_debug(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	if (!logging) return;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	write(logfd, buf, strlen(buf));
}

void
print_broken_packet(const char *pack, size_t len, struct sockaddr_in *from)
{
	size_t i;
	char tmp[4], buf[128];
	if (!badpackets) return;
	snprintf(buf, sizeof(buf), "From: %s [%u]", 
		 inet_ntoa(from->sin_addr), from->sin_addr.s_addr);
	write(packfd, buf, strlen(buf));
	for (i=0; i<len; i++) {
	    if (i%24 == 0) write(packfd, "\n    ", 5);
	    snprintf(tmp, sizeof(tmp), "%02x ", (unsigned char)pack[i]);
	    write(packfd, tmp, strlen(tmp));
	}
	write(packfd, "\n", 1);
}
