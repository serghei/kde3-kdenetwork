/**
 * Copyright (c) 1983, 1993
 *·     The Regents of the University of California.  All rights reserved.
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

#ifdef __cplusplus
extern "C" {
#endif

/* quirks for repairs.c */

#define QUIRK_NONE   0
#define QUIRK_OTALK  1

struct sockaddr_in;

extern char ourhostname[];

/* print.c */
void print_request(const char *cp, const CTL_MSG *mp);
void print_response(const char *cp, const CTL_RESPONSE *rp);
void print_broken_packet(const char *pack, size_t len, struct sockaddr_in *);
void ktalk_debug(const char *fmt, ...);
void set_debug(int logging, int badpackets);

/* table.c */
/*void insert_table(CTL_MSG *request, CTL_RESPONSE *response);
  CTL_MSG *find_request(CTL_MSG *request);
  CTL_MSG *find_match(CTL_MSG *request);
*/

/* repairs.c */
uint32_t byte_swap32(uint32_t);
int rationalize_packet(char *buf, size_t len, size_t maxlen,
		       struct sockaddr_in *);
size_t irrationalize_reply(char *buf, size_t maxbuf, int quirk);

/* other */
/*int announce(CTL_MSG *request, const char *remote_machine);
void process_request(CTL_MSG *mp, CTL_RESPONSE *rp, const char *fromhost);
int new_id(void);
int delete_invite(unsigned id_num);
*/

#ifdef __cplusplus
}
#endif
