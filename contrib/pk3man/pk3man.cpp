//-----------------------------------------------------------------------------
//
// $LogFile$
// $Revision: 1.7 $
// $Author: ttimo $
// $Date: 2002/05/02 14:41:21 $
// $Log: pk3man.cpp,v $
// Revision 1.7  2002/05/02 14:41:21  ttimo
// fixes for 1.2.7
//
// Revision 1.6  2002/01/27 22:04:06  ttimo
// fixes for linux ver
//
// Revision 1.5  2002/01/21 18:52:24  ttimo
// fixes for 1.2 compile
//
// Revision 1.4  2001/01/24 22:37:19  mickey
// Fixed projects for new radiant layout,
// Changes all file/dir dialogs and messageboxs to use radiant interface.
//
// Revision 1.3  2000/12/22 16:53:28  mickey
// fixed to support TA
//
// Revision 1.2  2000/11/01 14:05:01  lmz
// Linux fixes
//
// Revision 1.1.1.1  2000/10/03 18:36:26  mickey
// initial import
//
// Revision 1.3  2000/10/02 10:22:44  mickey
// added rename dialog and some sanity checks for unsaved/modifed pk3's
//
// Revision 1.2  2000/10/01 20:35:40  mickey
// beta stage
//
// Revision 1.1.1.1  2000/09/26 01:19:38  mickey
// initial import of Gtk module
//
//
// DESCRIPTION:
// main plugin implementation
// texturing tools for Q3Radiant
//

#include "stdafx.h"
#include "mainwnd.h"

// plugin interfaces
static bool g_bShaderTableInitDone = false;
_QERShadersTable g_ShadersTable;

// Radiant function table
_QERFuncTable_1 g_FuncTable;

bool g_bEpairInitDone = false;
#if 0 //mattn
_QEREpairsTable         g_EpairTable;
#endif

// plugin name
const char *PLUGIN_NAME = "Pk3Man";

// commands in the menu
static const char *PLUGIN_COMMANDS = "About...;-;Go...";

// globals
GtkWidget *g_pRadiantWnd=NULL;
CMainWnd *g_pMainWnd=NULL;

CString g_strAppPath;
CString g_strBitmapsPath;
CString g_strBasePath;
CString g_strTempPath;
CString g_strModPath;              // are we in TeamArena mode?

#include "version.h"
static const char *PLUGIN_ABOUT = "Pk3Man for Q3Radiant\n"
		"http://pk3man.sourceforge.net\n\n"
		"Gtk & Original Version\n"
		"by Mike Jackman (mickey@planetquake.com)\n\n"
		"Built against GtkRadiant " RADIANT_VERSION "\n"
		__DATE__;

extern "C" LPVOID WINAPI QERPlug_GetFuncTable()
{
	return &g_FuncTable;
}

extern "C" LPCSTR WINAPI QERPlug_Init(HMODULE hApp, GtkWidget* pMainWidget)
{
	g_pRadiantWnd = pMainWidget;
	memset(&g_FuncTable, 0, sizeof(_QERFuncTable_1));
	g_FuncTable.m_nSize = sizeof(_QERFuncTable_1);

	return "Pk3Man for GTKRadiant";
}

extern "C" LPCSTR WINAPI QERPlug_GetName()
{
	return (char*)PLUGIN_NAME;
}

extern "C" LPCSTR WINAPI QERPlug_GetCommandList()
{
	return (char*)PLUGIN_COMMANDS;
}

char *TranslateString (char *buf)
{
	static	char	buf2[32768];
	int		i, l;
	char	*out;

	l = strlen(buf);
	out = buf2;
	for (i=0 ; i<l ; i++)
	{
		if (buf[i] == '\n')
		{
			*out++ = '\r';
			*out++ = '\n';
		}
		else
			*out++ = buf[i];
	}

	*out++ = 0;
	return buf2;
}

/*
extern "C" void Sys_Printf (char *text, ...)
{
	va_list argptr;
	char	buf[32768];
	char	*out;

	va_start (argptr,text);
	vsprintf (buf, text,argptr);
	va_end (argptr);

	//out = TranslateString (buf);

	g_FuncTable.m_pfnSysMsg ( buf );
}
*/

extern "C" void WINAPI QERPlug_Dispatch (LPCSTR p, vec3_t vMin, vec3_t vMax, bool bSingleBrush)
{
	if (!g_bShaderTableInitDone)
	{
		g_ShadersTable.m_nSize = sizeof (_QERShadersTable);
		if ( g_FuncTable.m_pfnRequestInterface( QERShadersTable_GUID, static_cast<LPVOID>(&g_ShadersTable) ) )
		{
		g_bShaderTableInitDone = true;
		}
		else
		{
		Sys_Printf("Pk3Man plugin: _QERShadersTable interface request failed\n");
		return;
		}
	}

#if 0 //mattn
	if (!g_bEpairInitDone)
	{
		g_EpairTable.m_nSize = sizeof(_QEREpairsTable);
		if ( g_FuncTable.m_pfnRequestInterface( QEREpairsTable_GUID, static_cast<LPVOID>(&g_EpairTable) ) )
			g_bEpairInitDone = true;
		else
		{
			Sys_Printf("Pk3Man plugin: _QEREpairsTable interface request failed\n");
			return;
		}
	}
#endif

	if (!strcmp(p, "About..."))
	{
		g_FuncTable.m_pfnMessageBox(g_pRadiantWnd,PLUGIN_ABOUT,"About ...",MB_OK, NULL);
	}
	else if (!strcmp(p, "Go..."))
	{
		Sys_Printf ("Pk3Man starting ...\n");
		g_pMainWnd = new CMainWnd ();
		g_pMainWnd->Create (g_pRadiantWnd);
	}
}
