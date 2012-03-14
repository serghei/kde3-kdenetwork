/*
 * Copyright (c) 1983 Regents of the University of California, (c) 1998 David Faure
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
#ifndef TALKCONNECTION_H
#define TALKCONNECTION_H

#include "../includ.h"

enum ProtocolType {noProtocol, talkProtocol, ntalkProtocol};

class TalkConnection
{
  public:

    /** Global initialization. To be called once. */
    static void init();

    /** Create a talk connection.
     * @param r_addr Remote machine IP address
     * @param r_name Remote user name
     * @param l_name Local user name
     * @param _protocol Caller's protocol */
    TalkConnection(struct in_addr r_addr,
                   char * r_name,
                   char * l_name,
                   ProtocolType _protocol);

    /** Destructor. Closes the sockets if opened.*/
    ~TalkConnection();

    /** Create the sockets */
    void open_sockets();
    /** Close the sockets */
    void close_sockets();

    /** Methods for talking to remote daemon */
    void ctl_transact(int type, int id_num);
    int look_for_invite(int mandatory);

    /** Connect with address given back in response to LOOK_UP. */
    int connect();
    /** Prepare to accept a connection from another talk client */
    void listen();
    /** Accept a connection from another talk client */
    int accept();

    /** Exchange the first 3 characters, which are edit characters */
    void set_edit_chars();

    /** Write data into the socket, by 16 char blocks. */
    void write_banner(char * banner);

    // Methods to retrieve some information
    /** Returns the erase char used by the caller. */
    char get_char_erase() { return char_erase; }
    /** Returns the caller's name. */
    char * get_caller_name() { return new_msg.r_name; } // idem in old_msg
    /** Returns socket, for reading or writing */
    int get_sockt() { return sockt; }
    /** Returns response answer and id_num. Allows protocol independence.
     * Each param can be null (0L). Then it isn't filled.*/
    void getResponseItems(uint8_t * answer, uint32_t * id_num, struct talk_addr * addr);
    /** Returns connection socket address. For FWT. */
    const struct talk_addr get_addr() { return new_msg.addr; } // idem in old_msg

    // Methods to cheat with this talk connection
    // Used by the forwarding machine
    void set_addr(const struct talk_addr * addr);
    void set_ctl_addr(const struct talk_addr * ctl_addr);

    /** Prints the system error message in the log and exits the current thread */
    static void p_error(const char * str);

  protected:

    /** Used by open_sockets. */
    int open_socket (struct sockaddr_in *addr, int type);
    /** Check remote protocol. Used by ctl_transact. */
    void findProtocol();
    /** Find the correct IP address that the daemon at host
        "destination" has to respond to */
    static struct in_addr getReplyAddr (struct in_addr destination);
    static struct  in_addr defaultReplyAddr;

    static short int talkDaemonPort;          // Port number of talk demon  (517)
    static short int ntalkDaemonPort;         // Port number of ntalk demon (518)

    ProtocolType protocol;

    /* inet addresses of the two machines */
    struct  in_addr my_machine_addr;
    struct  in_addr his_machine_addr;

    int     ctl_sockt;
    int     sockt;

    OLD_CTL_MSG old_msg; // holds interesting data
    NEW_CTL_MSG new_msg; // holds interesting data
    OLD_CTL_RESPONSE old_resp; // only convenience structure for responses
    NEW_CTL_RESPONSE new_resp; // only convenience structure for responses
    struct talk_addr lookup_addr; // address returned by LOOKUP. Points to xxx_resp.addr

    char char_erase;
};

#endif
