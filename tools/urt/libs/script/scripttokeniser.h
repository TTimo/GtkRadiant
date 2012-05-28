
#if !defined( INCLUDED_SCRIPT_SCRIPTTOKENISER_H )
#define INCLUDED_SCRIPT_SCRIPTTOKENISER_H

#include "iscriplib.h"

class ScriptTokeniser : public Tokeniser
{
enum CharType
{
	eWhitespace,
	eCharToken,
	eNewline,
	eCharQuote,
	eCharSolidus,
	eCharStar,
	eCharSpecial,
};

typedef bool ( ScriptTokeniser::*Tokenise )( char c );

Tokenise m_stack[3];
Tokenise* m_state;
SingleCharacterInputStream<TextInputStream> m_istream;
std::size_t m_scriptline;

char m_token[MAXTOKEN];
char* m_write;

char m_current;
bool m_eof;
bool m_crossline;
bool m_unget;

bool m_special;

CharType charType( const char c ){
	switch ( c )
	{
	case '\n': return eNewline;
	case '"': return eCharQuote;
	case '/': return eCharSolidus;
	case '*': return eCharStar;
	case '{': case '(': case '}': case ')': case '[': case ']': case ',': case ':': return ( m_special ) ? eCharSpecial : eCharToken;
	}

	if ( c > 32 ) {
		return eCharToken;
	}
	return eWhitespace;
}

Tokenise state(){
	return *m_state;
}
void push( Tokenise state ){
	ASSERT_MESSAGE( m_state != m_stack + 2, "token parser: illegal stack push" );
	*( ++m_state ) = state;
}
void pop(){
	ASSERT_MESSAGE( m_state != m_stack, "token parser: illegal stack pop" );
	--m_state;
}
void add( const char c ){
	if ( m_write < m_token + MAXTOKEN - 1 ) {
		*m_write++ = c;
	}
}
void remove(){
	ASSERT_MESSAGE( m_write > m_token, "no char to remove" );
	--m_write;
}

bool tokeniseDefault( char c ){
	switch ( charType( c ) )
	{
	case eNewline:
		ASSERT_MESSAGE( m_crossline, "unexpected end-of-line before token" );
		break;
	case eCharToken:
	case eCharStar:
		push( Tokenise( &ScriptTokeniser::tokeniseToken ) );
		add( c );
		break;
	case eCharSpecial:
		push( Tokenise( &ScriptTokeniser::tokeniseSpecial ) );
		add( c );
		break;
	case eCharQuote:
		push( Tokenise( &ScriptTokeniser::tokeniseQuotedToken ) );
		break;
	case eCharSolidus:
		push( Tokenise( &ScriptTokeniser::tokeniseSolidus ) );
		break;
	default:
		break;
	}
	return false;
}
bool tokeniseToken( char c ){
	switch ( charType( c ) )
	{
	case eNewline:
	case eWhitespace:
	case eCharQuote:
	case eCharSpecial:
		pop();
		return true; // emit token
	case eCharSolidus:
		push( Tokenise( &ScriptTokeniser::tokeniseSolidus ) );
		break;
	case eCharToken:
	case eCharStar:
		add( c );
		break;
	default:
		break;
	}
	return false;
}
bool tokeniseQuotedToken( char c ){
	switch ( charType( c ) )
	{
	case eNewline:
		ASSERT_MESSAGE( !m_crossline, "unexpected end-of-line in quoted token" );
		break;
	case eWhitespace:
	case eCharToken:
	case eCharSolidus:
	case eCharStar:
	case eCharSpecial:
		add( c );
		break;
	case eCharQuote:
		pop();
		push( Tokenise( &ScriptTokeniser::tokeniseEndQuote ) );
		break;
	default:
		break;
	}
	return false;
}
bool tokeniseSolidus( char c ){
	switch ( charType( c ) )
	{
	case eNewline:
	case eWhitespace:
	case eCharQuote:
	case eCharSpecial:
		pop();
		add( '/' );
		return true; // emit single slash
	case eCharToken:
		pop();
		add( '/' );
		add( c );
		break;
	case eCharSolidus:
		pop();
		push( Tokenise( &ScriptTokeniser::tokeniseComment ) );
		break; // dont emit single slash
	case eCharStar:
		pop();
		push( Tokenise( &ScriptTokeniser::tokeniseBlockComment ) );
		break; // dont emit single slash
	default:
		break;
	}
	return false;
}
bool tokeniseComment( char c ){
	if ( c == '\n' ) {
		pop();
		if ( state() == Tokenise( &ScriptTokeniser::tokeniseToken ) ) {
			pop();
			return true; // emit token immediatly preceding comment
		}
	}
	return false;
}
bool tokeniseBlockComment( char c ){
	if ( c == '*' ) {
		pop();
		push( Tokenise( &ScriptTokeniser::tokeniseEndBlockComment ) );
	}
	return false;
}
bool tokeniseEndBlockComment( char c ){
	switch ( c )
	{
	case '/':
		pop();
		if ( state() == Tokenise( &ScriptTokeniser::tokeniseToken ) ) {
			pop();
			return true; // emit token immediatly preceding comment
		}
		break; // dont emit comment
	case '*':
		break; // no state change
	default:
		pop();
		push( Tokenise( &ScriptTokeniser::tokeniseBlockComment ) );
		break;
	}
	return false;
}
bool tokeniseEndQuote( char c ){
	pop();
	return true; // emit quoted token
}
bool tokeniseSpecial( char c ){
	pop();
	return true; // emit single-character token
}

void tokenise(){
	m_write = m_token;
	while ( !eof() )
	{
		char c = m_current;

		if ( c == '\n' ) {
			++m_scriptline;
		}

		if ( ( ( *this ).*state() )( c ) ) {
			return;
		}

		m_eof = !m_istream.readChar( m_current );
	}
}

const char* fillToken(){
	if ( eof() ) {
		return 0;
	}

	tokenise();

	if ( eof() && m_write == m_token ) {
		return 0;
	}

	add( '\0' );
	return m_token;
}

bool eof(){
	return m_eof;
}

public:
ScriptTokeniser( TextInputStream& istream, bool special )
	: m_state( m_stack ),
	m_istream( istream ),
	m_scriptline( 0 ),
	m_crossline( false ),
	m_unget( false ),
	m_special( special ){
	m_stack[0] = Tokenise( &ScriptTokeniser::tokeniseDefault );
	m_eof = !m_istream.readChar( m_current );
	m_token[MAXTOKEN - 1] = '\0';
}
void release(){
	delete this;
}
void nextLine(){
	m_crossline = true;
}
const char* getToken(){
	if ( m_unget ) {
		m_unget = false;
		return m_token;
	}

	return fillToken();
}
void ungetToken(){
	ASSERT_MESSAGE( !m_unget, "can't unget more than one token" );
	m_unget = true;
}
std::size_t getLine() const {
	return m_scriptline;
}
};


inline Tokeniser& NewScriptTokeniser( TextInputStream& istream ){
	return *( new ScriptTokeniser( istream, true ) );
}

inline Tokeniser& NewSimpleTokeniser( TextInputStream& istream ){
	return *( new ScriptTokeniser( istream, false ) );
}

#endif
