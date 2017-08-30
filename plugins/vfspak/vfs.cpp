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

//
// Rules:
//
// - Directories should be searched in the following order: ~/.q3a/baseq3,
//   install dir (/usr/local/games/quake3/baseq3) and cd_path (/mnt/cdrom/baseq3).
//
// - Pak files are searched first inside the directories.
// - Case insensitive.
// - Unix-style slashes (/)
//
// Leonardo Zide (leo@lokigames.com)
//

#include <glib.h>
#include <stdio.h>
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	#include <dirent.h>
	#include <unistd.h>
	#define WINAPI
#else
	#include <wtypes.h>
	#include <io.h>
	#define R_OK 04
	#define S_ISDIR( mode ) ( mode & _S_IFDIR )
#endif

#include "str.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "vfs.h"
#include "vfspak.h"

typedef struct
{
	char magic[4];       // Name of the new WAD format ("PACK")
	gint32 diroffset;    // Position of WAD directory from start of file
	gint32 dirsize;      // Number of entries * 0x40 (64 char)
} pakheader_t;

typedef struct
{
	char filename[0x38]; // Name of the file, Unix style, with extension, 50 chars, padded with '\0'.
	gint32 offset;       // Position of the entry in PACK file
	gint32 size;         // Size of the entry in PACK file
} pakentry_t;

typedef struct
{
	char*   name;
	pakentry_t entry;
	FILE *pak;
} VFS_PAKFILE;

// =============================================================================
// Global variables

static GSList* g_unzFiles;
static GSList* g_pakFiles;
static char g_strDirs[VFS_MAXDIRS][PATH_MAX];
static int g_numDirs;
static bool g_bUsePak = true;

// =============================================================================
// Static functions

static void vfsAddSlash( char *str ){
	int n = strlen( str );
	if ( n > 0 ) {
		if ( str[n - 1] != '\\' && str[n - 1] != '/' ) {
			strcat( str, "/" );
		}
	}
}

static void vfsFixDOSName( char *src ){
	if ( src == NULL ) {
		return;
	}

	while ( *src )
	{
		if ( *src == '\\' ) {
			*src = '/';
		}
		src++;
	}
}

static void vfsInitPakFile( const char *filename ){
	pakheader_t header;
	FILE *f;
	long i;

	f = fopen( filename, "rb" );
	if ( f == NULL ) {
		return;
	}

	// read header
	fread( header.magic, 1, 4, f );
	fread( &header.diroffset, 1, 4, f );
	fread( &header.dirsize, 1, 4, f );

	// fix endianess
	header.diroffset = GINT32_FROM_LE( header.diroffset );
	header.dirsize = GINT32_FROM_LE( header.dirsize );

	// check that the magic header
	if ( strncmp( header.magic, "PACK", 4 ) ) {
		fclose( f );
		return;
	}

	g_FuncTable.m_pfnSysPrintf( "  pak file: %s\n", filename );

	g_unzFiles = g_slist_append( g_unzFiles, f );
	fseek( f, header.diroffset, SEEK_SET );

	for ( i = 0; i < (long)( header.dirsize / sizeof( pakentry_t ) ); i++ )
	{
		VFS_PAKFILE* file;

		file = (VFS_PAKFILE*)g_malloc( sizeof( VFS_PAKFILE ) );
		g_pakFiles = g_slist_append( g_pakFiles, file );

		fread( file->entry.filename, 1, sizeof( file->entry.filename ), f );
		fread( &file->entry.offset, 1, sizeof( file->entry.offset ), f );
		fread( &file->entry.size, 1, sizeof( file->entry.size ), f );
		file->pak = f;

		// fix endianess
		file->entry.offset = GINT32_FROM_LE( file->entry.offset );
		file->entry.size = GINT32_FROM_LE( file->entry.size );

		// fix filename
		vfsFixDOSName( file->entry.filename );
		strlwr( file->entry.filename );
		//g_FuncTable.m_pfnSysPrintf("vfs file from pak: %s\n", file->entry.filename);
	}
}

