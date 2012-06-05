/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "StdAfx.h"
#include "ScriptParser.h"

CScriptParser::CScriptParser( void ) :
	m_pScript( NULL ),
	m_pScriptSection( NULL ),
	m_pLastScriptSection( NULL ),
	m_pToken( NULL ) {
	ClearBuffer();
}

CScriptParser::~CScriptParser( void ) {
	ClearBuffer();
}

void CScriptParser::ClearBuffer( void ) {
	if ( m_pScript ) {
		delete[] m_pScript;
		m_pScript = NULL;
	}
	if ( m_pToken ) {
		delete[] m_pToken;
		m_pToken = NULL;
	}
	m_pScriptSection = NULL;
	m_pLastScriptSection = NULL;
	memset( m_breakChars, 0, sizeof( m_breakChars ) );
}

const char* CScriptParser::MakeToken( const char* pToken ) {
	if ( m_pToken ) {
		delete[] m_pToken;
		m_pToken = NULL;
	}

	if ( !pToken ) {
		pToken = "";
	}

	int len = static_cast<int>( strlen( pToken ) );

	m_pToken = new char[len + 1];
	m_pToken[len] = '\0';
	strcpy( m_pToken, pToken );

	return m_pToken;
}

#define MAX_TOKEN_STRING 1024
// Should NEVER return NULL
const char* CScriptParser::GetToken( bool bAllowLinebreaks ) {
	int c = 0, len;
	char token[MAX_TOKEN_STRING];
	bool bNewLines = false;

	m_pLastScriptSection = m_pScriptSection;

	len = 0;
	*token = '\0';

	if ( !m_pScript || !m_pScriptSection ) {
		return MakeToken( token );
	}

	while ( true ) {
		SkipWhitespace( &bNewLines );
		if ( !*m_pScriptSection ) {
			return MakeToken( token );
		}
		if ( bNewLines && !bAllowLinebreaks ) {
			return MakeToken( token );
		}

		c = *m_pScriptSection;

		if ( c == '/' && m_pScriptSection[1] == '/' ) { // C style comments
			m_pScriptSection += 2;
			while ( *m_pScriptSection && *m_pScriptSection != '\n' ) {
				m_pScriptSection++;
			}
		}
		else if ( c == '/' && m_pScriptSection[1] == '*' ) { // C++ style comments
			m_pScriptSection += 2;
			while ( *m_pScriptSection && ( *m_pScriptSection != '*' || m_pScriptSection[1] != '/' ) ) {
				m_pScriptSection++;
			}
			if ( *m_pScriptSection ) {
				m_pScriptSection += 2;
			}
		}
		else {
			break;
		}
	}

	if ( c == '\"' ) {
		m_pScriptSection++;
		while ( true ) {
			c = *m_pScriptSection++;
			if ( c == '\"' || !c ) {
				token[len] = 0;
				return MakeToken( token );
			}
			if ( len < MAX_TOKEN_STRING ) {
				token[len] = c;
				len++;
			}
		}
	}

	do {
		if ( len > 0 && IsBreakChar( *m_pScriptSection ) ) {
			break;
		}

		if ( len < MAX_TOKEN_STRING ) {
			token[len] = c;
			len++;
		}
		m_pScriptSection++;

		if ( IsBreakChar( c ) ) {
			break;
		}

		c = *m_pScriptSection;
	} while ( c > 32 );

	if ( len == MAX_TOKEN_STRING ) {
		len = 0;
	}
	token[len] = 0;

	return MakeToken( token );
}

void CScriptParser::SkipWhitespace( bool* pbNewLines ) {
	int c;

	if ( !m_pScript || !m_pScriptSection ) {
		return;
	}

	while ( ( c = *m_pScriptSection ) <= ' ' ) {
		if ( !c ) {
			return;
		}
		if ( c == '\n' ) {
			*pbNewLines = true;
		}
		m_pScriptSection++;
	}
}

void CScriptParser::SkipBracedSection( void ) {
	const char      *token;
	int depth;

	depth = 0;
	do {
		token = GetToken( true );
		if ( token[1] == 0 ) {
			if ( *token == '{' ) {
				depth++;
			}
			else if ( *token == '}' ) {
				depth--;
			}
		}
	} while ( depth && *m_pScriptSection );
}

void CScriptParser::SkipRestOfLine( void ) {
	char    *p;
	int c;

	p = m_pScriptSection;
	while ( ( c = *p++ ) != 0 ) {
		if ( c == '\n' ) {
			break;
		}
	}
	m_pScriptSection = p;
}

void CScriptParser::UndoGetToken( void ) {
	if ( !m_pLastScriptSection ) {
		return;
	}
	m_pScriptSection = m_pLastScriptSection;
	m_pLastScriptSection = NULL;
}

void CScriptParser::ResetParseSession( void ) {
	if ( !m_pScript ) {
		return;
	}

	m_pScriptSection = m_pScript;
	m_pLastScriptSection = NULL;
}

char* CScriptParser::GetBufferCopy( void ) {
	if ( !m_pScript ) {
		return NULL;
	}

	int len = static_cast<int>( strlen( m_pScript ) );
	char* pBuffer = new char[len + 1];
	strcpy( pBuffer, m_pScript );
	return pBuffer;
}

int CScriptParser::GetTokenOffset( void ) {
	if ( !m_pScript || !m_pScriptSection ) {
		return 0;
	}

	return static_cast<int>( m_pScriptSection - m_pScript );
}

void CScriptParser::LoadScript( const char* pScript ) {
	ClearBuffer();

	int len = static_cast<int>( strlen( pScript ) );
	if ( len <= 0 ) {
		return;
	}

	m_pScript = new char[len + 1];
	m_pScript[len] = '\0';

	strcpy( m_pScript, pScript );
	m_pScriptSection = m_pScript;
}

void CScriptParser::AddBreakChar( char c ) {
	for ( int i = 0; i < SP_MAX_BREAKCHARS; i++ ) {
		if ( !m_breakChars[i] ) {
			m_breakChars[i] = c;
			return;
		}
	}

	// TODO: Error: max break chars hit
}

bool CScriptParser::IsBreakChar( char c ) {
	for ( int i = 0; i < SP_MAX_BREAKCHARS; i++ ) {
		if ( !m_breakChars[i] ) {
			return false;
		}
		if ( m_breakChars[i] == c ) {
			return true;
		}
	}
	return false;
}

void CScriptParser::SetScript( char* pScript ) {
	ClearBuffer();

	int len = static_cast<int>( strlen( pScript ) );
	if ( len <= 0 ) {
		return;
	}

	m_pScript = pScript;
	m_pScriptSection = m_pScript;
}
