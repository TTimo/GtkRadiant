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
// Missing functions
//
// Leonardo Zide (leo@lokigames.com)
//

#include "missing.h"
#include "qsysprintf.h"
#include "qe3.h"

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>

bool radCopyFile( const char *lpExistingFileName, const char *lpNewFileName, bool fatal_on_error ){
	FILE *src, *dst;
	void* buf;
	int l;
	bool ret = false;
	char realsrc[PATH_MAX], realdest[PATH_MAX];

	realpath( lpExistingFileName, realsrc );
	realpath( lpNewFileName, realdest );

	src = fopen( realsrc, "rb" );
	if ( !src ) {
      if ( fatal_on_error ) {
        Error( "Failed to open source for copy: %s\n", realsrc );
      }
      Sys_Printf( "Failed to open source for copy: %s\n", realsrc );      
      return false;
	}
	dst = fopen( realdest, "wb" );
	if ( !dst ) {
      if ( fatal_on_error ) {
        Error( "Failed to open destination for copy: %s\n", realdest );
      }
      Sys_Printf( "Failed to open destination for copy: %s\n", realdest );
      fclose( src );
      return false;
	}

	fseek( src, 0, SEEK_END );
	l = ftell( src );
	rewind( src );
	buf = g_malloc( l );

	if ( buf != NULL ) {
		if ( fread( buf, l, 1, src ) == 1 ) {
			if ( fwrite( buf, l, 1, dst ) == 1 ) {
				ret = true;
			}
		}
	}
	if ( !ret ) {
      if ( fatal_on_error ) {
        Error( "short read or short write while copying %s to %s\n", realsrc, realdest );
      }
      Sys_Printf( "short read or short write while copying %s to %s\n", realsrc, realdest );
    }

	g_free( buf );
	fclose( src );
	fclose( dst );

	return ret;
}

bool radCreateDirectory( const char *directory, bool fatal_on_error ) {
  if ( mkdir( directory, 0777 ) >= 0 ) {
    return true;
  }
  if ( fatal_on_error ) {
    Error( "mkdir %s failed - check your permissions", directory );
  }
  Sys_Printf( "mkdir %s failed - check your permissions\n", directory );
  return false;
}

int GetFullPathName( const char *lpFileName, int nBufferLength, char *lpBuffer, char **lpFilePart ){
	if ( lpFileName[0] == '/' ) {
		strcpy( lpBuffer, lpFileName );
		*lpFilePart = strrchr( lpBuffer, '/' );
		return strlen( lpBuffer );
	}

	if ( getcwd( lpBuffer, nBufferLength ) == NULL ) {
		return 0;
	}

	strcat( lpBuffer, "/" );
	*lpFilePart = lpBuffer + strlen( lpBuffer );
	strcat( lpBuffer, lpFileName );

	char *scr = lpBuffer, *dst = lpBuffer;
	for ( int i = 0; ( i < nBufferLength ) && ( *scr != 0 ); i++ )
	{
		if ( *scr == '/' && *( scr + 1 ) == '.' && *( scr + 2 ) == '.' ) {
			scr += 4;
			while ( dst != lpBuffer && *dst != '/' )
			{
				dst--;
				i--;
			}
		}

		*dst = *scr;

		scr++; dst++;
	}
	*dst = 0;

	return strlen( lpBuffer );
}

EPathCheck CheckFile( const char *path ) {
	struct stat sbuf;
	if ( stat( path, &sbuf ) == -1 ) {
		return PATH_FAIL;
	}
	if ( S_ISDIR( sbuf.st_mode ) ) {
		return PATH_DIRECTORY;
	}
	return PATH_FILE;
}

FindFiles::FindFiles( const char *_directory ) {
	findHandle = opendir( _directory );
}

FindFiles::~FindFiles() {
	if ( findHandle != NULL ) {
		closedir( findHandle );
		findHandle = NULL;
	}
}

const char* FindFiles::NextFile() {
	struct dirent *d;

	if ( findHandle == NULL ) {
		return NULL;
	}

	d = readdir( findHandle );
	if ( d ) {
		return d->d_name;
	}
	return NULL;
}

#else
// WIN32

