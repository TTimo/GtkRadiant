/*
   PrtView plugin for GtkRadiant
   Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

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

// PrtView.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#define Q3R_CMD_SPLITTER "-"
#define Q3R_CMD_ABOUT "About"
#define Q3R_CMD_LOAD "Load .prt file"
#define Q3R_CMD_RELEASE "Unload .prt file"
#define Q3R_CMD_SHOW_3D "Toggle portals (3D)"
#define Q3R_CMD_SHOW_2D "Toggle portals (2D)"
#define Q3R_CMD_OPTIONS "Configure Portal Viewer..."

static char INIfn[NAME_MAX];

/////////////////////////////////////////////////////////////////////////////
// CPrtViewApp construction

#define RENDER_2D "Render2D"
#define WIDTH_2D "Width2D"
#define AA_2D "AntiAlias2D"
#define COLOR_2D "Color2D"

#define RENDER_3D "Render3D"
#define WIDTH_3D "Width3D"
#define AA_3D "AntiAlias3D"
#define COLOR_3D "Color3D"
#define COLOR_FOG "ColorFog"
#define FOG "Fog"
#define ZBUFFER "ZBuffer"
#define POLYGON "Polygons"
#define LINE "Lines"
#define TRANS_3D "Transparency"
#define CLIP_RANGE "ClipRange"
#define CLIP "Clip"

void *g_pMainWidget = NULL;

void InitInstance(){
#ifdef _WIN32
	char fn[_MAX_PATH];
	char fn_drive[_MAX_DRIVE];
	char fn_dir[_MAX_DIR];
	char fn_name[_MAX_FNAME];
	char fn_ext[_MAX_EXT];

	GetModuleFileName( GetModuleHandle( "PrtView.dll" ), fn, _MAX_PATH );

	_splitpath( fn, fn_drive, fn_dir, fn_name, fn_ext );

	strcpy( INIfn, fn_drive );
	strcat( INIfn, fn_dir );
	strcat( INIfn, fn_name );
	strcat( INIfn, ".ini" );
#else // if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	strcpy( INIfn, g_get_home_dir() );
	strcat( INIfn, "/.radiant/" );
	strcat( INIfn, RADIANT_VERSION );
	strcat( INIfn, "/prtview.ini" );
#endif

	portals.show_2d = INIGetInt( RENDER_2D, FALSE ) ? true : false;
	portals.aa_2d = INIGetInt( AA_2D, FALSE ) ? true : false;
	portals.width_2d = (float)INIGetInt( WIDTH_2D, 10 );
	portals.color_2d = (COLORREF)INIGetInt( COLOR_2D, RGB( 0, 0, 255 ) ) & 0xFFFFFF;

	if ( portals.width_2d > 40.0f ) {
		portals.width_2d = 40.0f;
	}
	else if ( portals.width_2d < 2.0f ) {
		portals.width_2d = 2.0f;
	}

	portals.show_3d = INIGetInt( RENDER_3D, TRUE ) ? true : false;

	portals.zbuffer = INIGetInt( ZBUFFER, 1 );
	portals.fog = INIGetInt( FOG, FALSE ) ? true : false;
	portals.polygons = INIGetInt( POLYGON, TRUE );
	portals.lines = INIGetInt( LINE, TRUE );
	portals.aa_3d = INIGetInt( AA_3D, FALSE ) ? true : false;
	portals.width_3d = (float)INIGetInt( WIDTH_3D, 4 );
	portals.color_3d = (COLORREF)INIGetInt( COLOR_3D, RGB( 255, 255, 0 ) ) & 0xFFFFFF;
	portals.color_fog = (COLORREF)INIGetInt( COLOR_FOG, RGB( 127, 127, 127 ) ) & 0xFFFFFF;
	portals.trans_3d = (float)INIGetInt( TRANS_3D, 50 );
	portals.clip = INIGetInt( CLIP, FALSE ) ? true : false;
	portals.clip_range = (float)INIGetInt( CLIP_RANGE, 16 );

	if ( portals.clip_range < 1 ) {
		portals.clip_range = 1;
	}
	else if ( portals.clip_range > 128 ) {
		portals.clip_range = 128;
	}

	if ( portals.zbuffer < 0 ) {
		portals.zbuffer = 0;
	}
	else if ( portals.zbuffer > 2 ) {
		portals.zbuffer = 0;
	}

	if ( portals.width_3d > 40.0f ) {
		portals.width_3d = 40.0f;
	}
	else if ( portals.width_3d < 2.0f ) {
		portals.width_3d = 2.0f;
	}

	if ( portals.trans_3d > 100.0f ) {
		portals.trans_3d = 100.0f;
	}
	else if ( portals.trans_3d < 0.0f ) {
		portals.trans_3d = 0.0f;
	}

	SaveConfig();

	portals.FixColors();
}

void SaveConfig(){
	INISetInt( RENDER_2D, portals.show_2d, "Draw in 2D windows" );
	INISetInt( WIDTH_2D, (int)portals.width_2d, "Width of lines in 2D windows (in units of 1/2)" );
	INISetInt( COLOR_2D, (int)portals.color_2d, "Color of lines in 2D windows" );
	INISetInt( AA_2D, portals.aa_2d, "Draw lines in 2D window anti-aliased" );

	INISetInt( ZBUFFER, portals.zbuffer, "ZBuffer level in 3D window" );
	INISetInt( FOG, portals.fog, "Use depth cueing in 3D window" );
	INISetInt( POLYGON, portals.polygons, "Render using polygons polygons in 3D window" );
	INISetInt( LINE, portals.polygons, "Render using lines in 3D window" );
	INISetInt( RENDER_3D, portals.show_3d, "Draw in 3D windows" );
	INISetInt( WIDTH_3D, (int)portals.width_3d, "Width of lines in 3D window (in units of 1/2)" );
	INISetInt( COLOR_3D, (int)portals.color_3d, "Color of lines/polygons in 3D window" );
	INISetInt( COLOR_FOG, (int)portals.color_fog, "Color of distant lines/polygons in 3D window" );
	INISetInt( AA_3D, portals.aa_3d, "Draw lines in 3D window anti-aliased" );
	INISetInt( TRANS_3D, (int)portals.trans_3d, "Transparency in 3d view (0 = solid, 100 = invisible)" );
	INISetInt( CLIP, portals.clip, "Cubic clipper active for portal viewer" );
	INISetInt( CLIP_RANGE, (int)portals.clip_range, "Portal viewer cubic clip distance (in units of 64)" );
}

// Radiant function table
// use to access what Radiant provides
_QERFuncTable_1 g_FuncTable;
_QERQglTable g_QglTable;

#define CONFIG_SECTION "Configuration"

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

static bool read_var( const char *filename, const char *section, const char *key, char *value ){
	char line[1024], *ptr;
	FILE *rc;

	rc = fopen( filename, "rt" );

	if ( rc == NULL ) {
		return false;
	}

	while ( fgets( line, 1024, rc ) != 0 )
	{
		// First we find the section
		if ( line[0] != '[' ) {
			continue;
		}

		ptr = strchr( line, ']' );
		*ptr = '\0';

		if ( strcmp( &line[1], section ) == 0 ) {
			while ( fgets( line, 1024, rc ) != 0 )
			{
				ptr = strchr( line, '=' );

				if ( ptr == NULL ) {
					// reached the end of the section
					fclose( rc );
					return false;
				}
				*ptr = '\0';

				if ( strcmp( line, key ) == 0 ) {
					strcpy( value, ptr + 1 );
					fclose( rc );

					while ( value[strlen( value ) - 1] == 10 ||
							value[strlen( value ) - 1] == 13 ||
							value[strlen( value ) - 1] == 32 )
						value[strlen( value ) - 1] = 0;
					return true;
				}
			}
		}
	}

	fclose( rc );
	return false;
}

static bool save_var( const char *filename, const char *section, const char *key, const char *value ){
	char line[1024], *ptr;
	FILE *old_rc = NULL, *rc;
	bool found;

	rc = fopen( filename, "rb" );

	if ( rc != NULL ) {
		guint32 len;
		void *buf;

		char *tmpname = g_strdup_printf( "%s.tmp", filename );
		old_rc = fopen( tmpname, "w+b" );
		g_free( tmpname );

		fseek( rc, 0, SEEK_END );
		len = ftell( rc );
		rewind( rc );
		buf = g_malloc( len );
		fread( buf, len, 1, rc );
		fwrite( buf, len, 1, old_rc );
		g_free( buf );
		fclose( rc );
		rewind( old_rc );
	}

	rc = fopen( filename, "wb" );

	if ( rc == NULL ) {
		return false;
	}

	// First we need to find the section
	found = false;
	if ( old_rc != NULL ) {
		while ( fgets( line, 1024, old_rc ) != NULL )
		{
			fputs( line, rc );

			if ( line[0] == '[' ) {
				ptr = strchr( line, ']' );
				*ptr = '\0';

				if ( strcmp( &line[1], section ) == 0 ) {
					found = true;
					break;
				}
			}
		}
	}

	if ( !found ) {
		fputs( "\n", rc );
		fprintf( rc, "[%s]\n", section );
	}

	fprintf( rc, "%s=%s\n", key, value );

	if ( old_rc != NULL ) {
		while ( fgets( line, 1024, old_rc ) != NULL )
		{
			ptr = strchr( line, '=' );

			if ( ptr != NULL ) {
				*ptr = '\0';

				if ( strcmp( line, key ) == 0 ) {
					break;
				}

				*ptr = '=';
				fputs( line, rc );
			}
			else
			{
				fputs( line, rc );
				break;
			}
		}

		while ( fgets( line, 1024, old_rc ) != NULL )
			fputs( line, rc );

		fclose( old_rc );

		char *tmpname = g_strdup_printf( "%s.tmp", filename );
		remove( tmpname );
		g_free( tmpname );
	}

	fclose( rc );

	return true;
}

#endif

int INIGetInt( const char *key, int def ){
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	char value[1024];

	if ( read_var( INIfn, CONFIG_SECTION, key, value ) ) {
		return atoi( value );
	}
	else{
		return def;
	}
#else
	return GetPrivateProfileInt( CONFIG_SECTION, key, def, INIfn );
#endif
}

void INISetInt( const char *key, int val, const char *comment /* = NULL */ ){
	char s[1000];

	if ( comment ) {
		sprintf( s, "%d        ; %s", val, comment );
	}
	else{
		sprintf( s, "%d", val );
	}
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	save_var( INIfn, CONFIG_SECTION, key, s );
#else
	WritePrivateProfileString( CONFIG_SECTION, key, s, INIfn );
#endif
}


