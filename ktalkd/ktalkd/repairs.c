/*
 * Copyright 1998 David A. Holland.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder(s) nor the names of their
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char repairs_rcsid[] = 
  "$Id: repairs.c 501248 2006-01-22 14:37:01Z mueller $";

/*
 * Most, but not quite all, of the voodoo for detecting and handling
 * malformed packets goes here.
 *
 * Basically anything which we can detect by examining the packet we
 * try to do in rationalize_packet(). We return a quirk code for what
 * we did so we can send back replies that will make sense to the other
 * end. That's irrationalize_reply().
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <syslog.h>
#include "prot_talkd.h"
#include "proto.h"

uint32_t
byte_swap32(uint32_t k)
{
	return (k<<24) | ((k&0xff00) << 8) | ((k>>8) & 0xff00)  | (k>>24);
}

static uint16_t
byte_swap16(uint16_t k)
{
	return (k<<8) | (k>>8);
}

/***************************************************/

/* 
 * probe for strings that are meaningful in talkd packets.
 * rejects all control characters and delete. newlines and tabs have
 * no business in tty names or usernames.
 */
static int probe_string(const char *buf, size_t len) {
    size_t i;
    int ch;
    for (i=0; i<len; i++) {
	if (buf[i]==0) return 0;  /* success */
	ch = (unsigned char) buf[i];
	if ((ch&127) < 32 || (ch&127)==127) return -1;
    }
    return -1; /* no null-terminator, assume it's not a string */
}

/*
 * Check if an address from a talk packet matches the actual sender
 * address. If it doesn't, it's a good bet it's not the right packet format.
 * Allow assorted endianness though.
 * In an ideal world we'd save the endianness info for use elsewhere instead
 * of reprobing it, but oh well.
 */
static int probe_addr(struct talk_addr *ta, struct sockaddr_in *sn) {
    uint16_t family = sn->sin_family;
    uint16_t xfamily = byte_swap16(family);
    uint16_t port = sn->sin_port;
    uint16_t xport = byte_swap16(port);
    uint32_t addr = sn->sin_addr.s_addr;
    uint32_t xaddr = byte_swap32(addr);

    if (ta->ta_family != family && ta->ta_family != xfamily) return -1;
    if (ta->ta_port != port && ta->ta_port != xport) return -1;
    if (ta->ta_addr != addr && ta->ta_addr != xaddr) return -1;
    return 0;
}

/***************************************************/

/*
 * warning warning: in some cases this packet may need compiler
 * pragmas to force the compiler to not pad it. shouldn't with
 * gcc though.
 */

#define OTALK_PACKET_SIZE 76

#define OLD_NAME_SIZE   9
struct otalk_packet {
	char type;
	char l_name[OLD_NAME_SIZE];
	char r_name[OLD_NAME_SIZE];
	char filler;
	uint32_t id_num;
	uint32_t pid;
	char r_tty[TTY_SIZE];
	struct talk_addr addr;
	struct talk_addr ctl_addr;
};

struct otalk_reply {
	char type;
	char answer;
	uint16_t filler;
	uint32_t id_num;
	struct talk_addr addr;
};

/* additional types */
#define OLD_DELETE_INVITE  4
#define OLD_AUTO_LOOK_UP   5
#define OLD_AUTO_DELETE    6

static int probe_otalk_packet(char *buf, size_t len, size_t maxlen, 
			      struct sockaddr_in *sn)
{
	struct otalk_packet otp;
	CTL_MSG m;

	ktalk_debug("Probing for QUIRK_OTALK\n");

	if (sizeof(otp)!=OTALK_PACKET_SIZE) {
		syslog(LOG_ERR, "QUIRK_OTALK: struct otalk_packet padding "
		       "is wrong\n");
		return -1;
	}

	if (len!=sizeof(otp)) {
		ktalk_debug("QUIRK_OTALK: wrong size\n");
		return -1;
	}
	