static GSList* vfsGetListInternal( const char *dir, const char *ext, bool directories ){
	GSList *lst, *lst_aux, *files = NULL;
	char dirname[NAME_MAX], extension[NAME_MAX], filename[NAME_MAX];
	int dirlen;
	char *ptr;
	//struct dirent *dirlist;
	char *dirlist;
	struct stat st;
	GDir *diskdir;
	int i;

	dirname[0] = '\0';
	if ( dir != NULL ) {
		strcat( dirname, dir );
		strlwr( dirname );
		vfsFixDOSName( dirname );
		vfsAddSlash( dirname );
		Sys_Printf( "vfs dirname_1: %s\n", dirname );
	}
	//else
	//  dirname[0] = '\0';
	dirlen = strlen( dirname );

	if ( ext != NULL ) {
		strcpy( extension, ext );
	}
	else{
		extension[0] = '\0';
	}
	strlwr( extension );

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;
		gboolean found = FALSE;
		ptr = file->entry.filename;

		// check that the file name begins with dirname
		for ( i = 0; ( *ptr && i < dirlen ); i++, ptr++ )
			if ( *ptr != dirname[i] ) {
				break;
			}

		if ( i != dirlen ) {
			continue;
		}

		if ( directories ) {
			char *sep = strchr( ptr, '/' );
			if ( sep == NULL ) {
				continue;
			}

			i = sep - ptr;

			// check for duplicates
			for ( lst_aux = files; lst_aux; lst_aux = g_slist_next( lst_aux ) )
				if ( strncmp( (char*)lst_aux->data, ptr, i ) == 0 ) {
					found = TRUE;
					break;
				}

			if ( !found ) {
				char *name = g_strndup( ptr, i + 1 );
				name[i] = '\0';
				files = g_slist_append( files, name );
			}
		}
		else
		{
			// check extension
			if ( ( ext != NULL ) && ( strstr( ptr, extension ) == NULL ) ) {
				continue;
			}

			// check for duplicates
			for ( lst_aux = files; lst_aux; lst_aux = g_slist_next( lst_aux ) )
				if ( strcmp( (char*)lst_aux->data, ptr ) == 0 ) {
					found = TRUE;
					break;
				}

			if ( !found ) {
				files = g_slist_append( files, g_strdup( ptr ) );
			}
		}
	}

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( dirname, g_strDirs[i] );
		strcat( dirname, dir );
		strlwr( dirname );
		vfsFixDOSName( dirname );
		vfsAddSlash( dirname );

		diskdir = g_dir_open( dirname, 0, NULL );

		if ( diskdir != NULL ) {
			while ( 1 )
			{
				const char* name = g_dir_read_name( diskdir );
				if ( name == NULL ) {
					break;
				}

				if ( directories && ( name[0] == '.' ) ) {
					continue;
				}

				sprintf( filename, "%s%s", dirname, name );
				stat( filename, &st );
				Sys_Printf( "vfs FileName: %s\n", filename );

				if ( ( S_ISDIR( st.st_mode ) != 0 ) != directories ) {
					continue;
				}

				gboolean found = FALSE;

				dirlist = g_strdup( name );

				strlwr( dirlist );

				char *ptr_ext = strrchr( dirlist, '.' );
				if ( ext == NULL
					 || ( ext != NULL && ptr_ext != NULL && ptr_ext[0] != '\0' && strcmp( ptr_ext + 1, extension ) == 0 ) ) {

					// check for duplicates
					for ( lst_aux = files; lst_aux; lst_aux = g_slist_next( lst_aux ) )
						if ( strcmp( (char*)lst_aux->data, dirlist ) == 0 ) {
							found = TRUE;
							break;
						}

					if ( !found ) {
						files = g_slist_append( files, g_strdup( dirlist ) );
					}
				}

				g_free( dirlist );
			}
			g_dir_close( diskdir );
		}
	}

	return files;
}

