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
// - Unix-style slashes (/) (windows is backwards .. everyone knows that)
//
// Leonardo Zide (leo@lokigames.com)
//

#include <glib.h>
#include <stdio.h>
#include <errno.h>

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <dirent.h>
  #include <unistd.h>
#else
  #include <wtypes.h>
  #include <io.h>
  #define R_OK 04
  #define S_ISDIR( mode ) ( mode & _S_IFDIR )
#endif

// TTimo: String functions
//   see http://www.qeradiant.com/faq/index.cgi?file=175
#include "str.h"

#include <stdlib.h>
#include <sys/stat.h>

#include "vfspk3.h"
#include "vfs.h"
#include "unzip-vfspk3.h"

typedef struct
{
	char*   name;
	unz_s zipinfo;
	unzFile zipfile;
	guint32 size;
} VFS_PAKFILE;

// =============================================================================
// Global variables

static GSList* g_unzFiles;
static GSList* g_pakFiles;
static char g_strDirs[VFS_MAXDIRS][PATH_MAX];
static int g_numDirs;
static bool g_bUsePak = true;

// suported pak extension list
const char* pak_ext_list[4] = { ".pk3", ".pk4", ".dpk", NULL };

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
	unz_global_info gi;
	unzFile uf;
	guint32 i;
	int err;

	errno = 0;
	uf = unzOpen( filename );
	if ( uf == NULL ) {
		g_FuncTable.m_pfnSysFPrintf( SYS_WRN, "  failed to init pak file %s. %s\n", filename, ( errno !=0 ? strerror( errno ): "" ) );
		return;
	}
	g_FuncTable.m_pfnSysPrintf( "  pak file: %s\n", filename );

	g_unzFiles = g_slist_append( g_unzFiles, uf );

	err = unzGetGlobalInfo( uf,&gi );
	if ( err != UNZ_OK ) {
		return;
	}
	unzGoToFirstFile( uf );

	for ( i = 0; i < gi.number_entry; i++ )
	{
		char filename_inzip[NAME_MAX];
		unz_file_info file_info;
		VFS_PAKFILE* file;

		err = unzGetCurrentFileInfo( uf, &file_info, filename_inzip, sizeof( filename_inzip ), NULL, 0, NULL, 0 );
		if ( err != UNZ_OK ) {
			break;
		}

		file = (VFS_PAKFILE*)g_malloc( sizeof( VFS_PAKFILE ) );
		g_pakFiles = g_slist_append( g_pakFiles, file );

		vfsFixDOSName( filename_inzip );
		strlwr( filename_inzip );

		file->name = g_strdup( filename_inzip );
		file->size = file_info.uncompressed_size;
		file->zipfile = uf;
		memcpy( &file->zipinfo, uf, sizeof( unz_s ) );

		if ( ( i + 1 ) < gi.number_entry ) {
			err = unzGoToNextFile( uf );
			if ( err != UNZ_OK ) {
				break;
			}
		}
	}
}

