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

#ifndef _VFS_H_
#define _VFS_H_

#define VFS_MAXDIRS 64

void vfsInitDirectory( const char *path );
void vfsShutdown();
void vfsFreeFile( void *p );
GSList* vfsGetFileList( const char *dir, const char *ext );
GSList* vfsGetDirList( const char *dir );
void vfsClearFileDirList( GSList **lst );
int vfsGetFileCount( const char *filename, int flag );
int vfsLoadFile( const char *filename, void **buffer, int index = 0 );
int vfsLoadFullPathFile( const char *filename, void **buffer );

// some useful functions
// clean a file name to a unique representation
// very usefull if you have to do some weird manips on the files
// works on regular files and dirs
// will convert to lowercase, unix path ('/' filename seperator)
// on win32, will build the short path name
// directories will be cleaned, no ending filename seperator
// we modify the entry directly, the size of the string can only go down
void vfsCleanFileName( char * );
// these return a static char*, doesn't need to be freed or anything
// get the base path to use when raising file dialogs
// we manually add "maps/" or "sounds/" or "mapobjects/models/" etc.
const char* vfsBasePromptPath();
// extract the relative path from a full path
// will match against any of the base paths we have
// returns NULL if not found
char* vfsExtractRelativePath( const char *in );
// returns the full path (in a static buff) to a file given it's relative path
// returns the first file in the list or NULL if not found
// see ifilesystem.h for more notes
char* vfsGetFullPath( const char*, int index = 0, int flag = 0 );

// suported pak extension list
extern const char* pak_ext_list[];

#endif // _VFS_H_
