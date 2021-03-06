/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products 
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include "talk/xmpp/xmppengine.h"
#include "talk/xmpp/xmpptask.h"
#include "talk/p2p/client/sessionclient.h"

namespace cricket {

class Receiver : public buzz::XmppTask {
public:
  Receiver(Task *parent, SessionClient *session_client)
    : buzz::XmppTask(parent, buzz::XmppEngine::HL_TYPE) {
    session_client_ = session_client;
  }

  virtual int ProcessStart() {
    const buzz::XmlElement *stanza = NextStanza();
    if (stanza == NULL)
      return STATE_BLOCKED;
    session_client_->OnIncomingStanza(stanza);

    // Respond right away to the sender to let them know that we received
    // this IQ
    buzz::XmlElement * result = MakeIqResult(stanza);
    SendStanza(result);

    return STATE_START;
  }

protected:
  virtual bool HandleStanza(const buzz::XmlElement *stanza) {
    if (!session_client_->IsClientStanza(stanza))
      return false;
    QueueStanza(stanza);
    return true;
  }

private:
  SessionClient *session_client_;
};

}

#endif // _RECEIVER_H_
