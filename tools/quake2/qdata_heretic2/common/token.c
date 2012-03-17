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
//** token.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "token.h"
#include "inout.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef enum
{
	CHR_EOF,
	CHR_LETTER,
	CHR_NUMBER,
	CHR_QUOTE,
	CHR_SPECIAL
} chr_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ProcessLetterToken( void );
static void ProcessNumberToken( void );
static void ProcessQuoteToken( void );
static void ProcessSpecialToken( void );
static qboolean CheckForKeyword( void );
static void NextChr( void );

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

tokenType_t tk_Token;
int tk_Line;
int tk_IntNumber;
float tk_FloatNumber;
char *tk_String;
char tk_SourceName[MAX_FILE_NAME_LENGTH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char Chr;
static char *FileStart;
static char *FilePtr;
static char *FileEnd;
static qboolean SourceOpen;
static char ASCIIToChrCode[256];
static char TokenStringBuffer[MAX_QUOTED_LENGTH];
static qboolean IncLineNumber;
static char TempBuffer[2048];

static struct
{
	char *name;
	tokenType_t token;
} Keywords[] =
{
	"model",            TK_MODEL,
	"mesh",             TK_MESH,
	"vertices",         TK_VERTICES,
	"edges",            TK_EDGES,
	"position",         TK_POSITION,
	"polygons",         TK_POLYGONS,
	"nodes",            TK_NODES,
	"rotation",         TK_ROTATION,
	"scaling",          TK_SCALING,
	"translation",      TK_TRANSLATION,
	"vertex",           TK_VERTEX,
	"HRCH",             TK_HRCH,
	"Softimage",        TK_SOFTIMAGE,
	"material",         TK_MATERIAL,
	"spline",           TK_SPLINE,

	"Named",            TK_C_NAMED,
	"object",           TK_OBJECT,
	"Tri",              TK_C_TRI,
	"Vertices",         TK_C_VERTICES,
	"Faces",            TK_C_FACES,
	"Vertex",           TK_C_VERTEX,
	"list",             TK_LIST,
	"Face",             TK_C_FACE,

	"Hexen",            TK_C_HEXEN,
	"Triangles",        TK_C_TRIANGLES,
	"Version",          TK_C_VERSION,
	"faces",            TK_FACES,
	"face",             TK_FACE,
	"origin",           TK_ORIGIN,

	"DK_clusters",      TK_CLUSTERS,
	"DK_cluster_ncvs",  TK_NUM_CLUSTER_VERTICES,
	"name",             TK_NAME,
	"DK_cluster_name",  TK_CLUSTER_NAME,
	"DK_cluster_state", TK_CLUSTER_STATE,

	"actor_data",       TK_ACTOR_DATA,
	"uvTexture",        TK_UVTEXTURE,

	NULL,               -1
};

static char *TokenNames[] =
{
	"<nothing>",
	"<unknown_char>",
	"<EOF>",
	"<identifier>",
	"<string>",
	"<int_number>",
	"<float_number>",
	"(",
	")",
	"{",
	"}",
	"[",
	"]",
	":",
	"mesh",
	"model",
	"nodes",
	"rotation",
	"scaling",
	"translation",
	"polygons",
	"position",
	"vertex",
	"vertices",
	"HRCH",
	"Softimage"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// TK_Init
//
//==========================================================================

void TK_Init( void ){
	int i;

	for ( i = 0; i < 256; i++ )
	{
		ASCIIToChrCode[i] = CHR_SPECIAL;
	}
	for ( i = '0'; i <= '9'; i++ )
	{
		ASCIIToChrCode[i] = CHR_NUMBER;
	}
	for ( i = 'A'; i <= 'Z'; i++ )
	{
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	for ( i = 'a'; i <= 'z'; i++ )
	{
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	ASCIIToChrCode[ASCII_QUOTE] = CHR_QUOTE;
	ASCIIToChrCode[ASCII_UNDERSCORE] = CHR_LETTER;
	ASCIIToChrCode[EOF_CHARACTER] = CHR_EOF;
	tk_String = TokenStringBuffer;
	IncLineNumber = FALSE;
	SourceOpen = FALSE;
}

//==========================================================================
//
// TK_OpenSource
//
//==========================================================================

void TK_OpenSource( char *fileName ){
	int size;

	TK_CloseSource();
	size = LoadFile( fileName, (void **)&FileStart );
	strcpy( tk_SourceName, fileName );
	SourceOpen = TRUE;
	FileEnd = FileStart + size;
	FilePtr = FileStart;
	tk_Line = 1;
	tk_Token = TK_NONE;
	NextChr();
}

//==========================================================================
//
// TK_CloseSource
//
//==========================================================================

void TK_CloseSource( void ){
	if ( SourceOpen ) {
		free( FileStart );
		SourceOpen = FALSE;
	}
}

//==========================================================================
//
// TK_Fetch
//
//==========================================================================

tokenType_t TK_Fetch( void ){
	while ( Chr == ASCII_SPACE )
	{
		NextChr();
	}
	if ( Chr == '-' ) {
		ProcessNumberToken();
	}
	else{switch ( ASCIIToChrCode[(byte)Chr] )
		 {
		 case CHR_EOF:
			 tk_Token = TK_EOF;
			 break;
		 case CHR_LETTER:
			 ProcessLetterToken();
			 break;
		 case CHR_NUMBER:
			 ProcessNumberToken();
			 break;
		 case CHR_QUOTE:
			 ProcessQuoteToken();
			 break;
		 default:
			 ProcessSpecialToken();
			 break;
		 }}
	return tk_Token;
}

//==========================================================================
//
// TK_Require
//
//==========================================================================

void TK_Require( tokenType_t tokType ){
	if ( tokType == TK_FLOATNUMBER && tk_Token == TK_INTNUMBER ) {
		tk_FloatNumber = (float)tk_IntNumber;
		tk_Token = TK_FLOATNUMBER;
		return;
	}
	if ( tk_Token != tokType ) {
		Error( "File '%s', line %d:\nExpected '%s', found '%s'.\n",
			   tk_SourceName, tk_Line, TokenNames[tokType],
			   TokenNames[tk_Token] );
	}
}

void TK_FetchRequire( tokenType_t tokType ){
	TK_Fetch();
	TK_Require( tokType );
}

tokenType_t TK_RequireFetch( tokenType_t tokType ){
	TK_Require( tokType );
	return TK_Fetch();
}

tokenType_t TK_FetchRequireFetch( tokenType_t tokType ){
	TK_Fetch();
	TK_Require( tokType );
	return TK_Fetch();
}

tokenType_t TK_Beyond( tokenType_t tokType ){
	while ( tk_Token != tokType )
	{
		if ( TK_Fetch() == TK_EOF ) {
			Error( "File '%s':\nCould not find token '%s'.\n",       // FIXME: TokenNames table not big enuff
				   tk_SourceName, TokenNames[tokType] );
		}
	}
	return TK_Fetch();
}

void TK_BeyondRequire( tokenType_t bTok, tokenType_t rTok ){
	TK_Beyond( bTok );
	TK_Require( rTok );
}

tokenType_t TK_Search( tokenType_t tokType ){
	while ( tk_Token != tokType )
	{
		if ( TK_Fetch() == TK_EOF ) {
			return TK_EOF;
		}
	}
	return TK_Fetch();
}

tokenType_t TK_Get( tokenType_t tokType ){
	while ( tk_Token != tokType )
	{
		if ( TK_Fetch() == TK_EOF ) {
			Error( "File '%s':\nCould not find token '%s'.\n",
				   tk_SourceName, TokenNames[tokType] );
		}
	}
	return tk_Token;
}

//==========================================================================
//
// ProcessLetterToken
//
//==========================================================================

static void ProcessLetterToken( void ){
	int i;
	char *text;

	i = 0;
	text = TokenStringBuffer;
	while ( ASCIIToChrCode[(byte)Chr] == CHR_LETTER
			|| ASCIIToChrCode[(byte)Chr] == CHR_NUMBER )
	{
		if ( ++i == MAX_IDENTIFIER_LENGTH ) {
			Error( "File '%s', line %d:\nIdentifier too long.\n",
				   tk_SourceName, tk_Line );
		}
		*text++ = Chr;
		NextChr();
	}
	*text = 0;
	if ( CheckForKeyword() == FALSE ) {
		tk_Token = TK_IDENTIFIER;
	}
}

//==========================================================================
//
// CheckForKeyword
//
//==========================================================================

static qboolean CheckForKeyword( void ){
	int i;

	for ( i = 0; Keywords[i].name != NULL; i++ )
	{
		if ( strcmp( tk_String, Keywords[i].name ) == 0 ) {
			tk_Token = Keywords[i].token;
			return TRUE;
		}
	}
	return FALSE;
}

//==========================================================================
//
// ProcessNumberToken
//
//==========================================================================

static void ProcessNumberToken( void ){
	char *buffer;

	buffer = TempBuffer;
	*buffer++ = Chr;
	NextChr();
	while ( ASCIIToChrCode[(byte)Chr] == CHR_NUMBER )
	{
		*buffer++ = Chr;
		NextChr();
	}
	if ( Chr == '.' ) { // Float
		*buffer++ = Chr;
		NextChr(); // Skip period
		while ( ASCIIToChrCode[(byte)Chr] == CHR_NUMBER )
		{
			*buffer++ = Chr;
			NextChr();
		}
		*buffer = 0;
		tk_FloatNumber = (float)atof( TempBuffer );
		tk_Token = TK_FLOATNUMBER;
		return;
	}

	// Integer
	*buffer = 0;
	tk_IntNumber = atoi( TempBuffer );
	tk_Token = TK_INTNUMBER;
}

//==========================================================================
//
// ProcessQuoteToken
//
//==========================================================================

static void ProcessQuoteToken( void ){
	int i;
	char *text;

	i = 0;
	text = TokenStringBuffer;
	NextChr();
	while ( Chr != ASCII_QUOTE )
	{
		if ( Chr == EOF_CHARACTER ) {
			Error( "File '%s', line %d:\n<EOF> inside string.\n",
				   tk_SourceName, tk_Line );
		}
		if ( ++i > MAX_QUOTED_LENGTH - 1 ) {
			Error( "File '%s', line %d:\nString literal too long.\n",
				   tk_SourceName, tk_Line );
		}
		*text++ = Chr;
		NextChr();
	}
	*text = 0;
	NextChr();
	tk_Token = TK_STRING;
}

//==========================================================================
//
// ProcessSpecialToken
//
//==========================================================================

static void ProcessSpecialToken( void ){
	char c;

	c = Chr;
	NextChr();
	switch ( c )
	{
	case '(':
		tk_Token = TK_LPAREN;
		break;
	case ')':
		tk_Token = TK_RPAREN;
		break;
	case '{':
		tk_Token = TK_LBRACE;
		break;
	case '}':
		tk_Token = TK_RBRACE;
		break;
	case '[':
		tk_Token = TK_LBRACKET;
		break;
	case ']':
		tk_Token = TK_RBRACKET;
		break;
	case ':':
		tk_Token = TK_COLON;
		break;
	default:
		tk_Token = TK_UNKNOWNCHAR;
		break;
	}
}

//==========================================================================
//
// NextChr
//
//==========================================================================

static void NextChr( void ){
	if ( FilePtr >= FileEnd ) {
		Chr = EOF_CHARACTER;
		return;
	}
	if ( IncLineNumber == TRUE ) {
		tk_Line++;
		IncLineNumber = FALSE;
	}
	Chr = *FilePtr++;
	if ( Chr < ASCII_SPACE ) {
		if ( Chr == '\n' ) {
			IncLineNumber = TRUE;
		}
		Chr = ASCII_SPACE;
	}
}
