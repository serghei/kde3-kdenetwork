/*
 * Copyright 1999-2002 David Faure <faure@kde.org>
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

#include "defs.h"

struct sOptions Options = {
/* answmach             :*/ 1,
/* time_before_answmach :*/ 20,
/* sound                :*/ 0,
/* XAnnounce            :*/ 1,
/* soundfile            :*/ {""},
/* soundplayer          :*/ {""},
/* soundplayeropt       :*/ {""},
/* announce1            :*/ {ANNOUNCE1},
/* announce2            :*/ {ANNOUNCE2},
/* announce3            :*/ {ANNOUNCE3},
/* invitelines          :*/ {INVITE_LINES},
/* mailprog             :*/ {"mail.local"},
/* NEU_behaviour        :*/ 2,
/* NEU_user             :*/ {""},
/* NEUBanner1           :*/ {NEU_BANNER1},
/* NEUBanner2           :*/ {NEU_BANNER2},
/* NEUBanner3           :*/ {NEU_BANNER3},
/* NEU_forwardmethod    :*/ {"FWR"},
/* extprg               :*/ {""},
/* hostname             :*/ {""},
/* debug_mode           :*/ 0
};
