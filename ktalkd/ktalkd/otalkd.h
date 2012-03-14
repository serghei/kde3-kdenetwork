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

#ifndef OTALKD_H
#define OTALKD_H

#include "prot_talkd.h"

#define OLD_NAME_SIZE 9

/* Control Message structure for old talk protocol (earlier than BSD4.2) */

typedef struct {
  char     type;                   /* request type, see below */
  char     l_name [OLD_NAME_SIZE]; /* caller's name */
  char     r_name [OLD_NAME_SIZE]; /* callee's name */
  char     pad;
  int      id_num;                 /* message id */
  int      pid;                    /* caller's process id */
  char     r_tty [TTY_SIZE];   /* callee's tty name */
  struct talk_addr addr;            /* socket address for connection */
  struct talk_addr ctl_addr;        /* control socket address */
} OLD_CTL_MSG;

/* Control Response structure for old talk protocol (earlier than BSD4.2) */

typedef struct {
  char      type;         /* type of request message, see below */
  char      answer;       /* response to request message, see below */
  char      pad [2];
  int       id_num;       /* message id */
  struct talk_addr addr;   /* address for establishing conversation */
} OLD_CTL_RESPONSE;

#endif