FindFiles::FindFiles( const char *_directory ) {
	char endChar;
	directory = _directory;
	if ( directory.GetLength() > 0 ) {
		endChar = directory.GetAt( directory.GetLength() - 1 );
		if ( !( endChar == '/' || endChar == '\\' ) ) {
			// We're only using '/' as the path separator throughout this code, not '\'.
			// However, I'd hate to see the code silently fail due to a trailing '\', so
			// I added the check for it.
			directory += '/';
		}
	}
	directory += '*';
	findHandle = INVALID_HANDLE_VALUE;
}

FindFiles::~FindFiles() {
	if ( findHandle != INVALID_HANDLE_VALUE ) {
		FindClose( findHandle );
		findHandle = INVALID_HANDLE_VALUE;
	}
}

const char* FindFiles::NextFile() {
	if ( findHandle == INVALID_HANDLE_VALUE ) {
		findHandle = FindFirstFile( directory.GetBuffer(), &findFileData );
		if ( findHandle == INVALID_HANDLE_VALUE ) {
			return NULL;
		}
		return findFileData.cFileName;
	}
	if ( FindNextFile( findHandle, &findFileData ) == 0 ) {
		FindClose( findHandle );
		findHandle = INVALID_HANDLE_VALUE;
		return NULL;
	}
	return findFileData.cFileName;
}

EPathCheck CheckFile( const char *path ) {
  DWORD Attrib = GetFileAttributes( path );
  if ( Attrib == INVALID_FILE_ATTRIBUTES ) {
    return PATH_FAIL;
  }
  if ( Attrib & FILE_ATTRIBUTE_DIRECTORY ) {
    return PATH_DIRECTORY;
  }
  return PATH_FILE;
}

bool radCreateDirectory( const char *directory, bool fatal_on_error ) {
  if ( CreateDirectory( directory, NULL ) ) {
    return true;
  }
  if ( fatal_on_error ) {
    Error( "mkdir %s failed - check your permissions", directory );
  }
  Sys_Printf( "mkdir %s failed - check your permissions\n", directory );
  return false;
}

bool radCopyFile( const char *lpExistingFileName, const char *lpNewFileName, bool fatal_on_error ) {
  if ( CopyFile( lpExistingFileName, lpNewFileName, FALSE ) ) {
    return true;
  }
  if ( fatal_on_error ) {
    Error( "copy %s %s failed - check your permissions", lpExistingFileName, lpNewFileName );
  }
  Sys_Printf( "copy %s %s failed - check your permissions\n", lpExistingFileName, lpNewFileName );
  return false;
}

#endif

bool radCopyTree( const char *source, const char *dest, bool fatal_on_error ) {
	Str srcEntry;
	Str dstEntry;
	const char      *dirname;
	FindFiles fileScan( source );

	while ( ( dirname = fileScan.NextFile() ) != NULL ) {
		if ( strcmp( dirname, "." ) == 0 || strcmp( dirname, ".." ) == 0 ) {
			continue;
		}
		if ( strcmp( dirname, ".svn" ) == 0 ) {
			continue;
		}
		srcEntry = source;
		srcEntry += "/";
		srcEntry += dirname;
		dstEntry = dest;
		dstEntry += "/";
		dstEntry += dirname;
		switch ( CheckFile( srcEntry.GetBuffer() ) ) {
        case PATH_FAIL: {
          if ( fatal_on_error ) {
            Error( "%s does not exist. check your permissions", srcEntry.GetBuffer() );
          }
        }
		case PATH_DIRECTORY: {
			if ( CheckFile( dstEntry.GetBuffer() ) == PATH_FAIL ) {
              Sys_Printf( "Create directory %s\n", dstEntry.GetBuffer() );
              if ( !radCreateDirectory( dstEntry.GetBuffer(), fatal_on_error ) ) {
					return false;
				}
			}
			bool ret;
			ret = radCopyTree( srcEntry.GetBuffer(), dstEntry.GetBuffer(), fatal_on_error );
			if ( !ret ) {
				return false;
			}
			break;
		}
		case PATH_FILE: {
			Sys_Printf( "Copy %s -> %s\n", srcEntry.GetBuffer(), dstEntry.GetBuffer() );
			bool ret = radCopyFile( srcEntry.GetBuffer(), dstEntry.GetBuffer(), fatal_on_error );
			if ( !ret ) {
				return false;
			}
			break;
		}
		}
	}
	return true;
}
