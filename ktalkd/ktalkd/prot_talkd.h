/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *
 *	@(#)talkd.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _PROTOCOLS_TALKD_H
#define	_PROTOCOLS_TALKD_H

/*
 * This describes the protocol used by the talk server and clients.
 *
 * The talk server acts a repository of invitations, responding to
 * requests by clients wishing to rendezvous for the purpose of
 * holding a conversation.  In normal operation, a client, the caller,
 * initiates a rendezvous by sending a CTL_MSG to the server of
 * type LOOK_UP.  This causes the server to search its invitation
 * tables to check if an invitation currently exists for the caller
 * (to speak to the callee specified in the message).  If the lookup
 * fails, the caller then sends an ANNOUNCE message causing the server
 * to broadcast an announcement on the callee's login ports requesting
 * contact.  When the callee responds, the local server uses the
 * recorded invitation to respond with the appropriate rendezvous
 * address and the caller and callee client programs establish a
 * stream connection through which the conversation takes place.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>

/*
 * This is a copy of 4.3BSD struct sockaddr.
 */
struct talk_addr {
  uint16_t ta_family;
  uint16_t ta_port;
  uint32_t ta_addr;
  uint32_t ta_junk1;
  uint32_t ta_junk2;
};


/*
 * Client->server request message format.
 */
#define	NAME_SIZE	12
#define	TTY_SIZE	16

typedef struct {
	uint8_t	vers;		/* protocol version */
	uint8_t	type;		/* request type, see below */
	uint8_t	answer;		/* not used */
	uint8_t	pad;
	uint32_t	id_num;		/* message id */
	struct talk_addr addr;		/* address of client tcp port */
	struct talk_addr ctl_addr;	/* address of client udp port */
	uint32_t	pid;		/* caller's process id */
	char		l_name[NAME_SIZE];/* caller's name */
	char		r_name[NAME_SIZE];/* callee's name */
	char		r_tty[TTY_SIZE];/* callee's tty name */
} CTL_MSG;

/*
 * Server->client response message format.
 */
typedef struct {
	uint8_t    vers;	/* protocol version */
	uint8_t    type;	/* type of request message, see below */
	uint8_t    answer;	/* respose to request message, see below */
	uint8_t    pad;
	uint32_t   id_num;	/* message id */
	struct	talk_addr addr;	/* address for establishing conversation */
} CTL_RESPONSE;

#define	TALK_VERSION	1		/* protocol version */

/* message type values */
#define LEAVE_INVITE	0	/* leave invitation with server */
#define LOOK_UP		1	/* check for invitation by callee */
#define DELETE		2	/* delete invitation by caller */
#define ANNOUNCE	3	/* announce invitation by caller */

/* answer values */
#define SUCCESS		0	/* operation completed properly */
#define NOT_HERE	1	/* callee not logged in */
#define FAILED		2	/* operation failed for unexplained reason */
#define MACHINE_UNKNOWN	3	/* caller's machine name unknown */
#define PERMISSION_DENIED 4	/* callee's tty doesn't permit announce */
#define UNKNOWN_REQUEST	5	/* request has invalid type value */
#define	BADVERSION	6	/* request has invalid protocol version */
#define	BADADDR		7	/* request has invalid addr value */
#define	BADCTLADDR	8	/* request has invalid ctl_addr value */

/*
 * Operational parameters.
 * RING_WAIT should be 10's of seconds less than MAX_LIFE
 */
#define MAX_LIFE	60	/* max time daemon saves invitations */
#define RING_WAIT	30	/* time to wait before resending invitation */

#endif /* !_PROTOCOLS_TALKD_H */