/*!
   This behaves identically to -stricmp(a,b), except that ASCII chars
   [\]^`_ come AFTER alphabet chars instead of before. This is because
   it effectively converts all alphabet chars to uppercase before comparison,
   while stricmp converts them to lowercase.
 */
//!\todo Analyse the code in rtcw/q3 to see how it behaves.
static int vfsPakSort( const void *a, const void *b ){
	char    *s1, *s2;
	int c1, c2;

	s1 = (char*)a;
	s2 = (char*)b;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 >= 'a' && c1 <= 'z' ) {
			c1 -= ( 'a' - 'A' );
		}
		if ( c2 >= 'a' && c2 <= 'z' ) {
			c2 -= ( 'a' - 'A' );
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		// Arnout: note - sort pakfiles in reverse order. This ensures that
		// later pakfiles override earlier ones. This because the vfs module
		// returns a filehandle to the first file it can find (while it should
		// return the filehandle to the file in the most overriding pakfile, the
		// last one in the list that is).
		if ( c1 < c2 ) {
			//return -1;		// strings not equal
			return 1;       // strings not equal
		}
		if ( c1 > c2 ) {
			//return 1;
			return -1;
		}
	} while ( c1 );

	return 0;       // strings are equal
}

// =============================================================================
// Global functions

void vfsInitDirectory( const char *path ){
	char filename[PATH_MAX];
	//struct dirent *direntry;
	GDir *dir;
	GSList *dirlist = NULL;

	if ( g_numDirs == ( VFS_MAXDIRS - 1 ) ) {
		return;
	}

	strcpy( g_strDirs[g_numDirs], path );
	vfsFixDOSName( g_strDirs[g_numDirs] );
	vfsAddSlash( g_strDirs[g_numDirs] );
	g_numDirs++;

	if ( g_bUsePak ) {
		dir = g_dir_open( path, 0, NULL );
		if ( dir != NULL ) {
			g_FuncTable.m_pfnSysPrintf( "vfs directory: %s\n", path );

			for (;; )
			{
				const char* name = g_dir_read_name( dir );
				if ( name == NULL ) {
					break;
				}

				const char *ext = strrchr( name, '.' );
				if ( ( ext == NULL ) || ( strcasecmp( ext, ".pak" ) != 0 ) ) {
					continue;
				}

				char* direntry = g_strdup( name );
				dirlist = g_slist_append( dirlist, direntry );
			}

			g_dir_close( dir );


			// sort them
			dirlist = g_slist_sort( dirlist, vfsPakSort );

			// add the entries to the vfs and free the list
			while ( dirlist )
			{
				GSList *cur = dirlist;
				char* name = (char*)cur->data;

				sprintf( filename, "%s/%s", path, name );
				vfsInitPakFile( filename );

				g_free( name );
				dirlist = g_slist_remove( cur, name );
			}
		}
		else{
			g_FuncTable.m_pfnSysFPrintf( SYS_WRN, "vfs directory not found: %s\n", path );
		}

	}
}


// frees all memory that we allocated
void vfsShutdown(){
	while ( g_unzFiles )
	{
		fclose( (FILE*)g_unzFiles->data );
		g_unzFiles = g_slist_remove( g_unzFiles, g_unzFiles->data );
	}

	while ( g_pakFiles )
	{
		g_free( g_pakFiles->data );
		g_pakFiles = g_slist_remove( g_pakFiles, g_pakFiles->data );
	}
}

GSList* vfsGetFileList( const char *dir, const char *ext ){
	return vfsGetListInternal( dir, ext, false );
}

GSList* vfsGetDirList( const char *dir ){
	return vfsGetListInternal( dir, NULL, true );
}

void vfsClearFileDirList( GSList **lst ){
	while ( *lst )
	{
		g_free( ( *lst )->data );
		*lst = g_slist_remove( *lst, ( *lst )->data );
	}
}

