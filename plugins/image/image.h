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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "synapse.h"
#include "qerplugin.h"
#include "ifilesystem.h"
#include "iimage.h"

#ifdef __linux__

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;

#endif // __linux__

extern _QERFuncTable_1 g_FuncTable;
extern _QERFileSystemTable g_FileSystemTable;
void LoadJPG( const char *filename, unsigned char **pic, int *width, int *height );

#define Error g_FuncTable.m_pfnError
#define vfsGetFileCount g_FileSystemTable.m_pfnGetFileCount
#define vfsLoadFile g_FileSystemTable.m_pfnLoadFile
#define vfsFreeFile g_FileSystemTable.m_pfnFreeFile

class CSynapseClientImage : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
bool OnActivate();
const char* GetName() { return "image"; }

CSynapseClientImage() { }
virtual ~CSynapseClientImage() { }
};

#endif // _IMAGE_H_
