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

//====================================================================
//
// Name:			l_net.h
// Function:		-
// Programmer:		MrElusive
// Last update:		TTimo: cross-platform version, l_net library
// Tab size:		2
// Notes:
//====================================================================

//++timo FIXME: the l_net code understands that as the max size for the netmessage_s structure
//  we have defined unsigned char data[MAX_NETMESSAGE] in netmessage_s but actually it cannot be filled completely
//  we need to introduce a new #define and adapt to data[MAX_NETBUFFER]
#define MAX_NETMESSAGE      1024
#define MAX_NETADDRESS      32

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum { qfalse, qtrue } qboolean;
typedef unsigned char byte;
#endif

typedef struct address_s
{
	char ip[MAX_NETADDRESS];
} address_t;

typedef struct sockaddr_s
{
	short sa_family;
	unsigned char sa_data[14];
} sockaddr_t;

typedef struct netmessage_s
{
	unsigned char data[MAX_NETMESSAGE];
	int size;
	int read;
	int readoverflow;
} netmessage_t;

typedef struct socket_s
{
	int socket;                         //socket number
	sockaddr_t addr;                    //socket address
	netmessage_t msg;                   //current message being read
	int remaining;                      //remaining bytes to read for the current message
	struct socket_s *prev, *next;   //prev and next socket in a list
} socket_t;

//compare addresses
int Net_AddressCompare( address_t *addr1, address_t *addr2 );
//gives the address of a socket
void Net_SocketToAddress( socket_t *sock, address_t *address );
//converts a string to an address
void Net_StringToAddress( char *string, address_t *address );
//set the address ip port
void Net_SetAddressPort( address_t *address, int port );
//send a message to the given socket
int Net_Send( socket_t *sock, netmessage_t *msg );
//recieve a message from the given socket
int Net_Receive( socket_t *sock, netmessage_t *msg );
//connect to a host
// NOTE: port is the localhost port, usually 0
// ex: Net_Connect( "192.168.0.1:39000", 0 )
socket_t *Net_Connect( address_t *address, int port );
//disconnect from a host
void Net_Disconnect( socket_t *sock );
//returns the local address
void Net_MyAddress( address_t *address );
//listen at the given port
socket_t *Net_ListenSocket( int port );
//accept new connections at the given socket
socket_t *Net_Accept( socket_t *sock );
//setup networking
int Net_Setup( void );
//shutdown networking
void Net_Shutdown( void );
//message handling
void  NMSG_Clear( netmessage_t *msg );
void  NMSG_WriteChar( netmessage_t *msg, int c );
void  NMSG_WriteByte( netmessage_t *msg, int c );
void  NMSG_WriteShort( netmessage_t *msg, int c );
void  NMSG_WriteLong( netmessage_t *msg, int c );
void  NMSG_WriteFloat( netmessage_t *msg, float c );
void  NMSG_WriteString( netmessage_t *msg, char *string );
void  NMSG_ReadStart( netmessage_t *msg );
int   NMSG_ReadChar( netmessage_t *msg );
int   NMSG_ReadByte( netmessage_t *msg );
int   NMSG_ReadShort( netmessage_t *msg );
int   NMSG_ReadLong( netmessage_t *msg );
float NMSG_ReadFloat( netmessage_t *msg );
char *NMSG_ReadString( netmessage_t *msg );

//++timo FIXME: the WINS_ things are not necessary, they can be made portable arther easily
char *WINS_ErrorMessage( int error );

#ifdef __cplusplus
}
#endif
