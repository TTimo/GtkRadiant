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

#include "vfswad.h"
#include "vfs.h"
#include "unwad.h"

typedef struct
{
	char*         name;
	WAD3_LUMP wadlump;
	wadFile_t     *wadfile;
	unsigned long filenumber;
	unsigned long size;
} VFS_PAKFILE;

// =============================================================================
// Global variables

static GSList* g_wadFiles;
static GSList* g_pakFiles;
static char g_strDirs[VFS_MAXDIRS][PATH_MAX];
static int g_numDirs;

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

//FIXME: STUPID short filenames.. get RID of it asap
// copied verbatim from qe3.cpp
int vfsBuildShortPathName( const char* pPath, char* pBuffer, int nBufferLen ){
#ifdef _WIN32
	char *pFile = NULL;
	int nResult = GetFullPathName( pPath, nBufferLen, pBuffer, &pFile );
	nResult = GetShortPathName( pPath, pBuffer, nBufferLen );
	if ( nResult == 0 ) {
		strcpy( pBuffer, pPath );               // Use long filename
	}
	return nResult;
#elif defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

	// remove /../ from directories
	const char *scr = pPath; char *dst = pBuffer;
	for ( int i = 0; ( i < nBufferLen ) && ( *scr != 0 ); i++ )
	{
		if ( *scr == '/' && *( scr + 1 ) == '.' && *( scr + 2 ) == '.' ) {
			scr += 3;
			while ( dst != pBuffer && *( --dst ) != '/' )
			{
				i--;
			}
		}

		*dst = *scr;

		scr++; dst++;
	}
	*dst = 0;

	return strlen( pBuffer );
#else
#error "unsupported platform"
#endif
}

void ExtractFileName( const char *path, char *dest ){
	const char *src;

	src = path + strlen( path ) - 1;

//
// back up until a \ or the start
//
	while ( src > path && *( src - 1 ) != '/'
			&& *( src - 1 ) != '\\' )
		src--;

	while ( *src )
	{
		*dest++ = *src++;
	}
	*dest = 0;
}

static void vfsInitPakFile( const char *filename ){
	wadFile_t *wf;
	unsigned int i;
	int err;
	char wadname[NAME_MAX];
	char *ext;

	if ( !filename || strlen( filename ) <= 0 ) {
		g_FuncTable.m_pfnSysFPrintf( SYS_WRN, "  missing wad filename\n" );
		return;
	}
	wf = wadOpen( filename );
	if ( wf == NULL ) {
		g_FuncTable.m_pfnSysFPrintf( SYS_WRN, "  failed to init wad file %s\n", filename );
		return;
	}
	g_FuncTable.m_pfnSysPrintf( "  wad file: %s\n", filename );

	ExtractFileName( filename, wadname );
	strlwr( wadname );

	ext = strrchr( wadname, '.' );
	if ( ext && strcmp( ext, ".wad" ) == 0 ) {
		*ext = 0; // ditch the .wad so everthing looks nice!
	}

	g_wadFiles = g_slist_append( g_wadFiles, wf ); // store the wadfile handle

	wadGoToFirstFile( wf );

	for ( i = 0; i < wf->lpHeader->numlumps; i++ )
	{
		char filename_inwad[NAME_MAX];
		char filename_inwadfixed[NAME_MAX];
		unsigned long filesize;
		VFS_PAKFILE* file;

		err = wadGetCurrentFileInfo( wf, filename_inwad, sizeof( filename_inwad ) - 5, &filesize ); // -5 for extension + null terminator
		if ( err != 1 ) {
			break;
		}

		file = (VFS_PAKFILE*)g_malloc( sizeof( VFS_PAKFILE ) );
		g_pakFiles = g_slist_append( g_pakFiles, file );

		vfsFixDOSName( filename_inwad );
		strlwr( filename_inwad );

		// texturenames in wad files don't have an extensions or paths, so we must add them!
		if ( wf->lpLump->type == WAD2_TYPE_MIP ) {
			sprintf( filename_inwadfixed,"textures/%s/%s.mip",wadname,filename_inwad );
		}
		else {
			sprintf( filename_inwadfixed,"textures/%s/%s.hlw",wadname,filename_inwad );
		}

		//g_FuncTable.m_pfnSysFPrintf(SYS_WRN, "  scanned %s\\%s\n", filename,filename_inwad);

		file->name = g_strdup( filename_inwadfixed );
		file->size = filesize;
		file->filenumber = wf->currentfile;
		file->wadfile = wf;
		memcpy( &file->wadlump, wf->lpLump, sizeof( WAD3_LUMP ) );

		err = wadGoToNextFile( wf );
		if ( err != 1 ) {
			break;
		}
	}
}