// plugin name
static const char *PLUGIN_NAME = "Portal Viewer";
// commands in the menu
static const char *PLUGIN_COMMANDS =
	Q3R_CMD_ABOUT ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_OPTIONS ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_SHOW_2D ";"
	Q3R_CMD_SHOW_3D ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_RELEASE ";"
	Q3R_CMD_LOAD;

extern "C" LPVOID WINAPI QERPlug_GetFuncTable(){
	return &g_FuncTable;
}


//extern "C" LPCSTR WINAPI QERPlug_Init (HMODULE hApp, GtkWidget* hwndMain)
extern "C" const char* QERPlug_Init( void *hApp, void* pMainWidget ){
	g_pMainWidget = pMainWidget;
	// Setup defaults & load config
	InitInstance();

	return "Portal Viewer for Q3Radiant";
}

extern "C" const char* QERPlug_GetName(){
	return (char*)PLUGIN_NAME;
}

extern "C" const char* QERPlug_GetCommandList(){
	return (char*)PLUGIN_COMMANDS;
}

/*
   void Sys_Printf (char *text, ...)
   {
   va_list argptr;
   char buf[32768];

   va_start (argptr,text);
   vsprintf (buf, text, argptr);
   va_end (argptr);

   g_FuncTable.m_pfnSysMsg (buf);
   }
 */