	memcpy(&otp, buf, len);
	if (probe_string(otp.l_name, sizeof(otp.l_name))) {
		ktalk_debug("QUIRK_OTALK: l_name not a string\n");
		return -1;
	}
	if (probe_string(otp.r_name, sizeof(otp.r_name))) {
		ktalk_debug("QUIRK_OTALK: r_name not a string\n");
		return -1;
	}
	if (probe_string(otp.r_tty, sizeof(otp.r_tty))) {
		ktalk_debug("QUIRK_OTALK: r_tty not a string\n");
		return -1;
	}
	if (probe_addr(&otp.ctl_addr, sn)) {
		ktalk_debug("QUIRK_OTALK: addresses do not match\n");
		return -1;
	}

	switch (otp.type) {
	    case LEAVE_INVITE:
	    case LOOK_UP:
	    case DELETE:
	    case ANNOUNCE:
		break;
	    /* I'm not sure these will work. */
            case OLD_DELETE_INVITE: otp.type = DELETE; break;
	    case OLD_AUTO_LOOK_UP: otp.type = LOOK_UP; break;
	    case OLD_AUTO_DELETE: otp.type = DELETE; break;
	    default: 
		ktalk_debug("QUIRK_OTALK: invalid type field\n");
		return -1;
	}

	if (OLD_NAME_SIZE >= NAME_SIZE) {
		syslog(LOG_ERR, "QUIRK_OTALK: OLD_NAME_SIZE >= NAME_SIZE\n");
		syslog(LOG_ERR, "QUIRK_OTALK: fix repairs.c and recompile\n");
		return -1;
	}
	if (maxlen < sizeof(CTL_MSG)) {
		syslog(LOG_ERR, "QUIRK_OTALK: maxlen too small; enlarge "
		       "inbuf[] in talkd.c and recompile\n");
		return -1;
	}

	m.vers = TALK_VERSION;
	m.type = otp.type;
	m.answer = 0;
	m.pad = 0;
	m.id_num = otp.id_num;
	m.addr = otp.addr;
	m.ctl_addr = otp.ctl_addr;
	m.pid = otp.pid;
	memcpy(m.l_name, otp.l_name, OLD_NAME_SIZE);
	m.l_name[OLD_NAME_SIZE] = 0;
	memcpy(m.r_name, otp.r_name, OLD_NAME_SIZE);
	m.r_name[OLD_NAME_SIZE] = 0;
	memcpy(m.r_tty, otp.r_tty, TTY_SIZE);
	m.r_tty[TTY_SIZE-1] = 0;
	memcpy(buf, &m, sizeof(m));
	return 0;
}

static size_t do_otalk_reply(char *buf, size_t maxlen) {
	struct otalk_reply or;
	CTL_RESPONSE *r = (CTL_RESPONSE *)buf;
	if (sizeof(or) > maxlen) {
		syslog(LOG_ERR, "QUIRK_OTALK: reply: maxlen too small; "
		       "enlarge buf[] in send_packet and recompile\n");
		return sizeof(CTL_RESPONSE);
	}
	
	/* 
	 * If we changed the type above, this might break. Should we encode
	 * it in the quirk code?
	 */
	or.type = r->type;
	or.answer = r->answer;
	or.filler = 0;
	or.id_num = r->id_num;
	or.addr = r->addr;
	memcpy(buf, &or, sizeof(or));
	return sizeof(or);
}

/***************************************************/


/*
 * Return 0 if the packet's normal, -1 if we can't figure it out,
 * otherwise a quirk code from the quirk list.
 *
 * For now, we don't support any quirks. Need more data.
 */
int
rationalize_packet(char *buf, size_t len, size_t mlen, struct sockaddr_in *sn)
{
	if (len == sizeof(CTL_MSG)) {
		return 0;
	}

	ktalk_debug("Malformed packet (length %u)\n", len);

	if (probe_otalk_packet(buf, len, mlen, sn)==0) {
		ktalk_debug("Using QUIRK_OTALK\n");
		return QUIRK_OTALK;
	}
	return -1;
}

size_t
irrationalize_reply(char *buf, size_t maxlen, int quirk)
{
        switch (quirk) {
	    case QUIRK_NONE: return sizeof(CTL_RESPONSE);
	    case QUIRK_OTALK: return do_otalk_reply(buf, maxlen);
        }
	/* ? */
	return 0;
}
