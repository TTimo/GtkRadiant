/*
   This code is based on source provided under the terms of the Id Software
   LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with the
   GtkRadiant sources (see LICENSE_ID). If you did not receive a copy of
   LICENSE_ID, please contact Id Software immediately at info@idsoftware.com.

   All changes and additions to the original source which have been developed by
   other contributors (see CONTRIBUTORS) are provided under the terms of the
   license the contributors choose (see LICENSE), to the extent permitted by the
   LICENSE_ID. If you did not receive a copy of the contributor license,
   please contact the GtkRadiant maintainers at info@gtkradiant.com immediately.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// q_parse.c -- support for parsing text files

#include "q_shared.h"

/*
   ============================================================================

   PARSING

   ============================================================================
 */

// multiple character punctuation tokens
static const char *punctuation[] = {
	"+=", "-=",  "*=",  "/=", "&=", "|=", "++", "--",
	"&&", "||",  "<=",  ">=", "==", "!=",
	NULL
};

typedef struct {
	char token[MAX_TOKEN_CHARS];
	int lines;
	qboolean ungetToken;
	char parseFile[MAX_QPATH];
} parseInfo_t;

#define MAX_PARSE_INFO  16
static parseInfo_t parseInfo[MAX_PARSE_INFO];
static int parseInfoNum;
static parseInfo_t  *pi = &parseInfo[0];

/*
   ===================
   Com_BeginParseSession
   ===================
 */
void Com_BeginParseSession( const char *filename ) {
	if ( parseInfoNum == MAX_PARSE_INFO - 1 ) {
		Com_Error( ERR_FATAL, "Com_BeginParseSession: session overflow" );
	}
	parseInfoNum++;
	pi = &parseInfo[parseInfoNum];

	pi->lines = 1;
	Q_strncpyz( pi->parseFile, filename, sizeof( pi->parseFile ) );
}

/*
   ===================
   Com_EndParseSession
   ===================
 */
void Com_EndParseSession( void ) {
	if ( parseInfoNum == 0 ) {
		Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
	}
	parseInfoNum--;
	pi = &parseInfo[parseInfoNum];
}

/*
   ===================
   Com_GetCurrentParseLine
   ===================
 */
int Com_GetCurrentParseLine( void ) {
	return pi->lines;
}

/*
   ===================
   Com_ScriptError

   Prints the script name and line number in the message
   ===================
 */
void Com_ScriptError( const char *msg, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, msg );
	vsprintf( string, msg,argptr );
	va_end( argptr );

	Com_Error( ERR_DROP, "File %s, line %i: %s", pi->parseFile, pi->lines, string );
}

void Com_ScriptWarning( const char *msg, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, msg );
	vsprintf( string, msg,argptr );
	va_end( argptr );

	Com_Printf( "File %s, line %i: %s", pi->parseFile, pi->lines, string );
}


/*
   ===================
   Com_UngetToken

   Calling this will make the next Com_Parse return
   the current token instead of advancing the pointer
   ===================
 */
void Com_UngetToken( void ) {
	if ( pi->ungetToken ) {
		Com_ScriptError( "UngetToken called twice" );
	}
	pi->ungetToken = qtrue;
}


static const char *SkipWhitespace( const char (*data), qboolean *hasNewLines ) {
	int c;

	while ( ( c = *data ) <= ' ' ) {
		if ( !c ) {
			return NULL;
		}
		if ( c == '\n' ) {
			pi->lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

/*
   ==============
   Com_ParseExt

   Parse a token out of a string
   Will never return NULL, just empty strings.
   An empty string will only be returned at end of file.

   If "allowLineBreaks" is qtrue then an empty
   string will be returned if the next token is
   a newline.
   ==============
 */
static char *Com_ParseExt( const char *( *data_p ), qboolean allowLineBreaks ) {
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	const char *data;
	const char **punc;

	if ( !data_p ) {
		Com_Error( ERR_FATAL, "Com_ParseExt: NULL data_p" );
	}

	data = *data_p;
	len = 0;
	pi->token[0] = 0;

	// make sure incoming data is valid
	if ( !data ) {
		*data_p = NULL;
		return pi->token;
	}

	// skip any leading whitespace
	while ( 1 ) {
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data ) {
			*data_p = NULL;
			return pi->token;
		}
		if ( hasNewLines && !allowLineBreaks ) {
			*data_p = data;
			return pi->token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' ) {
			while ( *data && *data != '\n' ) {
				data++;
			}
			continue;
		}

		// skip /* */ comments
		if ( c == '/' && data[1] == '*' ) {
			while ( *data && ( *data != '*' || data[1] != '/' ) ) {
				if ( *data == '\n' ) {
					pi->lines++;
				}
				data++;
			}
			if ( *data ) {
				data += 2;
			}
			continue;
		}

		// a real token to parse
		break;
	}

	// handle quoted strings
	if ( c == '\"' ) {
		data++;
		while ( 1 ) {
			c = *data++;
			if ( ( c == '\\' ) && ( *data == '\"' ) ) {
				// allow quoted strings to use \" to indicate the " character
				data++;
			}
			else if ( c == '\"' || !c ) {
				pi->token[len] = 0;
				*data_p = ( char * ) data;
				return pi->token;
			}
			else if ( *data == '\n' ) {
				pi->lines++;
			}
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				pi->token[len] = c;
				len++;
			}
		}
	}

	// check for a number
	// is this parsing of negative numbers going to cause expression problems
	if ( ( c >= '0' && c <= '9' ) || ( c == '-' && data[ 1 ] >= '0' && data[ 1 ] <= '9' ) ||
		 ( c == '.' && data[ 1 ] >= '0' && data[ 1 ] <= '9' ) ) {
		do  {

			if ( len < MAX_TOKEN_CHARS - 1 ) {
				pi->token[len] = c;
				len++;
			}
			data++;

			c = *data;
		} while ( ( c >= '0' && c <= '9' ) || c == '.' );

		// parse the exponent
		if ( c == 'e' || c == 'E' ) {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				pi->token[len] = c;
				len++;
			}
			data++;
			c = *data;

			if ( c == '-' || c == '+' ) {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					pi->token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}

			do  {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					pi->token[len] = c;
					len++;
				}
				data++;

				c = *data;
			} while ( c >= '0' && c <= '9' );
		}

		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		pi->token[len] = 0;

		*data_p = ( char * ) data;
		return pi->token;
	}

	// check for a regular word
	// we still allow forward and back slashes in name tokens for pathnames
	// and also colons for drive letters
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_' || c == '/' || c == '\\' ) {
		do  {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				pi->token[len] = c;
				len++;
			}
			data++;

			c = *data;
		} while ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_'
				  || ( c >= '0' && c <= '9' ) || c == '/' || c == '\\' || c == ':' || c == '.' );

		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		pi->token[len] = 0;

		*data_p = ( char * ) data;
		return pi->token;
	}

	// check for multi-character punctuation token
	for ( punc = punctuation ; *punc ; punc++ ) {
		int l;
		int j;

		l = strlen( *punc );
		for ( j = 0 ; j < l ; j++ ) {
			if ( data[j] != ( *punc )[j] ) {
				break;
			}
		}
		if ( j == l ) {
			// a valid multi-character punctuation
			memcpy( pi->token, *punc, l );
			pi->token[l] = 0;
			data += l;
			*data_p = (char *)data;
			return pi->token;
		}
	}

	// single character punctuation
	pi->token[0] = *data;
	pi->token[1] = 0;
	data++;
	*data_p = (char *)data;

	return pi->token;
}

