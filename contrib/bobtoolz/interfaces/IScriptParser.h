#ifndef _ISCRIPTPARSER_H_
#define _ISCRIPTPARSER_H_

class IScriptParser {
public:
virtual ~IScriptParser() {};

virtual const char* GetToken( bool ) = 0;
virtual char*       GetBufferCopy( void ) = 0;
virtual int         GetTokenOffset( void ) = 0;

virtual void SkipBracedSection( void ) = 0;
virtual void SkipRestOfLine( void ) = 0;
virtual void UndoGetToken( void ) = 0;
virtual void ResetParseSession( void ) = 0;

virtual void LoadScript( const char*   ) = 0;
virtual void SetScript( char*         ) = 0;

virtual void AddBreakChar( char ) = 0;
};

#endif
