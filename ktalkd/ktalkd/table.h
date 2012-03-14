/*
 * Copyright (c) 1983  Regents of the University of California, (c) 1997 David Faure
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
 *
 */
#ifndef TABLE_H
#define TABLE_H

#include "includ.h"
#include <sys/time.h>
#include <time.h>

class ForwMachine;

typedef struct table_entry TABLE_ENTRY;

struct table_entry {
    NEW_CTL_MSG request;
    long	time;
    ForwMachine * fwm;
    TABLE_ENTRY *next;
    TABLE_ENTRY *last;
};

class KTalkdTable
{
  public:
    KTalkdTable() : table (0L) {};
    ~KTalkdTable();
    void insert_table(NEW_CTL_MSG *request, NEW_CTL_RESPONSE *response, ForwMachine * fwm);
    int delete_invite(uint32_t id_num);
    NEW_CTL_MSG * find_match(NEW_CTL_MSG *request);
    NEW_CTL_MSG * find_request(NEW_CTL_MSG *request);
    int new_id();
    TABLE_ENTRY * getTable() { return table; }

  private:
    void delete_entry(register TABLE_ENTRY *ptr);

    struct timeval tp;
    struct timezone txp;

    TABLE_ENTRY *table;
};
#endif