bool interfaces_started = false;

static void CheckInterfaces(){
	if ( interfaces_started ) {
		return;
	}

	render.Register();

	interfaces_started = true;
}

extern "C" void QERPlug_Dispatch( const char* p, vec3_t vMin, vec3_t vMax, bool bSingleBrush ){
	Sys_Printf( MSG_PREFIX "Command \"%s\"\n",p );

	if ( !strcmp( p,Q3R_CMD_ABOUT ) ) {
		DoAboutDlg( GTK_WIDGET( g_pMainWidget ) );
	}
	else if ( !strcmp( p,Q3R_CMD_LOAD ) ) {
		CheckInterfaces();

		if ( interfaces_started ) {
			if ( DoLoadPortalFileDialog( GTK_WIDGET( g_pMainWidget ) ) == IDOK ) {
				portals.Load();
				g_FuncTable.m_pfnSysUpdateWindows( UPDATE_ALL );
			}
			else
			{
				Sys_Printf( MSG_PREFIX "Portal file load aborted.\n", portals.fn );
			}
		}
	}
	else if ( !strcmp( p,Q3R_CMD_RELEASE ) ) {
		portals.Purge();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_ALL );
		}

		Sys_Printf( MSG_PREFIX "Portals unloaded.\n" );
	}
	else if ( !strcmp( p,Q3R_CMD_SHOW_2D ) ) {
		portals.show_2d = !portals.show_2d;

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_ALL );
		}
		SaveConfig();

		if ( portals.show_2d ) {
			Sys_Printf( MSG_PREFIX "Portals will be rendered in 2D view.\n" );
		}
		else{
			Sys_Printf( MSG_PREFIX "Portals will NOT be rendered in 2D view.\n" );
		}
	}
	else if ( !strcmp( p,Q3R_CMD_SHOW_3D ) ) {
		portals.show_3d = !portals.show_3d;
		SaveConfig();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_ALL );
		}

		if ( portals.show_3d ) {
			Sys_Printf( MSG_PREFIX "Portals will be rendered in 3D view.\n" );
		}
		else{
			Sys_Printf( MSG_PREFIX "Portals will NOT be rendered in 3D view.\n" );
		}
	}
	else if ( !strcmp( p,Q3R_CMD_OPTIONS ) ) {
		DoConfigDialog( GTK_WIDGET( g_pMainWidget ) );
		SaveConfig();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_ALL );
		}
	}
}



