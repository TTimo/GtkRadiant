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

#include "cmdlib.h"

#include "synapse.h"
#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#define USE_ECLASSMANAGER_DEFINE
#include "ieclass.h"
#define USE_SCRIPLIBTABLE_DEFINE
#include "iscriplib.h"

#define __VFSTABLENAME g_FileSystemTable_def
#define USE_VFSTABLE_DEFINE
#include "ifilesystem.h"


#include "eclass_def.h"

/*! \file eclass_def.cpp
    \brief .def entity description format
    implements parsing for .def entity format
    this is statically linked into the radiant core as we always need it, but really considered
    as an idependant module by the rest of the core. "ECLASS_MAJOR" "def"
 */

_QERScripLibTable g_ScripLibTable;
_EClassManagerTable g_EClassManagerTable;
_QERFuncTable_1 g_FuncTable;
_QERFileSystemTable g_FileSystemTable_def;

CSynapseBuiltinClientDef eclass_def;

// forward declare, I'm cheap
void Eclass_ScanFile( char *filename );

const char* EClass_GetExtension(){
	return "def";
}

void CSynapseBuiltinClientDef::EnumerateInterfaces( CSynapseServer *server ){
	AddAPI( SCRIPLIB_MAJOR, NULL, sizeof( g_ScripLibTable ), SYN_REQUIRE, &g_ScripLibTable );
	AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	AddAPI( ECLASSMANAGER_MAJOR, NULL, sizeof( g_EClassManagerTable ), SYN_REQUIRE, &g_EClassManagerTable );
	// hardcode the minor for now, we can still add it to the synapse.config at some point
	AddAPI( VFS_MAJOR, "*", sizeof( g_FileSystemTable_def ), SYN_REQUIRE, &g_FileSystemTable_def );

	AddAPI( ECLASS_MAJOR, "def", sizeof( _EClassTable ) );
}