// return the number of files that match
int vfsGetFileCount( const char *filename, int flag ){
	int i, count = 0;
	char fixed[NAME_MAX], tmp[NAME_MAX];
	GSList *lst;

	strcpy( fixed, filename );
	vfsFixDOSName( fixed );
	strlwr( fixed );

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

		if ( strcmp( file->entry.filename, fixed ) == 0 ) {
			count++;
		}
	}

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( tmp, g_strDirs[i] );
		strcat( tmp, fixed );
		if ( access( tmp, R_OK ) == 0 ) {
			count++;
		}
	}

	return count;
}

// NOTE: when loading a file, you have to allocate one extra byte and set it to \0
int vfsLoadFile( const char *filename, void **bufferptr, int index ){
	int i, count = 0;
	char tmp[NAME_MAX], fixed[NAME_MAX];
	GSList *lst;

	*bufferptr = NULL;
	strcpy( fixed, filename );
	vfsFixDOSName( fixed );
	strlwr( fixed );

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( tmp, g_strDirs[i] );
		strcat( tmp, filename );
		if ( access( tmp, R_OK ) == 0 ) {
			if ( count == index ) {
				long len;
				FILE *f;

				f = fopen( tmp, "rb" );
				if ( f == NULL ) {
					return -1;
				}

				fseek( f, 0, SEEK_END );
				len = ftell( f );
				rewind( f );

				*bufferptr = g_malloc( len + 1 );
				if ( *bufferptr == NULL ) {
					return -1;
				}

				fread( *bufferptr, 1, len, f );
				fclose( f );

				// we need to end the buffer with a 0
				( (char*) ( *bufferptr ) )[len] = 0;

				return len;
			}

			count++;
		}
	}

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

		if ( strcmp( file->entry.filename, fixed ) != 0 ) {
			continue;
		}

		if ( count == index ) {
			fseek( file->pak, file->entry.offset, SEEK_SET );

			*bufferptr = g_malloc( file->entry.size + 1 );
			// we need to end the buffer with a 0
			( (char*) ( *bufferptr ) )[file->entry.size] = 0;

			return fread( *bufferptr, 1, file->entry.size, file->pak );
		}

		count++;
	}

	return -1;
}

void vfsFreeFile( void *p ){
	g_free( p );
}

// open a full path file
int vfsLoadFullPathFile( const char *filename, void **bufferptr ){
	FILE *f;
	long len;

	f = fopen( filename, "rb" );
	if ( f == NULL ) {
		return -1;
	}

	fseek( f, 0, SEEK_END );
	len = ftell( f );
	rewind( f );

	*bufferptr = g_malloc( len + 1 );
	if ( *bufferptr == NULL ) {
		return -1;
	}

	fread( *bufferptr, 1, len, f );
	fclose( f );

	// we need to end the buffer with a 0
	( (char*) ( *bufferptr ) )[len] = 0;

	return len;
}

void vfsCleanFileName( char *in ){
	strlwr( in );
	vfsFixDOSName( in );
	int n = strlen( in );
	if ( in[n - 1] == '/' ) {
		in[n - 1] = '\0';
	}
}

const char* vfsBasePromptPath(){
#ifdef _WIN32
	static const char* path = "C:";
#else
	static const char* path = "/";
#endif
	return path;
}

/*!
   \param shorten will try to match against the short version
   recent switch back to short path names in project settings has broken some stuff
   with shorten == true, we will convert in to short version before looking for root
   FIXME WAAA .. the stuff below is much more simple on linux .. add appropriate #ifdef
 */
