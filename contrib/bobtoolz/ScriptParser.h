
#ifndef _SCRIPTPARSER_H_
#define _SCRIPTPARSER_H_

#include "interfaces/IScriptParser.h"

#define SP_MAX_BREAKCHARS 16

class CScriptParser: public IScriptParser {
public:
	CScriptParser(void);
	~CScriptParser(void);
private:
	char	m_breakChars[SP_MAX_BREAKCHARS];
	char*	m_pScript;
	char*	m_pScriptSection;
	char*	m_pLastScriptSection;
	char*	m_pToken;

	void SkipWhitespace(bool* pbNewLines);
	void ClearBuffer(void);
	const char* MakeToken(const char* pToken);
	bool IsBreakChar(char c);
public:
	const char* GetToken(bool bAllowLinebreaks);
	void SkipBracedSection(void);
	void SkipRestOfLine(void);
	void UndoGetToken(void);
	void ResetParseSession(void);

	char* GetBufferCopy(void);
	int GetTokenOffset(void);

	void LoadScript(const char* pScript);
	void SetScript(char* pScript);

	void AddBreakChar(char c);
private:
};

#endif
