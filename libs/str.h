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

#ifndef __STR__
#define __STR__

//
// class Str
// loose replacement for CString from MFC
//

#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef __APPLE__
  #ifdef NULL
	#undef NULL
	#define NULL 0
  #endif
#endif


#ifdef _WIN32
#define strcasecmp strcmpi
#endif

// NOTE TTimo __StrDup was initially implemented in pakstuff.cpp
//   causing a bunch of issues for broader targets that use Str.h (such as plugins and modules)
//   Q_StrDup should be used now, using a #define __StrDup for easy transition

#define __StrDup Q_StrDup

inline char* Q_StrDup( char* _pStr ) {
	const char* pStr = _pStr;
	if ( pStr == NULL ) {
		pStr = "";
	}

	return strcpy( new char[ strlen( pStr ) + 1 ], pStr );
}

inline char* Q_StrDup( const char* pStr ) {
	if ( pStr == NULL ) {
		pStr = "";
	}

	return strcpy( new char[strlen( pStr ) + 1], pStr );
}

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#define strcmpi strcasecmp
#define stricmp strcasecmp
#define strnicmp strncasecmp

inline char* strlwr( char* string ){
	char *cp;
	for ( cp = string; *cp; ++cp )
	{
		if ( 'A' <= *cp && *cp <= 'Z' ) {
			*cp += 'a' - 'A';
		}
	}

	return string;
}

inline char* strupr( char* string ){
	char *cp;
	for ( cp = string; *cp; ++cp )
	{
		if ( 'a' <= *cp && *cp <= 'z' ) {
			*cp += 'A' - 'a';
		}
	}

	return string;
}
#endif

static char *g_pStrWork = NULL;