/*
   ===================
   Com_Parse
   ===================
 */
const char *Com_Parse( const char *( *data_p ) ) {
	if ( pi->ungetToken ) {
		pi->ungetToken = qfalse;
		return pi->token;
	}
	return Com_ParseExt( data_p, qtrue );
}

/*
   ===================
   Com_ParseOnLine
   ===================
 */
const char *Com_ParseOnLine( const char *( *data_p ) ) {
	if ( pi->ungetToken ) {
		pi->ungetToken = qfalse;
		return pi->token;
	}
	return Com_ParseExt( data_p, qfalse );
}



/*
   ==================
   Com_MatchToken
   ==================
 */
void Com_MatchToken( const char *( *buf_p ), const char *match, qboolean warning ) {
	const char  *token;

	token = Com_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		if ( warning ) {
			Com_ScriptWarning( "MatchToken: %s != %s", token, match );
		}
		else {
			Com_ScriptError( "MatchToken: %s != %s", token, match );
		}
	}
}


/*
   =================
   Com_SkipBracedSection

   The next token should be an open brace.
   Skips until a matching close brace is found.
   Internal brace depths are properly skipped.
   =================
 */
void Com_SkipBracedSection( const char *( *program ) ) {
	const char          *token;
	int depth;

	depth = 0;
	do {
		token = Com_Parse( program );
		if ( token[1] == 0 ) {
			if ( token[0] == '{' ) {
				depth++;
			}
			else if ( token[0] == '}' ) {
				depth--;
			}
		}
	} while ( depth && *program );
}

/*
   =================
   Com_SkipRestOfLine
   =================
 */
void Com_SkipRestOfLine( const char *( *data ) ) {
	const char  *p;
	int c;

	p = *data;
	while ( ( c = *p++ ) != 0 ) {
		if ( c == '\n' ) {
			pi->lines++;
			break;
		}
	}

	*data = p;
}

/*
   ====================
   Com_ParseRestOfLine
   ====================
 */
const char *Com_ParseRestOfLine( const char *( *data_p ) ) {
	static char line[MAX_TOKEN_CHARS];
	const char *token;

	line[0] = 0;
	while ( 1 ) {
		token = Com_ParseOnLine( data_p );
		if ( !token[0] ) {
			break;
		}
		if ( line[0] ) {
			Q_strcat( line, sizeof( line ), " " );
		}
		Q_strcat( line, sizeof( line ), token );
	}

	return line;
}


float Com_ParseFloat( const char *( *buf_p ) ) {
	const char      *token;

	token = Com_Parse( buf_p );
	if ( !token[0] ) {
		return 0;
	}
	return atof( token );
}

int Com_ParseInt( const char *( *buf_p ) ) {
	const char      *token;

	token = Com_Parse( buf_p );
	if ( !token[0] ) {
		return 0;
	}
	return (int)atof( token );
}



void Com_Parse1DMatrix( const char *( *buf_p ), int x, float *m ) {
	const char  *token;
	int i;

	Com_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < x ; i++ ) {
		token = Com_Parse( buf_p );
		m[i] = atof( token );
	}

	Com_MatchToken( buf_p, ")" );
}

void Com_Parse2DMatrix( const char *( *buf_p ), int y, int x, float *m ) {
	int i;

	Com_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < y ; i++ ) {
		Com_Parse1DMatrix( buf_p, x, m + i * x );
	}

	Com_MatchToken( buf_p, ")" );
}

void Com_Parse3DMatrix( const char *( *buf_p ), int z, int y, int x, float *m ) {
	int i;

	Com_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < z ; i++ ) {
		Com_Parse2DMatrix( buf_p, y, x, m + i * x * y );
	}

	Com_MatchToken( buf_p, ")" );
}
