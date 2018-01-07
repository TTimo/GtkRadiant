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

#include "DEntity.h"
#include "funchandlers.h"

#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#endif

/*==========================
        Global Vars
   ==========================*/

//HANDLE bsp_process;
char g_CurrentTexture[256] = "";

//=============================================================
//=============================================================

void ReadCurrentTexture(){
	const char* textureName = g_FuncTable.m_pfnGetCurrentTexture();
	strcpy( g_CurrentTexture, textureName );
}

const char*  GetCurrentTexture(){
	ReadCurrentTexture();
	return g_CurrentTexture;
}

epair_t* GetNextChainItem( epair_t* lastItem, const char* key, const char* value ){
	epair_t* nextEPair = g_FuncTable.m_pfnAllocateEpair( key, value );

	if ( lastItem != NULL ) {
		lastItem->next = nextEPair;
	}

	return nextEPair;
}

void MoveBlock( int dir, vec3_t min, vec3_t max, float dist ){
	switch ( dir )
	{
	case MOVE_EAST:
	{
		min[0] += dist;
		max[0] += dist;
		break;
	}
	case MOVE_WEST:
	{
		min[0] -= dist;
		max[0] -= dist;
		break;
	}
	case MOVE_NORTH:
	{
		min[1] += dist;
		max[1] += dist;
		break;
	}
	case MOVE_SOUTH:
	{
		min[1] -= dist;
		max[1] -= dist;
		break;
	}
	}
}

void SetInitialStairPos( int dir, vec3_t min, vec3_t max, float width ){
	switch ( dir )
	{
	case MOVE_EAST:
	{
		max[0] = min[0] + width;
		break;
	}
	case MOVE_WEST:
	{
		min[0] = max[0] - width;
		break;
	}
	case MOVE_NORTH:
	{
		max[1] = min[1] + width;
		break;
	}
	case MOVE_SOUTH:
	{
		min[1] = max[1] - width;
		break;
	}
	}
}

char* TranslateString( const char *buf ){
	static char buf2[32768];
	int i, l;
	char    *out;

	l = strlen( buf );
	out = buf2;
	for ( i = 0 ; i < l ; i++ )
	{
		if ( buf[i] == '\n' ) {
			*out++ = '\r';
			*out++ = '\n';
		}
		else{
			*out++ = buf[i];
		}
	}
	*out++ = 0;

	return buf2;
}

void Sys_ERROR( const char* text, ... ){
	va_list argptr;
	char buf[32768];

	va_start( argptr,text );
	vsprintf( buf, text,argptr );
	va_end( argptr );

	Sys_FPrintf( SYS_ERR, "BobToolz::ERROR->%s", buf );
}

/*void Sys_Printf (char *text, ...)
   {
    va_list argptr;
    char	buf[32768];

    va_start (argptr,text);
    vsprintf (buf, text,argptr);
    va_end (argptr);

    g_FuncTable.m_pfnSysMsg ( buf );
   }*/

char* UnixToDosPath( char* path ){
#ifndef _WIN32
	return path;
#else
	for ( char* p = path; *p; p++ )
	{
		if ( *p == '/' ) {
			*p = '\\';
		}
	}
	return path;
#endif
}

const char* ExtractFilename( const char* path ){
	const char* p = strrchr( path, '/' );
	if ( !p ) {
		p = strrchr( path, '\\' );

		if ( !p ) {
			return path;
		}
	}
	return ++p;
}

extern const char* PLUGIN_NAME;
/*char* GetGameFilename(char* buffer, const char* filename)
   {
    strcpy(buffer, g_FuncTable.m_pfnGetGamePath());
    char* p = strrchr(buffer, '/');
   *++p = '\0';
    strcat(buffer, filename);
    buffer = UnixToDosPath(buffer);
    return buffer;
   }*/

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
// the bCreateConsole parameter is ignored on linux ..
bool Q_Exec( const char *pCmd, bool bCreateConsole ){
	switch ( fork() )
	{
	case -1:
		return false;
//      Error ("CreateProcess failed");
		break;
	case 0:
#ifdef _DEBUG
		printf( "Running system...\n" );
		printf( "Command: %s\n", pCmd );
#endif
		// NOTE: we could use that to detect when a step finishes. But then it
		// would not work for remote compiling stuff.
//      execlp (pCmd, pCmd, NULL);
		system( pCmd );
		printf( "system() returned" );
		_exit( 0 );
		break;
	}
	return true;
}
#endif

#ifdef _WIN32
bool Q_Exec( const char *pCmd, bool bCreateConsole ){
	// G_DeWan: Don't know if this is needed for linux version

	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};            // Initialize all members to zero
	si.cb = sizeof( STARTUPINFO );     // Set byte count
	DWORD dwCreationFlags;

	if ( bCreateConsole ) {
		dwCreationFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;
	}
	else{
		dwCreationFlags = DETACHED_PROCESS | NORMAL_PRIORITY_CLASS;
	}

	for (; *pCmd == ' '; pCmd++ ) ;

	if ( !CreateProcess( NULL, (char *)pCmd, NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &pi ) ) {
		return false;
	}

	return true;
}
#endif