// =============================================================================
// SYNAPSE

class CSynapseClientPrtView : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientPrtView() { }
virtual ~CSynapseClientPrtView() { }
};


CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientPrtView g_SynapseClient;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" CSynapseClient * SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces( const char *version, CSynapseServer *pServer ) {
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
	if ( strcmp( version, SYNAPSE_VERSION ) ) {
		Syn_Printf( "ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version );
		return NULL;
	}
	g_pSynapseServer = pServer;
	g_pSynapseServer->IncRef();
	Set_Syn_Printf( g_pSynapseServer->Get_Syn_Printf() );

	g_SynapseClient.AddAPI( PLUGIN_MAJOR, PRTVIEW_MINOR, sizeof( _QERPluginTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( g_QglTable ), SYN_REQUIRE, &g_QglTable );

	return &g_SynapseClient;
}

bool CSynapseClientPrtView::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, PLUGIN_MAJOR ) ) {
		if ( !strcmp( pAPI->minor_name, PRTVIEW_MINOR ) ) {
			_QERPluginTable* pTable = static_cast<_QERPluginTable*>( pAPI->mpTable );

			pTable->m_pfnQERPlug_Init = QERPlug_Init;
			pTable->m_pfnQERPlug_GetName = QERPlug_GetName;
			pTable->m_pfnQERPlug_GetCommandList = QERPlug_GetCommandList;
			pTable->m_pfnQERPlug_Dispatch = QERPlug_Dispatch;
			return true;
		}
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientPrtView::GetInfo(){
	return "PrtView module built " __DATE__ " " RADIANT_VERSION;
}
