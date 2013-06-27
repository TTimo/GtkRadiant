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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// header for xml stuff used in radiant
//

#ifndef __XMLSTUFF__
#define __XMLSTUFF__

#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/parser.h"

struct message_info_s;

class ISAXHandler
{
public:
virtual ~ISAXHandler() { }
virtual void saxStartElement( struct message_info_s *ctx, const xmlChar *name, const xmlChar **attrs ) = 0;
virtual void saxEndElement( struct message_info_s *ctx, const xmlChar *name ) = 0;
virtual void saxCharacters( struct message_info_s *ctx, const xmlChar *ch, int len ) = 0;
virtual char *getName() { return NULL; }
virtual void Highlight() { }
virtual void DropHighlight() { }
virtual bool ShouldDelete() { return true; }   // should the handler be deleted when the feedback dialog is cleared?
};

// a 'user data' structure we pass along in the SAX callbacks to represent the current state
// the recurse value tracks the current depth in the tree
// message_info also stores information to exit the stream listening cleanly with an error:
//   if msg_level == SYS_ERR, then we will reset the listening at the end of the current node
//   the level for stopping the feed is stored in stop_depth
// unkown nodes are ignored, we use ignore_depth to track the level we start ignoring from
typedef struct message_info_s {
	int msg_level;                  // current message level (SYS_MSG, SYS_WRN, SYS_ERR)
	int recurse;                    // current recursion depth (used to track various things)
	int ignore_depth;               // the ignore depth limit when we are jumping over unknown nodes (0 means we are not ignoring)
	int stop_depth;                 // the depth we need to stop at the end
	bool bGeometry;                 // are we parsing some geometry information (i.e. do we forward the SAX calls?)
	ISAXHandler       *pGeometry;   // the handler
} message_info_t;

#endif