bool CSynapseBuiltinClientDef::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, ECLASS_MAJOR ) ) {
		_EClassTable* pTable = static_cast<_EClassTable*>( pAPI->mpTable );
		pTable->m_pfnScanFile = &Eclass_ScanFile;
		pTable->m_pfnGetExtension = &EClass_GetExtension;

		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseBuiltinClientDef::GetInfo(){
	return "Builtin .def module built " __DATE__ " " RADIANT_VERSION;
}

// ------------------------------------------------------------------------------------------------

qboolean eclass_found;
char *debugname;

void setSpecialLoad( eclass_t *e, const char* pWhat, char*& p ){
	// Hydra: removed some amazingly bad cstring usage, whoever wrote that
	// needs to be taken out and shot.

	char *pText = NULL;
	char *where = NULL;

	p = NULL; // incase we don't find what we're looking for.
	where = strstr( e->comments,pWhat );
	if ( !where ) {
		return;
	}

	pText = where + strlen( pWhat );
	if ( *pText == '\"' ) {
		pText++;
	}

	where = strchr( pText,'\"' );
	if ( where ) {
		int len = ( where - pText );
		p = new char[len + 1];
		strncpy( p,pText,len );
		p[len] = 0; // just to make sure, as most implementations of strncpy don't null terminate
	}
	else{
		p = strdup( pText );
	}
}

qboolean IsModelEntity( const char *name );

eclass_t *Eclass_InitFromText( char *text ){
	char    *t;
	int len;
	int r, i;
	char parms[256], *p;
	eclass_t    *e;
	char color[128];

	e = (eclass_t*)malloc( sizeof( *e ) );
	memset( e, 0, sizeof( *e ) );

	text += strlen( "/*QUAKED " );

	// grab the name
	text = COM_Parse( text );
	e->name = (char*)malloc( strlen( Get_COM_Token() ) + 1 );
	strcpy( e->name, Get_COM_Token() );
	debugname = e->name;

	// grab the color, reformat as texture name
	r = sscanf( text," (%f %f %f)", &e->color[0], &e->color[1], &e->color[2] );
	if ( r != 3 ) {
		return e;
	}
	sprintf( color, "(%f %f %f)", e->color[0], e->color[1], e->color[2] );
	//strcpy (e->texdef.name, color);
	e->texdef.SetName( color );

	while ( *text != ')' )
	{
		if ( !*text ) {
			return e;
		}
		text++;
	}
	text++;

	// get the size
	t = COM_Parse( text );
	if ( Get_COM_Token()[0] == '(' ) { // parse the size as two vectors
                text = t;
		e->fixedsize = true;
		r = sscanf( text,"%f %f %f) (%f %f %f)", &e->mins[0], &e->mins[1], &e->mins[2],
					&e->maxs[0], &e->maxs[1], &e->maxs[2] );
		if ( r != 6 ) {
			return e;
		}

		for ( i = 0 ; i < 2 ; i++ )
		{
			while ( *text != ')' )
			{
				if ( !*text ) {
					return e;
				}
				text++;
			}
			text++;
		}
	}
	else if ( Get_COM_Token()[0] == '?' ) {
		 text = t;
	}

	// get the flags

	// copy to the first /n
	p = parms;
	while ( *text && *text != '\n' )
		*p++ = *text++;
	*p = 0;
	text++;

	// any remaining words are parm flags
	p = parms;
	for ( i = 0 ; i < MAX_FLAGS ; i++ )
	{
		p = COM_Parse( p );
		if ( !p ) {
			break;
		}
		strcpy( e->flagnames[i], Get_COM_Token() );
	}

	// find the length until close comment
	for ( t = text ; t[0] && !( t[0] == '*' && t[1] == '/' ) ; t++ )
		;

	// copy the comment block out
	len = t - text;
	e->comments = (char*)malloc( len + 1 );
	memcpy( e->comments, text, len );
#ifdef _WIN32
	// the win32 Gtk widgets are expecting text stuff to be in unix format (that is CR only instead of DOS's CR/LF)
	// we convert on the fly by replacing the LF with a ' ' (yeah I'm cheap)
	for ( i = 0 ; i < len ; i++ )
		if ( text[i] == '\r' ) {
			e->comments[i] = ' ';
		}
		else{
			e->comments[i] = text[i];
		}
#endif
	e->comments[len] = 0;

	setSpecialLoad( e, "model=", e->modelpath );
	setSpecialLoad( e, "skin=", e->skinpath );
	char *pFrame = NULL;
	setSpecialLoad( e, "frame=", pFrame );
	if ( pFrame != NULL ) {
		e->nFrame = atoi( pFrame );
		delete pFrame; //Hydra - Fixed memory leak!
	}
	char *pAngle = NULL;
	setSpecialLoad( e, "eangle=", pAngle );
	if ( pAngle != NULL ) {
		if ( strcmpi( pAngle, "true" ) == 0 || atoi( pAngle ) == 1 ) {
			e->nShowFlags |= ECLASS_ANGLE;
		}
		delete pAngle;
	}

	if ( !e->skinpath ) {
		setSpecialLoad( e, "texture=", e->skinpath );
	}

	// setup show flags
	e->nShowFlags = 0;
	if ( strcmpi( e->name, "light" ) == 0 || strcmpi( e->name, "dlight" ) == 0
			|| strcmpi( e->name, "lightjunior" ) == 0 || strcmpi( e->name, "light_spot" ) == 0 ) {
		e->nShowFlags |= ECLASS_LIGHT;
	}

	if (  ( strnicmp( e->name, "info_player", strlen( "info_player" ) ) == 0 )
		  || ( strnicmp( e->name, "path_corner", strlen( "path_corner" ) ) == 0 )
		  || ( strnicmp( e->name, "team_ctf", strlen( "team_ctf" ) ) == 0 )
		  || ( strnicmp( e->name, "misc_teleporter_dest", strlen( "misc_teleporter_dest" ) ) == 0 )
		  ) {
		e->nShowFlags |= ECLASS_ANGLE;
	}
	for ( i = 0 ; i < MAX_FLAGS ; i++ )
	{
		if ( e->flagnames[i] && e->flagnames[i][0] != 0 && strcmpi( e->flagnames[i], "angle" ) && e->fixedsize ) {
			e->nShowFlags |= ECLASS_ANGLE;
		}
	}
	if ( strcmpi( e->name, "path" ) == 0 ) {
		e->nShowFlags |= ECLASS_PATH;
	}
	if ( IsModelEntity( e->name ) == qtrue ) {
		e->nShowFlags |= ECLASS_MISCMODEL;
	}

	return e;
}

void Eclass_ScanFile( char *filename ){
	int size;
	char    *data;
	eclass_t    *e;
	int i;
	char temp[1024];

	QE_ConvertDOSToUnixName( temp, filename );

	size = vfsLoadFullPathFile( filename, (void**)&data );
	if ( size <= 0 ) {
		Sys_FPrintf( SYS_ERR, "Eclass_ScanFile: %s not found\n", filename );
		return;
	}
	Sys_Printf( "ScanFile: %s\n", temp );
	eclass_found = false;
	for ( i = 0 ; i < size ; i++ )
	{
		if ( !strncmp( data + i, "/*QUAKED",8 ) ) {
			e = Eclass_InitFromText( data + i );
			if ( e ) {
				Eclass_InsertAlphabetized( e );
			}
			else{
				Sys_FPrintf( SYS_ERR, "Error parsing: %s in %s\n",debugname, filename );
			}

			// single ?
			*Get_Eclass_E() = e;
			Set_Eclass_Found( true );
			if ( Get_Parsing_Single() ) {
				break;
			}
		}
	}

	g_free( data );
}