void StartBSP(){
	char exename[256];
	GetFilename( exename, "q3map" );
	UnixToDosPath( exename ); // do we want this done in linux version?

	char mapname[256];
	const char *pn = g_FuncTable.m_pfnReadProjectKey( "mapspath" );

	strcpy( mapname, pn );
	strcat( mapname, "/ac_prt.map" );
	UnixToDosPath( mapname );

	char command[1024];
	sprintf( command, "%s -nowater -fulldetail %s", exename, mapname );

	Q_Exec( command, TRUE );
}

void BuildMiniPrt( list<Str>* exclusionList ){
	// yes, we could just use -fulldetail option, but, as SPOG said
	// it'd be faster without all the hint, donotenter etc textures and
	// doors, etc

	DEntity world;

	char buffer[128];
	const char *pn = g_FuncTable.m_pfnReadProjectKey( "mapspath" );

	strcpy( buffer, pn );
	strcat( buffer, "/ac_prt.map" );
	FILE* pFile = fopen( buffer, "w" );

	// ahem, thx rr2
	if ( !pFile ) {
		return;
	}

	int count = g_FuncTable.m_pfnGetEntityCount();
	for ( int i = 0; i < count; i++ )
	{
		entity_t* ent = (entity_t*)g_FuncTable.m_pfnGetEntityHandle( i );

		epair_t* epl = *g_EntityTable.m_pfnGetEntityKeyValList( ent );

		epair_t* ep = epl;
		while ( ep )
		{
			if ( !strcmp( ep->key, "classname" ) ) {
				if ( !strcmp( ep->value, "worldspawn" ) ) {
					world.LoadFromEntity( i, FALSE );
					world.RemoveNonCheckBrushes( exclusionList, TRUE );
					world.SaveToFile( pFile );
				}
				else if ( strstr( ep->value, "info_" ) ) {
					world.ClearBrushes();
					world.ClearEPairs();
					world.LoadEPairList( epl );
					world.SaveToFile( pFile );
				}
				break;
			}

			ep = ep->next;
		}
	}

	fclose( pFile );

	StartBSP();
}

entity_s* FindEntityFromTargetname( const char* targetname, int* entNum ){
	DEntity world;

	int count = g_FuncTable.m_pfnGetEntityCount();
	for ( int i = 0; i < count; i++ )
	{
		world.ClearEPairs();

		entity_s* ent = (entity_s*)g_FuncTable.m_pfnGetEntityHandle( i );

		world.LoadEPairList( *g_EntityTable.m_pfnGetEntityKeyValList( ent ) );

		DEPair* tn = world.FindEPairByKey( "targetname" );
		if ( tn ) {
			if ( !stricmp( tn->value, targetname ) ) {
				if ( entNum ) {
					*entNum = i;
				}
				return ent;
			}
		}
	}
	return NULL;
}

void FillDefaultTexture( _QERFaceData* faceData, vec3_t va, vec3_t vb, vec3_t vc, const char* texture ){
	faceData->m_bBPrimit = FALSE;
	faceData->m_fRotate = 0;
	faceData->m_fScale[0] = 0.5;
	faceData->m_fScale[1] = 0.5;
	faceData->m_fShift[0] = 0;
	faceData->m_fShift[1] = 0;
	faceData->m_nContents = 0;
	faceData->m_nFlags = 0;
	faceData->m_nValue = 0;
	if ( *texture ) {
		strcpy( faceData->m_TextureName, texture );
	}
	else{
		strcpy( faceData->m_TextureName, "textures/common/caulk" );
	}
	VectorCopy( va, faceData->m_v1 );
	VectorCopy( vb, faceData->m_v2 );
	VectorCopy( vc, faceData->m_v3 );
}

float Determinant3x3( float a1, float a2, float a3,
					  float b1, float b2, float b3,
					  float c1, float c2, float c3 ){
	return a1 * ( b2 * c3 - b3 * c2 ) - a2 * ( b1 * c3 - b3 * c1 ) + a3 * ( b1 * c2 - b2 * c1 );
}

bool GetEntityCentre( const char* entity, vec3_t centre ){
	entity_s* ent = FindEntityFromTargetname( entity, NULL );
	if ( !ent ) {
		return FALSE;
	}

	int cnt = g_FuncTable.m_pfnAllocateEntityBrushHandles( ent );
	if ( cnt == 0 ) {
		g_FuncTable.m_pfnReleaseEntityBrushHandles();
		return FALSE;
	}

	brush_t* brush = (brush_t*)g_FuncTable.m_pfnGetEntityBrushHandle( 0 );
	DBrush cBrush;
	cBrush.LoadFromBrush_t( brush, FALSE );

	vec3_t min, max;
	cBrush.GetBounds( min, max );

	VectorAdd( min, max, centre );
	VectorScale( centre, 0.5f, centre );

	g_FuncTable.m_pfnReleaseEntityBrushHandles();
	return TRUE;
}

vec_t Min( vec_t a, vec_t b ){
	if ( a < b ) {
		return a;
	}
	return b;
}

void MakeNormal( vec_t* va, vec_t* vb, vec_t* vc, vec_t* out ) {
	vec3_t v1, v2;
	VectorSubtract( va, vb, v1 );
	VectorSubtract( vc, vb, v2 );
	CrossProduct( v1, v2, out );
}
