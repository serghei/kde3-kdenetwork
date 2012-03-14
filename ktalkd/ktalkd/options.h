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

#ifndef OPTIONS_H
#define OPTIONS_H
/** This struct handles global options. Not user ones */

#include <sys/utsname.h>
#ifndef SYS_NMLN
#define SYS_NMLN  65             /* max hostname size */
#endif

struct sOptions
{ 
    int answmach; /* used in talkd.cpp */
    int time_before_answmach; /* in machines/answmach.cpp */
    /* used in announce.cpp : */
    int sound;
    int XAnnounce;
    char soundfile [S_CFGLINE];
    char soundplayer [S_CFGLINE];
    char soundplayeropt [S_CFGLINE];
    char announce1 [S_CFGLINE];
    char announce2 [S_CFGLINE];
    char announce3 [S_CFGLINE];
    char invitelines [S_INVITE_LINES];/* used in machines/answmach.cpp */
    char mailprog [S_CFGLINE]; /* used in machines/answmach.cpp */
    int NEU_behaviour;
    char NEU_user[S_CFGLINE];
    char NEUBanner1 [S_CFGLINE];
    char NEUBanner2 [S_CFGLINE];
    char NEUBanner3 [S_CFGLINE];
    char NEU_forwardmethod [5];
    char extprg [S_CFGLINE];
    // No really an option, but it's useful to have it here :
    char hostname[SYS_NMLN];
    int debug_mode;
};

extern struct sOptions Options;

#endif
