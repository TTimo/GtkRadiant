/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "StdAfx.h"

#include "funchandlers.h"
#include "misc.h"

#include "dialogs/dialogs-gtk.h"

// Radiant function table
_QERFuncTable_1 g_FuncTable;
_QERAppBSPFrontendTable g_BSPTable;             // for map name

BOOL g_bBSPInitDone                 = FALSE;

// plugin name
static const char *PLUGIN_NAME = "ctfToolz";

// commands in the menu
static const char *PLUGIN_COMMANDS = "About...,Colour Changer...,Swap Light Colours,Change Angles 180,Swap Spawn Points";

// globals
GtkWidget *g_pRadiantWnd = NULL;

static const char *PLUGIN_ABOUT = "ctfToolz for GtkRadiant\n"
								  "by djbob\n"
								  "http://www.planetquake.com/toolz\n\n";

extern "C" LPVOID WINAPI QERPlug_GetFuncTable(){
	return &g_FuncTable;
}

extern "C" LPCSTR WINAPI QERPlug_Init( HMODULE hApp, GtkWidget* pMainWidget ){
	g_pRadiantWnd = pMainWidget;
	memset( &g_FuncTable, 0, sizeof( _QERFuncTable_1 ) );
	g_FuncTable.m_fVersion = QER_PLUG_VERSION;
	g_FuncTable.m_nSize = sizeof( _QERFuncTable_1 );

	return "ctfToolz for GTKradiant";
}

extern "C" LPCSTR WINAPI QERPlug_GetName(){
	return (char*)PLUGIN_NAME;
}

extern "C" LPCSTR WINAPI QERPlug_GetCommandList(){
	return (char*)PLUGIN_COMMANDS;
}

extern "C" void WINAPI QERPlug_Dispatch( LPCSTR p, vec3_t vMin, vec3_t vMax, bool bSingleBrush ){
	LoadLists();

	if ( !g_bBSPInitDone ) {
		g_BSPTable.m_nSize = sizeof( _QERAppBSPFrontendTable );
		if ( g_FuncTable.m_pfnRequestInterface( QERAppBSPFrontendTable_GUID, static_cast<LPVOID>( &g_BSPTable ) ) ) {
			g_bBSPInitDone = TRUE;
		}
		else
		{
			Sys_ERROR( "_QERAppBSPFrontendTable interface request failed\n" );
			return;
		}
	}

	if ( !strcmp( p, "About..." ) ) {
		DoMessageBox( PLUGIN_ABOUT, "About", IDOK );
	}
	else if ( !strcmp( p, "Colour Changer..." ) ) {
		DoCTFColourChanger();
	}
	else if ( !strcmp( p, "Swap Light Colours" ) ) {
		DoSwapLights();
	}
	else if ( !strcmp( p, "Change Angles 180" ) ) {
		DoChangeAngles();
	}
	else if ( !strcmp( p, "Swap Spawn Points" ) ) {
		DoSwapSpawns();
	}
}
