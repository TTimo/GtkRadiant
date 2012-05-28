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

//===========================================================================
//
// Name:         l_net_wins.h
// Function:     WinSock
// Programmer:   MrElusive
// Last update:  TTimo: cross-platform version, l_net library
// Tab Size:     3
// Notes:
//===========================================================================

int  WINS_Init( void );
void WINS_Shutdown( void );
char *WINS_MyAddress( void );
int  WINS_Listen( int socket );
int  WINS_Accept( int socket, struct sockaddr_s *addr );
int  WINS_OpenSocket( int port );
int  WINS_OpenReliableSocket( int port );
int  WINS_CloseSocket( int socket );
int  WINS_Connect( int socket, struct sockaddr_s *addr );
int  WINS_CheckNewConnections( void );
int  WINS_Read( int socket, byte *buf, int len, struct sockaddr_s *addr );
int  WINS_Write( int socket, byte *buf, int len, struct sockaddr_s *addr );
int  WINS_Broadcast( int socket, byte *buf, int len );
char *WINS_AddrToString( struct sockaddr_s *addr );
int  WINS_StringToAddr( char *string, struct sockaddr_s *addr );
int  WINS_GetSocketAddr( int socket, struct sockaddr_s *addr );
int  WINS_GetNameFromAddr( struct sockaddr_s *addr, char *name );
int  WINS_GetAddrFromName( char *name, struct sockaddr_s *addr );
int  WINS_AddrCompare( struct sockaddr_s *addr1, struct sockaddr_s *addr2 );
int  WINS_GetSocketPort( struct sockaddr_s *addr );
int  WINS_SetSocketPort( struct sockaddr_s *addr, int port );