class Str
{
protected:
bool m_bIgnoreCase;
char *m_pStr;

public:
Str(){
	m_bIgnoreCase = true;
	m_pStr = new char[1];
	m_pStr[0] = '\0';
}

Str( char *p ){
	m_bIgnoreCase = true;
	m_pStr = __StrDup( p );
}

Str( const char *p ){
	m_bIgnoreCase = true;
	m_pStr = __StrDup( p );
}

Str( const unsigned char *p ){
	m_bIgnoreCase = true;
	m_pStr = __StrDup( (const char *)p );
}

Str( const char c ){
	m_bIgnoreCase = true;
	m_pStr = new char[2];
	m_pStr[0] = c;
	m_pStr[1] = '\0';
}

const char* GetBuffer() const {
	return m_pStr;
}

Str( const Str &s ){
	m_bIgnoreCase = true;
	m_pStr = __StrDup( s.GetBuffer() );
}

void Deallocate(){
	delete []m_pStr;
	m_pStr = NULL;
}

void Allocate( int n ){
	Deallocate();
	m_pStr = new char[n];
}

void MakeEmpty(){
	Deallocate();
	m_pStr = __StrDup( "" );
}

virtual ~Str(){
	Deallocate();
	// NOTE TTimo: someone explain this g_pStrWork to me?
	if ( g_pStrWork ) {
		delete []g_pStrWork;
	}
	g_pStrWork = NULL;
}

void MakeLower(){
	if ( m_pStr ) {
		strlwr( m_pStr );
	}
}

void MakeUpper(){
	if ( m_pStr ) {
		strupr( m_pStr );
	}
}

void TrimRight(){
	char* lpsz = m_pStr;
	char* lpszLast = NULL;
	while ( *lpsz != '\0' )
	{
		if ( isspace( *lpsz ) ) {
			if ( lpszLast == NULL ) {
				lpszLast = lpsz;
			}
		}
		else{
			lpszLast = NULL;
		}
		lpsz++;
	}

	if ( lpszLast != NULL ) {
		// truncate at trailing space start
		*lpszLast = '\0';
	}
}

void TrimLeft(){
	// find first non-space character
	char* lpsz = m_pStr;
	while ( isspace( *lpsz ) )
		lpsz++;

	// fix up data and length
	int nDataLength = GetLength() - ( lpsz - m_pStr );
	memmove( m_pStr, lpsz, ( nDataLength + 1 ) );
}

int Find( const char *p ){
	char *pf = strstr( m_pStr, p );
	return ( pf ) ? ( pf - m_pStr ) : -1;
}

// search starting at a given offset
int Find( const char *p, int offset ){
	char *pf = strstr( m_pStr + offset, p );
	return ( pf ) ? ( pf - m_pStr ) : -1;
}

int Find( const char ch ){
	char *pf = strchr( m_pStr, ch );
	return ( pf ) ? ( pf - m_pStr ) : -1;
}

int ReverseFind( const char ch ){
	char *pf = strrchr( m_pStr, ch );
	return ( pf ) ? ( pf - m_pStr ) : -1;
}

int Compare( const char* str ) const {
	return strcmp( m_pStr, str );
}

int CompareNoCase( const char* str ) const {
	return strcasecmp( m_pStr, str );
}

int GetLength(){
	return ( m_pStr ) ? strlen( m_pStr ) : 0;
}

const char* Left( int n ){
	delete []g_pStrWork;
	if ( n > 0 ) {
		g_pStrWork = new char[n + 1];
		strncpy( g_pStrWork, m_pStr, n );
		g_pStrWork[n] = '\0';
	}
	else
	{
		g_pStrWork = new char[1];
		g_pStrWork[0] = '\0';
	}
	return g_pStrWork;
}

const char* Right( int n ){
	delete []g_pStrWork;
	if ( n > 0 ) {
		g_pStrWork = new char[n + 1];
		int nStart = GetLength() - n;
		strncpy( g_pStrWork, &m_pStr[nStart], n );
		g_pStrWork[n] = '\0';
	}
	else
	{
		g_pStrWork = new char[1];
		g_pStrWork[0] = '\0';
	}
	return g_pStrWork;
}

const char* Mid( int nFirst ) const {
	return Mid( nFirst, strlen( m_pStr ) - nFirst );
}

const char* Mid( int first, int n ) const {
	delete []g_pStrWork;
	if ( n > 0 ) {
		g_pStrWork = new char[n + 1];
		strncpy( g_pStrWork, m_pStr + first, n );
		g_pStrWork[n] = '\0';
	}
	else
	{
		g_pStrWork = new char[1];
		g_pStrWork[0] = '\0';
	}
	return g_pStrWork;
}

#ifdef __G_LIB_H__
void Format( const char* fmt, ... ){
	va_list args;
	char *buffer;

	va_start( args, fmt );
	buffer = g_strdup_vprintf( fmt, args );
	va_end( args );

	delete[] m_pStr;
	m_pStr = __StrDup( buffer );
	g_free( buffer );
}
#else
void Format( const char* fmt, ... ){
	va_list args;
	m_pStr = new char[1024];

	va_start( args, fmt );
	vsprintf( m_pStr, fmt, args );
	va_end( args );
}
#endif

void SetAt( int n, char ch ){
	if ( n >= 0 && n < GetLength() ) {
		m_pStr[n] = ch;
	}
}

// NOTE: unlike CString, this looses the pointer
void ReleaseBuffer( int n = -1 ){
	if ( n == -1 ) {
		n = GetLength();
	}

	char* tmp = m_pStr;
	tmp[n] = '\0';
	m_pStr = __StrDup( tmp );
	delete []tmp;
}

char* GetBufferSetLength( int n ){
	if ( n < 0 ) {
		n = 0;
	}

	char *p = new char[n + 1];
	strncpy( p, m_pStr, n );
	p[n] = '\0';
	delete []m_pStr;
	m_pStr = p;
	return m_pStr;
}

//  char& operator *() { return *m_pStr; }
//  char& operator *() const { return *const_cast<Str*>(this)->m_pStr; }
operator void*() {
	return m_pStr;
}
operator char*() {
	return m_pStr;
}
operator const char*(){
	return reinterpret_cast<const char*>( m_pStr );
}
operator unsigned char*() {
	return reinterpret_cast<unsigned char*>( m_pStr );
}
operator const unsigned char*() {
	return reinterpret_cast<const unsigned char*>( m_pStr );
}
Str& operator =( const Str& rhs ){
	if ( &rhs != this ) {
		delete[] m_pStr;
		m_pStr = __StrDup( rhs.m_pStr );
	}
	return *this;
}

Str& operator =( const char* pStr ){
	if ( m_pStr != pStr ) {
		delete[] m_pStr;
		m_pStr = __StrDup( pStr );
	}
	return *this;
}

Str& operator +=( const char ch ){
	int len = GetLength();
	char *p = new char[len + 1 + 1];

	if ( m_pStr ) {
		strcpy( p, m_pStr );
		delete[] m_pStr;
	}

	m_pStr = p;
	m_pStr[len] = ch;
	m_pStr[len + 1] = '\0';

	return *this;
}

Str& operator +=( const char *pStr ){
	if ( pStr ) {
		if ( m_pStr ) {
			char *p = new char[strlen( m_pStr ) + strlen( pStr ) + 1];
			strcpy( p, m_pStr );
			strcat( p, pStr );
			delete[] m_pStr;
			m_pStr = p;
		}
		else
		{
			m_pStr = __StrDup( pStr );
		}
	}
	return *this;
}


bool operator ==( const Str& rhs ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, rhs.m_pStr ) == 0 : strcmp( m_pStr, rhs.m_pStr ) == 0; }
bool operator ==( char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) == 0 : strcmp( m_pStr, pStr ) == 0; }
bool operator ==( const char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) == 0 : strcmp( m_pStr, pStr ) == 0; }
bool operator !=( Str& rhs ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, rhs.m_pStr ) != 0 : strcmp( m_pStr, rhs.m_pStr ) != 0; }
bool operator !=( char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) != 0 : strcmp( m_pStr, pStr ) != 0; }
bool operator !=( const char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) != 0 : strcmp( m_pStr, pStr ) != 0; }
bool operator <( const Str& rhs ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, rhs.m_pStr ) < 0 : strcmp( m_pStr, rhs.m_pStr ) < 0; }
bool operator <( char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) < 0 : strcmp( m_pStr, pStr ) < 0; }
bool operator <( const char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) < 0 : strcmp( m_pStr, pStr ) < 0; }
bool operator >( const Str& rhs ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, rhs.m_pStr ) > 0 : strcmp( m_pStr, rhs.m_pStr ) > 0; }
bool operator >( char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) > 0 : strcmp( m_pStr, pStr ) > 0; }
bool operator >( const char* pStr ) const { return ( m_bIgnoreCase ) ? stricmp( m_pStr, pStr ) > 0 : strcmp( m_pStr, pStr ) > 0; }
char& operator []( int nIndex ) { return m_pStr[nIndex]; }
char& operator []( int nIndex ) const { return m_pStr[nIndex]; }
const char GetAt( int nIndex ) { return m_pStr[nIndex]; }
};



#endif
