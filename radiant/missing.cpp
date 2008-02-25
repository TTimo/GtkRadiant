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

#if defined (__linux__) || defined (__APPLE__)

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include "missing.h"
#include "qsysprintf.h"

bool CopyFile(const char *lpExistingFileName, const char *lpNewFileName)
{
  FILE *src, *dst;
  void* buf;
  int l;
  bool ret = false;
  char realsrc[PATH_MAX], realdest[PATH_MAX];

  realpath (lpExistingFileName, realsrc);
  realpath (lpNewFileName, realdest);

  src = fopen (realsrc, "rb");
  if ( !src ) {
    return false;
  }
  dst = fopen (realdest, "wb");
  if (!dst) {
    fclose (src);
    return false;
  }
 
  fseek (src, 0, SEEK_END);
  l = ftell (src);
  rewind (src);
  buf = g_malloc (l);

  if (buf != NULL)
    if (fread (buf, l, 1, src) == 1)
      if (fwrite (buf, l, 1, dst) == 1)
	ret = true;

  g_free (buf);
  fclose (src);
  fclose (dst);

  return ret;
}

bool CreateDirectory( const char *directory ) {
	if ( mkdir( directory, 0777 ) == -1 ) {
		Sys_Printf( "mkdir %s failed\n", directory );
		return false;
	}
	return true;
}

bool CopyTree( const char *source, const char *dest ) {
	DIR				*dir;
	struct dirent	*dirlist;
	struct stat		sbuf;
	Str				srcEntry;
	Str				dstEntry;

	dir = opendir( source );
	if ( dir != NULL ) {
		while ( ( dirlist = readdir( dir ) ) != NULL ) {
			if ( strcmp( dirlist->d_name, "." ) == 0 || strcmp( dirlist->d_name, ".." ) == 0 ) {
				continue;
			}
			if ( strcmp( dirlist->d_name, ".svn" ) == 0 ) {
				continue;
			}
			srcEntry = source;
			srcEntry += "/";
			srcEntry += dirlist->d_name;
			dstEntry = dest;
			dstEntry += "/";
			dstEntry += dirlist->d_name;
			if ( stat( srcEntry.GetBuffer(), &sbuf ) == -1 ) {
				Sys_Printf( "stat %s failed\n", srcEntry.GetBuffer() );
			}
			if ( S_ISDIR( sbuf.st_mode ) ) {
				bool ret;
				if ( stat( dstEntry.GetBuffer(), &sbuf ) == -1 ) {
					ret = CreateDirectory( dstEntry.GetBuffer() );
				}
				ret = CopyTree( srcEntry.GetBuffer(), dstEntry.GetBuffer() );
				if ( !ret ) {
					return false;
				}
			} else {
				Sys_Printf( "copy %s -> %s\n", srcEntry.GetBuffer(), dstEntry.GetBuffer() );
				bool ret = CopyFile( srcEntry.GetBuffer(), dstEntry.GetBuffer() );
				if ( !ret ) {
					return false;
				}
			}
		}
		closedir( dir );
	}
	return true;
}

int GetFullPathName(const char *lpFileName, int nBufferLength, char *lpBuffer, char **lpFilePart)
{
  if (lpFileName[0] == '/')
  {
    strcpy (lpBuffer, lpFileName);
    *lpFilePart = strrchr (lpBuffer, '/');
    return strlen (lpBuffer);
  }

  if (getcwd (lpBuffer, nBufferLength) == NULL)
    return 0;

  strcat (lpBuffer, "/");
  *lpFilePart = lpBuffer + strlen (lpBuffer);
  strcat (lpBuffer, lpFileName);

  char *scr = lpBuffer, *dst = lpBuffer;
  for (int i = 0; (i < nBufferLength) && (*scr != 0); i++)
  {
    if (*scr == '/' && *(scr+1) == '.' && *(scr+2) == '.')
    {
      scr += 4;
      while (dst != lpBuffer && *dst != '/')
      {
	dst--;
	i--;
      }
    }

    *dst = *scr;

    scr++; dst++;
  }
  *dst = 0;

  return strlen (lpBuffer);
}

#endif
