// Wild.cpp: implementation of the CWild class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "pakman.h"
#include "wild.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWild::CWild()
{

}

CWild::~CWild()
{

}

int CWild::fpattern_isvalid(const char *pat)
{
	int		len;

	/* Check args */
	if (pat == NULL)
		return (FALSE);

	/* Verify that the pattern is valid */
	for (len = 0;  pat[len] != '\0';  len++)
	{
		switch (pat[len])
		{
		case FPAT_SET_L:
			/* Char set */
			len++;
			if (pat[len] == FPAT_SET_NOT)
				len++;			/* Set negation */

			while (pat[len] != FPAT_SET_R)
			{
				if (pat[len] == QUOTE)
					len++;		/* Quoted char */
				if (pat[len] == '\0')
					return (FALSE);	/* Missing closing bracket */
				len++;

				if (pat[len] == FPAT_SET_THRU)
				{
					/* Char range */
					len++;
					if (pat[len] == QUOTE)
						len++;		/* Quoted char */
					if (pat[len] == '\0')
						return (FALSE);	/* Missing closing bracket */
					len++;
				}

				if (pat[len] == '\0')
					return (FALSE);	/* Missing closing bracket */
			}
			break;

		case QUOTE:
			/* Quoted char */
			len++;
			if (pat[len] == '\0')
				return (FALSE);		/* Missing quoted char */
			break;

		case FPAT_NOT:
			/* Negated pattern */
			len++;
			if (pat[len] == '\0')
				return (FALSE);		/* Missing subpattern */
			break;

		default:
			/* Valid character */
			break;
		}
	}

	return (TRUE);
}


int CWild::fpattern_submatch(const char *pat, const char *fname)
{
	int		fch;
	int		pch;
	int		i;
	int		yes, match;
	int		lo, hi;

	/* Attempt to match subpattern against subfilename */
	while (*pat != '\0')
	{
		fch = *fname;
		pch = *pat;
		pat++;

		switch (pch)
		{
		case FPAT_ANY:
			/* Match a single char */
			if (fch == DEL  ||  fch == DEL2  ||  fch == '\0')
				return (FALSE);
			fname++;
			break;

		case FPAT_CLOS:
			/* Match zero or more chars */
			i = 0;
			while (fname[i] != '\0')//  && fname[i] != DEL  &&  fname[i] != DEL2)
				i++;

			while (i >= 0)
			{
				if (fpattern_submatch(pat, fname+i))
					return (TRUE);
				i--;
			}
			return (FALSE);

		case SUB:
			/* Match zero or more chars */
			i = 0;
			while (fname[i] != '\0'  && fname[i] != DEL  &&  fname[i] != DEL2  && fname[i] != '.')
				i++;
			while (i >= 0)
			{
				if (fpattern_submatch(pat, fname+i))
					return (TRUE);
				i--;
			}
			return (FALSE);

		case QUOTE:
			/* Match a quoted char */
			pch = *pat;
			if (lowercase(fch) != lowercase(pch)  ||  pch == '\0')
				return (FALSE);
			fname++;
			pat++;
			break;

		case FPAT_SET_L:
			/* Match char set/range */
			yes = TRUE;
			if (*pat == FPAT_SET_NOT)
			{
			pat++;
			yes = FALSE;	/* Set negation */
			}

			/* Look for [s], [-], [abc], [a-c] */
			match = !yes;
			while (*pat != FPAT_SET_R  &&  *pat != '\0')
			{
				if (*pat == QUOTE)
					pat++;	/* Quoted char */

				if (*pat == '\0')
					break;
				lo = *pat++;
				hi = lo;

				if (*pat == FPAT_SET_THRU)
				{
					/* Range */
					pat++;

					if (*pat == QUOTE)
						pat++;	/* Quoted char */

					if (*pat == '\0')
						break;
					hi = *pat++;
				}

				if (*pat == '\0')
					break;

				/* Compare character to set range */
				if (lowercase(fch) >= lowercase(lo)  &&
					lowercase(fch) <= lowercase(hi))
					match = yes;
			}

			if (!match)
				return (FALSE);

			if (*pat == '\0')
				return (FALSE);		/* Missing closing bracket */

			fname++;
			pat++;
			break;

		case FPAT_NOT:
			/* Match only if rest of pattern does not match */
			if (*pat == '\0')
				return (FALSE);		/* Missing subpattern */
			i = fpattern_submatch(pat, fname);
			return !i;


		case DEL:
		case DEL2:

			/* Match path delimiter char */
			if (fch != DEL  &&  fch != DEL2)
				return (FALSE);
			fname++;
			break;

		default:
			/* Match a (non-null) char exactly */
			if (lowercase(fch) != lowercase(pch))
				return (FALSE);
			fname++;
			break;
		}
	}

	/* Check for complete match */
	if (*fname != '\0')
		return (FALSE);

	/* Successful match */
	return (TRUE);
}


/*-----------------------------------------------------------------------------
* fpattern_match()
*	Attempts to match pattern 'pat' to filename 'fname'.
*
* Returns
*	1 (true) if the filename matches, otherwise 0 (false).
*
* Caveats
*	If 'fname' is null, zero (false) is returned.
*
*	If 'pat' is null, zero (false) is returned.
*
*	If 'pat' is empty (""), the only filename it matches is the empty
*	string ("").
*
*	If 'fname' is empty, the only pattern that will match it is the empty
*	string ("").
*
*	If 'pat' is not a well-formed pattern, zero (false) is returned.
*
*	Upper and lower case letters are treated the same; alphabetic
*	characters are converted to lower case before matching occurs.
*	Conversion to lower case is dependent upon the current locale setting.
*/

int CWild::fpattern_match(const char *pat, const char *fname)
{
	int		rc;

	/* Check args */
	if (fname == NULL)
		return (FALSE);

	if (pat == NULL)
		return (FALSE);

	/* Verify that the pattern is valid, and get its length */
	if (!fpattern_isvalid(pat))
		return (FALSE);

	/* Attempt to match pattern against filename */
	if (fname[0] == '\0')
		return (pat[0] == '\0');	/* Special case */
	rc = fpattern_submatch(pat, fname);

	return (rc);
}


/*-----------------------------------------------------------------------------
* fpattern_matchn()
*	Attempts to match pattern 'pat' to filename 'fname'.
*	This operates like fpattern_match() except that it does not verify that
*	pattern 'pat' is well-formed, assuming that it has been checked by a
*	prior call to fpattern_isvalid().
*
* Returns
*	1 (true) if the filename matches, otherwise 0 (false).
*
* Caveats
*	If 'fname' is null, zero (false) is returned.
*
*	If 'pat' is null, zero (false) is returned.
*
*	If 'pat' is empty (""), the only filename it matches is the empty ("")
*	string.
*
*	If 'pat' is not a well-formed pattern, unpredictable results may occur.
*
*	Upper and lower case letters are treated the same; alphabetic
*	characters are converted to lower case before matching occurs.
*	Conversion to lower case is dependent upon the current locale setting.
*
* See also
*	fpattern_match().
*/

int CWild::fpattern_matchn(const char *pat, const char *fname)
{
	int		rc;

	/* Check args */
	if (fname == NULL)
		return (FALSE);

	if (pat == NULL)
		return (FALSE);

	/* Assume that pattern is well-formed */

	/* Attempt to match pattern against filename */
	rc = fpattern_submatch(pat, fname);

	return (rc);
}
