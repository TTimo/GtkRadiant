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

// cmdlib.h

#ifndef __CMDLIB__
#define __CMDLIB__

#include "bytebool.h"

#ifdef _WIN32
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float

#pragma check_stack(off)

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32

#pragma intrinsic( memset, memcpy )

#endif


#define MAX_OS_PATH     1024
#define MEM_BLOCKSIZE 4096

// the dec offsetof macro doesnt work very well...
#define myoffsetof( type,identifier ) ( (size_t)& ( (type *)0 )->identifier )

#define SAFE_MALLOC
#ifdef SAFE_MALLOC
void *safe_malloc( size_t size );
void *safe_malloc_info( size_t size, char* info );
#else
#define safe_malloc( a ) malloc( a )
#endif /* SAFE_MALLOC */

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

char *strlower( char *in );
int Q_strncasecmp( const char *s1, const char *s2, int n );
int Q_stricmp( const char *s1, const char *s2 );
void Q_getwd( char *out );

int Q_filelength( FILE *f );
int FileTime( const char *path );

void    Q_mkdir( const char *path );

extern char qdir[1024];
extern char gamedir[1024];
extern char writedir[1024];
extern char    *moddirparam;
void SetQdirFromPath( const char *path );
char *ExpandArg( const char *path );    // from cmd line
char *ExpandPath( const char *path );   // from scripts
char *ExpandGamePath( const char *path );
char *ExpandPathAndArchive( const char *path );
void ExpandWildcards( int *argc, char ***argv );


double I_FloatTime( void );

void    Error( const char *error, ... );
int     CheckParm( const char *check );

FILE    *SafeOpenWrite( const char *filename );
FILE    *SafeOpenRead( const char *filename );
void    SafeRead( FILE *f, void *buffer, int count );
void    SafeWrite( FILE *f, const void *buffer, int count );

int     LoadFile( const char *filename, void **bufferptr );
int   LoadFileBlock( const char *filename, void **bufferptr );
int     TryLoadFile( const char *filename, void **bufferptr );
void    SaveFile( const char *filename, const void *buffer, int count );
qboolean    FileExists( const char *filename );

void    DefaultExtension( char *path, const char *extension );
void    DefaultPath( char *path, const char *basepath );
void    StripFilename( char *path );
void    StripExtension( char *path );

void    ExtractFilePath( const char *path, char *dest );
void    ExtractFileBase( const char *path, char *dest );
void    ExtractFileExtension( const char *path, char *dest );

int     ParseNum( const char *str );

short   BigShort( short l );
short   LittleShort( short l );
int     BigLong( int l );
int     LittleLong( int l );
float   BigFloat( float l );
float   LittleFloat( float l );


char *COM_Parse( char *data );

extern char com_token[1024];
extern qboolean com_eof;

char *copystring( const char *s );


void CRC_Init( unsigned short *crcvalue );
void CRC_ProcessByte( unsigned short *crcvalue, byte data );
unsigned short CRC_Value( unsigned short crcvalue );

void    CreatePath( const char *path );
void    QCopyFile( const char *from, const char *to );

extern qboolean archive;
extern char archivedir[1024];

// sleep for the given amount of milliseconds
void Sys_Sleep( int n );

// for compression routines
typedef struct
{
	void    *data;
	int count, width, height;
} cblock_t;


#endif