char* vfsExtractRelativePath_short( const char *in, bool shorten ){
	int i;
	char l_in[PATH_MAX];
	char check[PATH_MAX];
	static char out[PATH_MAX];
	out[0] = 0;

#ifdef DBG_RLTPATH
	Sys_Printf( "vfsExtractRelativePath: %s\n", in );
#endif

#ifdef _WIN32
	if ( shorten ) {
		// make it short
		if ( GetShortPathName( in, l_in, PATH_MAX ) == 0 ) {
#ifdef DBG_RLTPATH
			Sys_Printf( "GetShortPathName failed\n" );
#endif
			return NULL;
		}
	}
	else
	{
		strcpy( l_in,in );
	}
	vfsCleanFileName( l_in );
#else
	strcpy( l_in, in );
	vfsCleanFileName( l_in );
#endif // ifdef WIN32


#ifdef DBG_RLTPATH
	Sys_Printf( "cleaned path: %s\n", l_in );
#endif

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( check,g_strDirs[i] );
		vfsCleanFileName( check );
#ifdef DBG_RLTPATH
		Sys_Printf( "Matching against %s\n", check );
#endif

		// try to find a match
		if ( strstr( l_in, check ) ) {
			strcpy( out,l_in + strlen( check ) + 1 );
			break;
		}

	}
	if ( out[0] != 0 ) {
#ifdef DBG_RLTPATH
		Sys_Printf( "vfsExtractRelativePath: success\n" );
#endif
		return out;
	}
#ifdef DBG_RLTPATH
	Sys_Printf( "vfsExtractRelativePath: failed\n" );
#endif
	return NULL;
}

// HYDRA: this now searches VFS/PAK files in addition to the filesystem
// if FLAG is unspecified then ONLY dirs are searched.
// PAK's are searched before DIRs to mimic engine behaviour
// index is ignored when searching PAK files.
// see ifilesystem.h
char* vfsGetFullPath( const char *in, int index, int flag ){
	int count = 0;
	static char out[PATH_MAX];
	char tmp[NAME_MAX];
	int i;

	if ( flag & VFS_SEARCH_PAK ) {
		char fixed[NAME_MAX];
		GSList *lst;

		strcpy( fixed, in );
		vfsFixDOSName( fixed );
		strlwr( fixed );

		for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
		{
			VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

			char *ptr,*lastptr;
			lastptr = file->name;

			while ( ( ptr = strchr( lastptr,'/' ) ) != NULL )
				lastptr = ptr + 1;

			if ( strcmp( lastptr, fixed ) == 0 ) {
				strncpy( out,file->name,PATH_MAX );
				return out;
			}
		}

	}

	if ( !flag || ( flag & VFS_SEARCH_DIR ) ) {
		for ( i = 0; i < g_numDirs; i++ )
		{
			strcpy( tmp, g_strDirs[i] );
			strcat( tmp, in );
			if ( access( tmp, R_OK ) == 0 ) {
				if ( count == index ) {
					strcpy( out, tmp );
					return out;
				}
				count++;
			}
		}
	}
	return NULL;
}

// FIXME TTimo: this and the above should be merged at some point
char* vfsExtractRelativePath( const char *in ){
	static char out[PATH_MAX];
	unsigned int i, count;
	char *chunk, *backup = NULL; // those point to out stuff
	char *ret = vfsExtractRelativePath_short( in, false );
	if ( !ret ) {
#ifdef DBG_RLTPATH
		Sys_Printf( "trying with a short version\n" );
#endif
		ret = vfsExtractRelativePath_short( in, true );
		if ( ret ) {
			// ok, but we have a relative short version now
			// hack the long relative version out of here
			count = 0;
			for ( i = 0; i < strlen( ret ); i++ )
			{
				if ( ret[i] == '/' ) {
					count++;
				}
			}
			// this is the clean, not short version
			strcpy( out, in );
			vfsCleanFileName( out );
			for ( i = 0; i <= count; i++ )
			{
				chunk = strrchr( out, '/' );
				if ( backup ) {
					backup[0] = '/';
				}
				chunk[0] = '\0';
				backup = chunk;
			}
			return chunk + 1;
		}
	}
	return ret;
}
