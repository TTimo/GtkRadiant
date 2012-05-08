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

#ifndef _MISSING_H_
#define _MISSING_H_

// NOTE TTimo
//   this goes along with str.h and provides various utility classes
//   and portability defines
//   the file name (missing.h) is a legacy issue, it would be better to clean that up
//   in a central 'portability' lib

#include <glib.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#define R_OK 04

#else // !_WIN32

// LZ: very ugly hacks
inline int GetLastError() { return 0; };

// temp stuff
inline int GetPrivateProfileInt( char* a, char* b, int i, char* c ) { return i; };
int GetFullPathName( const char *lpFileName, int nBufferLength, char *lpBuffer, char **lpFilePart );

#ifndef APIENTRY
#define APIENTRY
#endif

int MemorySize( void *ptr );
#define _msize MemorySize

#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010

#include <dirent.h>
#include <iostream>

#endif

#define CString Str
#include "str.h"

class CPtrArray
{
public:
CPtrArray ()
{ m_ptrs = g_ptr_array_new(); };
virtual ~CPtrArray ()
{ g_ptr_array_free( m_ptrs, TRUE ); };

void* operator[]( int i ) const
{ return g_ptr_array_index( m_ptrs,i ); };
void* GetAt( int i ) const
{ return g_ptr_array_index( m_ptrs,i ); };
int GetSize() const
{ return m_ptrs->len; };
void Add( void* ptr )
{ g_ptr_array_add( m_ptrs, ptr ); };
void RemoveAll()
{ g_ptr_array_set_size( m_ptrs, 0 ); };
void RemoveAt( int index, int count = 1 ){
	if ( ( index < 0 ) || ( count < 0 ) || ( count + index > (int)m_ptrs->len ) ) {
		return;
	}
	for (; count > 0; count-- )
		g_ptr_array_remove_index( m_ptrs, index );
}
void InsertAt( int nStartIndex, CPtrArray* pNewArray ){
	for ( int i = 0; i < pNewArray->GetSize(); i++ )
		InsertAt( nStartIndex + i, pNewArray->GetAt( i ) );
}
void InsertAt( int nIndex, void* newElement, int nCount = 1 ){
	if ( (guint32)nIndex >= m_ptrs->len ) {
		g_ptr_array_set_size( m_ptrs, nIndex + nCount ); // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_ptrs->len;
		g_ptr_array_set_size( m_ptrs, m_ptrs->len + nCount );
		// shift old data up to fill gap
		memmove( &m_ptrs->pdata[nIndex + nCount], &m_ptrs->pdata[nIndex],
				 ( nOldSize - nIndex ) * sizeof( gpointer ) );

		memset( &m_ptrs->pdata[nIndex], 0, nCount * sizeof( gpointer ) );
	}

	// insert new value in the gap
	while ( nCount-- )
		m_ptrs->pdata[nIndex++] = newElement;
}
void Copy( const CPtrArray& src ){
	g_ptr_array_set_size( m_ptrs, src.m_ptrs->len );
	memcpy( m_ptrs->pdata, src.m_ptrs->pdata, m_ptrs->len * sizeof( gpointer ) );
}

protected:
GPtrArray* m_ptrs;
};

typedef struct stringmap_s
{
	char* key;
	char* value;
} stringmap_t;

class CMapStringToString
{
public:
CMapStringToString ()
{ m_map = g_ptr_array_new(); };
~CMapStringToString (){
	for ( guint32 i = 0; i < m_map->len; i++ )
		FreeElement( (stringmap_t*)g_ptr_array_index( m_map,i ) );
	g_ptr_array_set_size( m_map, 0 );
	g_ptr_array_free( m_map, TRUE );
};
void SetAt( char* key, char* newValue ){
	for ( guint32 i = 0; i < m_map->len; i++ )
	{
		stringmap_t* entry = (stringmap_t*)g_ptr_array_index( m_map,i );
		if ( strcmp( entry->key, key ) == 0 ) {
			g_free( entry->value );
			entry->value = g_strdup( newValue );
			return;
		}
	}
	stringmap_t* entry = (stringmap_t*)g_malloc( sizeof( stringmap_t ) );
	entry->key = g_strdup( key );
	entry->value = g_strdup( newValue );
	g_ptr_array_add( m_map, entry );
}

bool Lookup( const char* key, CString& rValue ) const {
	for ( guint32 i = 0; i < m_map->len; i++ )
	{
		stringmap_t* entry = (stringmap_t*)g_ptr_array_index( m_map,i );
		if ( strcmp( entry->key, key ) == 0 ) {
			rValue = entry->value;
			return true;
		}
	}
	return false;
}

protected:
GPtrArray* m_map;

void FreeElement( stringmap_t* elem ){
	g_free( elem->key );
	g_free( elem->value );
	g_free( elem );
};
};

class FindFiles {
public:
FindFiles( const char *directory );
~FindFiles();

const char* NextFile();
private:
#ifdef _WIN32
Str directory;
HANDLE findHandle;
WIN32_FIND_DATA findFileData;
#else
DIR             * findHandle;
#endif
};

bool radCopyTree( const char* source, const char* dest, bool fatal_on_error = true );

typedef enum {
	PATH_FAIL,      // stat call failed (does not exist is likely)
	PATH_DIRECTORY,
	PATH_FILE
} EPathCheck;

// check a path for existence, return directory / file
EPathCheck CheckFile( const char *path );

bool radCreateDirectory( const char *directory, bool fatal_on_error = true );
bool radCopyFile( const char *lpExistingFileName, const char *lpNewFileName, bool fatal_on_error = true );

#endif // _MISSING_H_
