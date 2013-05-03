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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// all purpose scriplib interface for Q3Radiant plugins (cf. parse.h)
//

#ifndef __ISCRIPLIB_H_
#define __ISCRIPLIB_H_

/*! \file iscriplib.h
   \brief function tables for Radiant core's text parsing functions
   two token based parsers cohexist in Radiant
   the primary one (GetToken UnGetToken etc.) is used on the .map parsing etc.
   COM_Parse is another parser, used on .def parse for instance

   NOTE: I hope we can totally get rid of this part when we have XML support
 */

#define SCRIPLIB_MAJOR "scriptlib"

typedef qboolean ( *PFN_GETTOKEN )( qboolean crossline );
typedef void ( *PFN_UNGETTOKEN )();
// only used to retrieve &token
typedef char*     ( *PFN_TOKEN )();
typedef void ( *PFN_STARTTOKENPARSING )( char * );
// script line
typedef int ( *PFN_SCRIPTLINE )();
typedef qboolean ( *PFN_TOKENAVAILABLE )();
// COM_Parse
typedef char*     ( *PFN_COM_PARSE )( char *data );
typedef char*     ( *PFN_GET_COM_TOKEN )();
// Hydra: added support for GetTokenExtra()
typedef qboolean ( *PFN_GETTOKENEXTRA )( qboolean crossline,const char *delimiters,qboolean keepdelimiter );

struct _QERScripLibTable
{
	float m_fVersion;
	int m_nSize;
	PFN_GETTOKEN m_pfnGetToken;
	PFN_GETTOKENEXTRA m_pfnGetTokenExtra;   // Hydra: added support for GetTokenExtra()
	PFN_UNGETTOKEN m_pfnUnGetToken;
	PFN_TOKEN m_pfnToken;
	PFN_STARTTOKENPARSING m_pfnStartTokenParsing;
	PFN_SCRIPTLINE m_pfnScriptLine;
	PFN_TOKENAVAILABLE m_pfnTokenAvailable;
	PFN_COM_PARSE m_pfnCOM_Parse;
	PFN_GET_COM_TOKEN m_pfnGet_COM_Token;
};

#ifdef USE_SCRIPLIBTABLE_DEFINE
#ifndef __SCRIPLIBTABLENAME
#define __SCRIPLIBTABLENAME g_ScripLibTable
#endif
#define GetToken __SCRIPLIBTABLENAME.m_pfnGetToken
#define Token __SCRIPLIBTABLENAME.m_pfnToken
#define UnGetToken __SCRIPLIBTABLENAME.m_pfnUnGetToken
#define StartTokenParsing __SCRIPLIBTABLENAME.m_pfnStartTokenParsing
#define ScriptLine __SCRIPLIBTABLENAME.m_pfnScriptLine
#define TokenAvailable __SCRIPLIBTABLENAME.m_pfnTokenAvailable
#define COM_Parse __SCRIPLIBTABLENAME.m_pfnCOM_Parse
#define Get_COM_Token __SCRIPLIBTABLENAME.m_pfnGet_COM_Token
#define GetTokenExtra __SCRIPLIBTABLENAME.m_pfnGetTokenExtra // Hydra: added support for GetTokenExtra()
#endif

#endif
