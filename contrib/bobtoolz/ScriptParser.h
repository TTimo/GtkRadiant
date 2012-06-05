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

#ifndef _SCRIPTPARSER_H_
#define _SCRIPTPARSER_H_

#include "interfaces/IScriptParser.h"

#define SP_MAX_BREAKCHARS 16

class CScriptParser : public IScriptParser {
public:
CScriptParser( void );
~CScriptParser( void );
private:
char m_breakChars[SP_MAX_BREAKCHARS];
char*   m_pScript;
char*   m_pScriptSection;
char*   m_pLastScriptSection;
char*   m_pToken;

void SkipWhitespace( bool* pbNewLines );
void ClearBuffer( void );
const char* MakeToken( const char* pToken );
bool IsBreakChar( char c );
public:
const char* GetToken( bool bAllowLinebreaks );
void SkipBracedSection( void );
void SkipRestOfLine( void );
void UndoGetToken( void );
void ResetParseSession( void );

char* GetBufferCopy( void );
int GetTokenOffset( void );

void LoadScript( const char* pScript );
void SetScript( char* pScript );

void AddBreakChar( char c );
private:
};

#endif
