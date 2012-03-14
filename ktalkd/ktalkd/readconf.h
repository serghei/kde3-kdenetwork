/* This file is part of ktalkd
 *
 *  Copyright (C) 1997 David Faure (faure@kde.org)
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

/** Warning to ktalkd hackers : this file is the definition of a
 * generic interface for reading systemwide and user configuration.
 * Two files implement it : readconf.cpp for non-KDE users
 * and readcfg++.cpp for KDE users.
 */

  /**
        * Initiate user-config-file reading.
        *
        * @param l_name User name
        */
int init_user_config(const char * l_name);

  /**
        * Read one entry in user-config-file.
        *
        * @param key Key, such as "Mail", "Subj", "Head", "Msg1".."Msg3"
        */
int read_user_config(const char * key, char * result, int max);

  /**
        * Read a boolean entry in user-config-file.
        *
        * @param key Key, such as "Sound"
        */
int read_bool_user_config(const char * key, int * result);

  /**
        * Close user-config-file and destroys objects used.
        */
void end_user_config();

  /**
        * Read all site-wide configuration in one pass
        */
int process_config_file(void);    
