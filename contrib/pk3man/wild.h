// Wild.h: interface for the CWild class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WILD_H__DC88A1E2_A432_40DE_A5B2_D8EF34272EFC__INCLUDED_)
#define AFX_WILD_H__DC88A1E2_A432_40DE_A5B2_D8EF34272EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FPAT_QUOTE	'\\'		/* Quotes a special char	*/
#define FPAT_QUOTE2	'`'		/* Quotes a special char	*/
#define FPAT_DEL	'/'		/* Path delimiter		*/
#define FPAT_DEL2	'\\'		/* Path delimiter		*/
#define FPAT_DOT	'.'		/* Dot char			*/
#define FPAT_NOT	'!'		/* Exclusion			*/
#define FPAT_ANY	'?'		/* Any one char			*/
#define FPAT_CLOS	'*'		/* Zero or more chars		*/
#define FPAT_CLOSP	'\x1A'		/* Zero or more nondelimiters	*/
#define FPAT_SET_L	'['		/* Set/range open bracket	*/
#define FPAT_SET_R	']'		/* Set/range close bracket	*/
#define FPAT_SET_NOT	'!'		/* Set exclusion		*/
#define FPAT_SET_THRU	'-'		/* Set range of chars		*/

#define DEL			FPAT_DEL
#define DEL2		FPAT_DEL2
#define QUOTE		FPAT_QUOTE2
#define SUB			FPAT_CLOSP

/* Local function macros */
#define lowercase(c)	tolower(c)

class CWild  
{
public:
	CWild();
	virtual ~CWild();

	static int	fpattern_submatch(const char *pat, const char *fname);
	static int	fpattern_isvalid(const char *pat);
	static int	fpattern_match(const char *pat, const char *fname);
	static int	fpattern_matchn(const char *pat, const char *fname);

};

#endif // !defined(AFX_WILD_H__DC88A1E2_A432_40DE_A5B2_D8EF34272EFC__INCLUDED_)
