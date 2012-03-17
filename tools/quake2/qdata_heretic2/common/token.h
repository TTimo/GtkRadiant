/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


//**************************************************************************
//**
//** token.h
//**
//**************************************************************************

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "cmdlib.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif
#define ASCII_SPACE 32
#define ASCII_QUOTE 34
#define ASCII_UNDERSCORE 95
#define EOF_CHARACTER 127
#define MAX_IDENTIFIER_LENGTH 64
#define MAX_QUOTED_LENGTH 1024
#define MAX_FILE_NAME_LENGTH 1024

typedef enum
{
	TK_NONE,
	TK_UNKNOWNCHAR,
	TK_EOF,
	TK_IDENTIFIER,          // VALUE: (char *) tk_String
	TK_STRING,              // VALUE: (char *) tk_String
	TK_INTNUMBER,           // VALUE: (int) tk_IntNumber
	TK_FLOATNUMBER,         // VALUE: (float) tk_FloatNumber
	TK_LPAREN,
	TK_RPAREN,
	TK_LBRACE,
	TK_RBRACE,              // 10
	TK_LBRACKET,
	TK_RBRACKET,
	TK_COLON,
	TK_MESH,
	TK_MODEL,               // 15
	TK_NODES,
	TK_ROTATION,
	TK_SCALING,
	TK_TRANSLATION,
	TK_POLYGONS,            // 20
	TK_POSITION,
	TK_VERTEX,
	TK_VERTICES,
	TK_EDGES,
	TK_HRCH,                // 25
	TK_SOFTIMAGE,
	TK_MATERIAL,
	TK_SPLINE,              // 28

	TK_C_NAMED,
	TK_OBJECT,              // 30
	TK_C_TRI,
	TK_C_VERTICES,
	TK_C_FACES,
	TK_C_VERTEX,
	TK_LIST,                // 35
	TK_C_FACE,

	TK_C_HEXEN,
	TK_C_TRIANGLES,
	TK_C_VERSION,
	TK_FACES,               // 40
	TK_FACE,
	TK_ORIGIN,

	TK_CLUSTERS,
	TK_NUM_CLUSTER_VERTICES,
	TK_NAME,                // 45
	TK_CLUSTER_NAME,
	TK_CLUSTER_STATE,

	TK_ACTOR_DATA,
	TK_UVTEXTURE,
} tokenType_t;

void TK_Init( void );
void TK_OpenSource( char *fileName );
void TK_CloseSource( void );
tokenType_t TK_Fetch( void );
void TK_Require( tokenType_t tokType );
void TK_FetchRequire( tokenType_t tokType );
tokenType_t TK_RequireFetch( tokenType_t tokType );
tokenType_t TK_FetchRequireFetch( tokenType_t tokType );
tokenType_t TK_Beyond( tokenType_t tokType );
void TK_BeyondRequire( tokenType_t bTok, tokenType_t rTok );
tokenType_t TK_Search( tokenType_t tokType );
tokenType_t TK_Get( tokenType_t tokType );

extern tokenType_t tk_Token;
extern int tk_Line;
extern int tk_IntNumber;
extern float tk_FloatNumber;
extern char *tk_String;
extern char tk_SourceName[MAX_FILE_NAME_LENGTH];

#endif