static GSList* vfsGetListInternal( const char *refdir, const char *ext, bool directories ){
	GSList *lst, *lst_aux, *files = NULL;
	char dirname[NAME_MAX], extension[NAME_MAX], filename[NAME_MAX];
	char basedir[NAME_MAX];
	int dirlen;
	char *ptr;
	struct stat st;
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

		GDir* dir = g_dir_open( basedir, 0, NULL );

		if ( dir != NULL ) {
			for (;; )
			{
				const char* name = g_dir_read_name( dir );
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

				char* direntry = g_strdup( name );

				strlwr( direntry );

				char *ptr_ext = strrchr( direntry, '.' );

				if ( ext == NULL
					 || ( ext != NULL && ptr_ext != NULL && ptr_ext[0] != '\0' && strcmp( ptr_ext + 1, extension ) == 0 ) ) {

					// check for duplicates
					for ( lst_aux = files; lst_aux; lst_aux = g_slist_next( lst_aux ) )
						if ( strcmp( (char*)lst_aux->data, direntry ) == 0 ) {
							found = TRUE;
							break;
						}

					if ( !found ) {
						files = g_slist_append( files, g_strdup( direntry ) );
					}
				}

				g_free( direntry );
			}
			g_dir_close( dir );
		}
	}

	return files;
}

// =============================================================================
// Global functions

// reads all pak files from a dir
void vfsInitDirectory( const char *path ){
	char filename[PATH_MAX];

	if ( g_numDirs == ( VFS_MAXDIRS - 1 ) ) {
		return;
	}

	strcpy( g_strDirs[g_numDirs], path );
	vfsFixDOSName( g_strDirs[g_numDirs] );
	vfsAddSlash( g_strDirs[g_numDirs] );
	g_numDirs++;

//  if (g_PrefsDlg.m_bPAK)
	// TODO: can't read prefs from a module, bah..
	if ( 1 ) {
		GDir* dir = g_dir_open( path, 0, NULL );
		if ( dir != NULL ) {
			g_FuncTable.m_pfnSysPrintf( "vfs directory: %s\n", path );
			while ( 1 )
			{
				const char* name = g_dir_read_name( dir );
				if ( name == NULL ) {
					break;
				}

				const char *ext = strrchr( name, '.' );
				if ( ( ext == NULL ) || ( strcmp( ext, ".wad" ) != 0 ) ) {
					continue;
				}

				sprintf( filename, "%s/%s", path, name );
				vfsInitPakFile( filename );
			}
			g_dir_close( dir );
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
	while ( g_wadFiles )
	{
		wadCleanup( (wadFile_t *)g_wadFiles->data );
		g_wadFiles = g_slist_remove( g_wadFiles, g_wadFiles->data );
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

// return the number of files that match
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
				/*
				   long len;
				   FILE *f;

				   f = fopen (tmp, "rb");
				   if (f == NULL)
				   return -1;

				   fseek (f, 0, SEEK_END);
				   len = ftell (f);
				   rewind (f);

				   *bufferptr = g_malloc (len+1);
				   if (*bufferptr == NULL)
				   return -1;

				   fread (*bufferptr, 1, len, f);
				   fclose (f);

				   // we need to end the buffer with a 0
				   ((char*) (*bufferptr))[len] = 0;

				   return len;
				 */
			}

			count++;
		}
	}


	// Textures in HalfLife wads don't have paths, but in the list of files
	// we store the actual full paths of the files and what WAD they're in.
	// so what we have to do is strip the paths and just compare filenames.

	// Hydra: well, we did do this, but now we don't, as the map loader now
	// fills in the correct paths for each texture.

	/*
	   char *searchname;
	   char *fixedptr;

	   fixedptr = fixed;

	   for (i = strlen(fixed)-1 ; i >= 0 && fixed[i] != '\\' && fixed[i] != '/' ; i --)
	   fixedptr = (char *)fixed + i;
	 */
	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;


		/*
		   searchname = file->name;
		   for (i = strlen(file->name)-1 ; i >= 0 && file->name[i] != '\\' && file->name[i] != '/' ; i --)
		   searchname = (char *)file->name + i;
		   if (strcmp (searchname, fixedptr) != 0)
		   continue;
		 */

		if ( strcmp( file->name, fixed ) != 0 ) {
			continue;
		}

		if ( count == index ) {
			// Useful for debugging
			//Sys_Printf("VFSWAD: reading from %s\n",file->wadfile->wadfilename);

			if ( wadOpenCurrentFileByNum( file->wadfile, file->filenumber ) != 1 ) {
				return -1;
			}

			*bufferptr = g_malloc( file->size + 1 );
			// we need to end the buffer with a 0
			( (char*) ( *bufferptr ) )[file->size] = 0;

			i = wadReadCurrentFile( file->wadfile, (char *)*bufferptr, file->size );
			wadCloseCurrentFile( file->wadfile );
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
#if 0
  #define DBG_RLTPATH
#endif

char* vfsExtractRelativePath( const char *in ){
	int i;
	char l_in[PATH_MAX];
	char check[PATH_MAX];
	static char out[PATH_MAX];
	out[0] = 0;

#ifdef DBG_RLTPATH
	Sys_Printf( "vfsExtractRelativePath: %s\n", in );
#endif

	strcpy( l_in,in );
	vfsCleanFileName( l_in );

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

// removed CString usage
void vfsCleanFileName( char *in ){
	char str[PATH_MAX];
	vfsBuildShortPathName( in, str, PATH_MAX );
	strlwr( str );
	vfsFixDOSName( str );
	int n = strlen( str );
	if ( str[n - 1] == '/' ) {
		str[n - 1] = '\0';
	}
	strcpy( in, str );
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
