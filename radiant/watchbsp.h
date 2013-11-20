/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BSPWINDOW_H_
#define _BSPWINDOW_H_

#include "l_net/l_net.h"

class CWatchBSP
{
private:
// a flag we have set to true when using an external BSP plugin
// the resulting code with that is a bit dirty, cleaner solution would be to seperate the succession of commands from the listening loop
// (in two seperate classes probably)
bool m_bBSPPlugin;

// EIdle: we are not listening
//   DoMonitoringLoop will change state to EBeginStep
// EBeginStep: the socket is up for listening, we are expecting incoming connection
//   incoming connection will change state to EWatching
// EWatching: we have a connection, monitor it
//   connection closed will see if we start a new step (EBeginStep) or launch Quake3 and end (EIdle)
enum EWatchBSPState { EIdle, EBeginStep, EWatching } m_eState;
socket_t *m_pListenSocket;
socket_t *m_pInSocket;
netmessage_t msg;
GPtrArray *m_pCmd;
// used to timeout EBeginStep
GTimer    *m_pTimer;
unsigned int m_iCurrentStep;
// name of the map so we can run the engine
char    *m_sBSPName;
// buffer we use in push mode to receive data directly from the network
xmlParserInputBufferPtr m_xmlInputBuffer;
xmlParserInputPtr m_xmlInput;
xmlParserCtxtPtr m_xmlParserCtxt;
// run the game engine after successful compile
void RunQuake();
// call this to switch the set listening mode
bool SetupListening();
// start a new EBeginStep
void DoEBeginStep();
// the xml and sax parser state
char m_xmlBuf[MAX_NETMESSAGE];
bool m_bNeedCtxtInit;
message_info_s m_message_info;

public:
CWatchBSP() { m_bBSPPlugin = false; m_pListenSocket = NULL; m_pInSocket = NULL; m_eState = EIdle; m_pTimer = g_timer_new(); m_sBSPName = NULL; m_pCmd = NULL; m_iCurrentStep = 0; m_xmlInputBuffer = NULL; m_xmlParserCtxt = NULL; }
virtual ~CWatchBSP();
bool HasBSPPlugin() const
{ return m_bBSPPlugin; }

// called regularly to keep listening
void RoutineProcessing();
// start a monitoring loop with the following steps
void DoMonitoringLoop( GPtrArray *pCmd, char *sBSPName );
// close everything - may be called from the outside to abort the process
void Reset();
// start a listening loop for an external process, possibly a BSP plugin
void ExternalListen();
};

void WINAPI QERApp_Listen();

#endif
