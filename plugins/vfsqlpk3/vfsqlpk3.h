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

#ifndef _VFSQLPK3_H_
#define _VFSQLPK3_H_

#ifdef __linux__

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;

#endif // __linux__

#include "synapse.h"
#include "qerplugin.h"
#include "ifilesystem.h"

extern _QERFuncTable_1 g_FuncTable;
extern CSynapseServer* g_pSynapseServer;

#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf

class CSynapseClientVFS : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientVFS() { }
virtual ~CSynapseClientVFS() { }
};

#endif // _VFSQLPK3_H_
