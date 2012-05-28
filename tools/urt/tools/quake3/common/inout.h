/*
   This code is based on source provided under the terms of the Id Software
   LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with the
   GtkRadiant sources (see LICENSE_ID). If you did not receive a copy of
   LICENSE_ID, please contact Id Software immediately at info@idsoftware.com.

   All changes and additions to the original source which have been developed by
   other contributors (see CONTRIBUTORS) are provided under the terms of the
   license the contributors choose (see LICENSE), to the extent permitted by the
   LICENSE_ID. If you did not receive a copy of the contributor license,
   please contact the GtkRadiant maintainers at info@gtkradiant.com immediately.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __INOUT__
#define __INOUT__

// inout is the only stuff relying on xml, include the headers there
#include "libxml/tree.h"

// some useful xml routines
xmlNodePtr xml_NodeForVec( vec3_t v );
void xml_SendNode( xmlNodePtr node );
// print a message in q3map output and send the corresponding select information down the xml stream
// bError: do we end with an error on this one or do we go ahead?
void xml_Select( char *msg, int entitynum, int brushnum, qboolean bError );
// end q3map with an error message and send a point information in the xml stream
// note: we might want to add a boolean to use this as a warning or an error thing..
void xml_Winding( char *msg, vec3_t p[], int numpoints, qboolean die );
void xml_Point( char *msg, vec3_t pt );

extern qboolean bNetworkBroadcast;
void Broadcast_Setup( const char *dest );
void Broadcast_Shutdown();

#define SYS_VRB 0 // verbose support (on/off)
#define SYS_STD 1 // standard print level
#define SYS_WRN 2 // warnings
#define SYS_ERR 3 // error
#define SYS_NOXML 4 // don't send that down the XML stream

extern qboolean verbose;
void Sys_Printf( const char *text, ... );
void Sys_FPrintf( int flag, const char *text, ... );

#ifdef _DEBUG
#define DBG_XML 1
#endif

#ifdef DBG_XML
void DumpXML();
#endif

#endif