static GSList* vfsGetListInternal( const char *refdir, const char *ext, bool directories ){
	GSList *lst, *lst_aux, *files = NULL;
	char dirname[NAME_MAX], extension[NAME_MAX], filename[NAME_MAX];
	char basedir[NAME_MAX];
	int dirlen;
	char *ptr;
	char *dirlist;
	struct stat st;
	GDir *diskdir;
	int i;

	if ( refdir != NULL ) {
		strcpy( dirname, refdir );
		strlwr( dirname );
		vfsFixDOSName( dirname );
		vfsAddSlash( dirname );
	}
	else{
		dirname[0] = '\0';
	}
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
		ptr = file->name;

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
			char *ptr_ext = strrchr( ptr, '.' );
			if ( ( ext != NULL ) && ( ( ptr_ext == NULL ) || ( strcmp( ptr_ext + 1, extension ) != 0 ) ) ) {
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
		strcpy( basedir, g_strDirs[i] );
		strcat( basedir, dirname );

		diskdir = g_dir_open( basedir, 0, NULL );

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

				sprintf( filename, "%s%s", basedir, name );
				stat( filename, &st );

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

// reads all pak files from a dir
/*!
   The gamemode hacks in here will do undefined things with files called zz_*.
   This is simple to fix by cleaning up the hacks, but may be better left alone
   if the engine code does the same thing.
 */
void vfsInitDirectory( const char *path ){
	char filename[PATH_MAX];
	const char* pakdir_suf = "dir";
	GDir *dir;
	GSList *dirlist = NULL;
	int iGameMode; // 0: no filtering 1: SP 2: MP

	if ( g_numDirs == ( VFS_MAXDIRS - 1 ) ) {
		return;
	}

	// See if we are in "sp" or "mp" mapping mode
	const char* gamemode = g_FuncTable.m_pfnReadProjectKey( "gamemode" );

	if ( gamemode ) {
		if ( strcmp( gamemode, "sp" ) == 0 ) {
			iGameMode = 1;
		}
		else if ( strcmp( gamemode, "mp" ) == 0 ) {
			iGameMode = 2;
		}
		else{
			iGameMode = 0;
		}
	}
	else{
		iGameMode = 0;
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

				char *ext = (char*)strrchr( name, '.' );
				if ( ext == NULL ) {
					continue;
				}

				gboolean is_pak = FALSE;

				for ( int i = 0; pak_ext_list[i] != NULL ; i++ ) {
					const char* cur_ext = pak_ext_list[i];
					if ( strcasecmp( ext, cur_ext ) == 0 ) {
						is_pak = TRUE;
					}
					cur_ext = g_strconcat(cur_ext, pakdir_suf, NULL);
					if ( strcasecmp( ext, cur_ext ) == 0 ) {
						is_pak = TRUE;
					}
				}

				if ( !is_pak ) {
					continue;
				}

				char* direntry = g_strdup( name );

				// using the same kludge as in engine to ensure consistency
				switch ( iGameMode )
				{
				case 1: // SP
					if ( strncmp( direntry,"sp_",3 ) == 0 ) {
						memcpy( direntry,"zz",2 );
					}
					break;
				case 2: // MP
					if ( strncmp( direntry,"mp_",3 ) == 0 ) {
						memcpy( direntry,"zz",2 );
					}
					break;
				}

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

				switch ( iGameMode )
				{
				case 1: // SP
					if ( strncmp( name,"mp_",3 ) == 0 ) {
						g_free( name );
						dirlist = g_slist_remove( cur, name );
						continue;
					}
					else if ( strncmp( name,"zz_",3 ) == 0 ) {
						memcpy( name,"sp",2 );
					}
					break;
				case 2: // MP
					if ( strncmp( name,"sp_",3 ) == 0 ) {
						g_free( name );
						dirlist = g_slist_remove( cur, name );
						continue;
					}
					else if ( strncmp( name,"zz_",3 ) == 0 ) {
						memcpy( name,"mp",2 );
					}
					break;
				}

				sprintf( filename, "%s/%s", path, name );
				if ( g_str_has_suffix( name, "dir" ) ) {
					vfsInitDirectory( filename );
				} else {
					vfsInitPakFile( filename );
				}

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
// FIXME TTimo this should be improved so that we can shutdown and restart the VFS without exiting Radiant?
//   (for instance when modifying the project settings)
void vfsShutdown(){
	while ( g_unzFiles )
	{
		unzClose( (unzFile)g_unzFiles->data );
		g_unzFiles = g_slist_remove( g_unzFiles, g_unzFiles->data );
	}

	// avoid dangling pointer operation (makes BC hangry)
	GSList *cur = g_pakFiles;
	GSList *next = cur;
	while ( next )
	{
		cur = next;
		VFS_PAKFILE* file = (VFS_PAKFILE*)cur->data;
		g_free( file->name );
		g_free( file );
		next = g_slist_remove( cur, file );
	}
	g_pakFiles = NULL;
}

void vfsFreeFile( void *p ){
	g_free( p );
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

int vfsGetFileCount( const char *filename, int flag ){
	int i, count = 0;
	char fixed[NAME_MAX], tmp[NAME_MAX];
	GSList *lst;

	strcpy( fixed, filename );
	vfsFixDOSName( fixed );
	strlwr( fixed );

	if ( !flag || ( flag & VFS_SEARCH_PAK ) ) {
		for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
		{
			VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

			if ( strcmp( file->name, fixed ) == 0 ) {
				count++;
			}
		}
	}

	if ( !flag || ( flag & VFS_SEARCH_DIR ) ) {
		for ( i = 0; i < g_numDirs; i++ )
		{
			strcpy( tmp, g_strDirs[i] );
			strcat( tmp, fixed );
			if ( access( tmp, R_OK ) == 0 ) {
				count++;
			}
		}
	}

	return count;
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
				return vfsLoadFullPathFile( tmp,bufferptr );
			}

			count++;
		}
	}

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

		if ( strcmp( file->name, fixed ) != 0 ) {
			continue;
		}

		if ( count == index ) {
			memcpy( file->zipfile, &file->zipinfo, sizeof( unz_s ) );

			if ( unzOpenCurrentFile( file->zipfile ) != UNZ_OK ) {
				return -1;
			}

			*bufferptr = g_malloc( file->size + 1 );
			// we need to end the buffer with a 0
			( (char*) ( *bufferptr ) )[file->size] = 0;

			i = unzReadCurrentFile( file->zipfile, *bufferptr, file->size );
			unzCloseCurrentFile( file->zipfile );
			if ( i > 0 ) {
				return file->size;
			}
			else{
				return -1;
			}
		}

		count++;
	}

	return -1;
}

//#ifdef _DEBUG
#if 1
  #define DBG_RLTPATH
#endif

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

void vfsCleanFileName( char *in ){
	strlwr( in );
	vfsFixDOSName( in );
	int n = strlen( in );
	if ( in[n - 1] == '/' ) {
		in[n - 1] = '\0';
	}
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


// TODO TTimo on linux the base prompt is ~/.q3a/<fs_game>
// given the file dialog, we could push the strFSBasePath and ~/.q3a into the directory shortcuts
// FIXME TTimo is this really a VFS functionality?
//   actually .. this should be the decision of the core isn't it?
//   or .. add an API so that the base prompt can be set during VFS init
const char* vfsBasePromptPath(){
#ifdef _WIN32
	static const char* path = "C:";
#else
	static const char* path = "/";
#endif
	return path;
}
