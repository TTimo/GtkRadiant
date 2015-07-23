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

#include "stdafx.h"

#define MAX_POINTFILE   8192
static vec3_t s_pointvecs[MAX_POINTFILE];
static int s_num_points, s_check_point;

CPointfile g_pointfile;

// CPointfile routine used by the standard code ---------------------------------

void CPointfile::Init(){
	s_num_points = 0;
}

void CPointfile::PushPoint( vec3_t v ){
	if ( s_num_points < MAX_POINTFILE ) {
		VectorCopy( v, s_pointvecs[s_num_points] );
		s_num_points++;
	}
}

// create the display list at the end
void CPointfile::GenerateDisplayList(){
	int i;

	if ( !g_qeglobals.d_pointfile_display_list ) {
		g_qeglobals.d_pointfile_display_list = qglGenLists( 1 );
	}

	qglNewList( g_qeglobals.d_pointfile_display_list,  GL_COMPILE );

	qglColor3f( 1, 0, 0 );
	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_TEXTURE_1D );
	qglLineWidth( 4 );
	qglBegin( GL_LINE_STRIP );
	for ( i = 0; i < s_num_points; i++ )
	{
		if ( s_num_points < MAX_POINTFILE ) {
			qglVertex3fv( s_pointvecs[i] );
		}
	}
	qglEnd();
	qglLineWidth( 1 );

	qglEndList();
}

// old (but still relevant) pointfile code -------------------------------------

void Pointfile_Delete( void ){
	char name[1024];

	strcpy( name, currentmap );
	StripExtension( name );
	strcat( name, ".lin" );

	remove( name );
}

// advance camera to next point
void Pointfile_Next( void ){
	vec3_t dir;

	if ( s_check_point >= s_num_points - 2 ) {
		Sys_Status( "End of pointfile", 0 );
		return;
	}
	s_check_point++;
	VectorCopy( s_pointvecs[s_check_point], g_pParentWnd->GetCamWnd()->Camera()->origin );
	VectorCopy( s_pointvecs[s_check_point], g_pParentWnd->GetXYWnd()->GetOrigin() );
	VectorSubtract( s_pointvecs[s_check_point + 1], g_pParentWnd->GetCamWnd()->Camera()->origin, dir );
	VectorNormalize( dir, dir );
	g_pParentWnd->GetCamWnd()->Camera()->angles[1] = atan2( dir[1], dir[0] ) * 180 / 3.14159;
	g_pParentWnd->GetCamWnd()->Camera()->angles[0] = asin( dir[2] ) * 180 / 3.14159;

	Sys_UpdateWindows( W_ALL );
}

// advance camera to previous point
void Pointfile_Prev( void ){
	vec3_t dir;

	if ( s_check_point == 0 ) {
		Sys_Status( "Start of pointfile", 0 );
		return;
	}
	s_check_point--;
	VectorCopy( s_pointvecs[s_check_point], g_pParentWnd->GetCamWnd()->Camera()->origin );
	VectorCopy( s_pointvecs[s_check_point], g_pParentWnd->GetXYWnd()->GetOrigin() );
	VectorSubtract( s_pointvecs[s_check_point + 1], g_pParentWnd->GetCamWnd()->Camera()->origin, dir );
	VectorNormalize( dir, dir );
	g_pParentWnd->GetCamWnd()->Camera()->angles[1] = atan2( dir[1], dir[0] ) * 180 / 3.14159;
	g_pParentWnd->GetCamWnd()->Camera()->angles[0] = asin( dir[2] ) * 180 / 3.14159;

	Sys_UpdateWindows( W_ALL );
}

void WINAPI Pointfile_Check( void ){
	char name[1024];
	int size;
	char    *data;
	char  *text;
	int line = 1;
	vec3_t v;

	strcpy( name, currentmap );
	StripExtension( name );
	strcat( name, ".lin" );

	size = vfsLoadFullPathFile( name, (void**)&data );
	if ( size <= 0 ) {
		Sys_FPrintf( SYS_ERR, "Pointfile %s not found\n", name );
		return;
	}

	// store a pointer
	text = data;

	Sys_Printf( "Reading pointfile %s\n", name );

	g_pointfile.Init();

	while ( *data )
	{
		if ( sscanf( data,"%f %f %f", &v[0], &v[1], &v[2] ) != 3 ) {
			Sys_Printf( "Corrupt point file, line %d\n",line );
			break;
		}

		while ( *data && *data != '\n' )
		{
			if ( *( data - 1 ) == ' ' && *( data ) == '-' && *( data + 1 ) == ' ' ) {
				break;
			}
			data++;
		}
		// deal with zhlt style point files.
		if ( *data == '-' ) {
			if ( sscanf( data,"- %f %f %f", &v[0], &v[1], &v[2] ) != 3 ) {
				Sys_Printf( "Corrupt point file, line %d\n",line );
				break;
			}

			while ( *data && *data != '\n' )
				data++;

		}
		while ( *data == '\n' )
		{
			data++; // skip the \n
			line++;
		}
		g_pointfile.PushPoint( v );
	}

	g_free( text );

	if ( g_PrefsDlg.m_bDisplayLists ) {
		g_pointfile.GenerateDisplayList();
	}
	s_check_point = 0;

	Sys_UpdateWindows( W_ALL );
}

void Pointfile_Draw( void ){
	if ( g_PrefsDlg.m_bDisplayLists ) {
		int i;

		qglColor3f( 1, 0, 0 );
		qglDisable( GL_TEXTURE_2D );
		qglDisable( GL_TEXTURE_1D );
		qglLineWidth( 4 );
		qglBegin( GL_LINE_STRIP );
		for ( i = 0; i < s_num_points; i++ )
		{
			if ( s_num_points < MAX_POINTFILE ) {
				qglVertex3fv( s_pointvecs[i] );
			}
		}
		qglEnd();
		qglLineWidth( 1 );
	} else {
		qglCallList( g_qeglobals.d_pointfile_display_list );
	}
}

void Pointfile_Clear( void ){
	if ( !g_qeglobals.d_pointfile_display_list ) {
		return;
	}

	qglDeleteLists( g_qeglobals.d_pointfile_display_list, 1 );
	g_qeglobals.d_pointfile_display_list = 0;
	Sys_UpdateWindows( W_ALL );
}

// CPointfile implementation for SAX speicific stuff -------------------------------
void CPointfile::saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs ){
	if ( strcmp( (char *)name, "polyline" ) == 0 ) {
		Init();
		// there's a prefs setting to avoid stopping on leak
		if ( !g_PrefsDlg.m_bLeakStop ) {
			ctx->stop_depth = 0;
		}
	}
}

void CPointfile::saxEndElement( message_info_t *ctx, const xmlChar *name ){
	if ( strcmp( (char *)name, "polyline" ) == 0 ) {
		// we are done
		if ( g_PrefsDlg.m_bDisplayLists ) {
			GenerateDisplayList();
		}
		s_check_point = 0;
		ctx->bGeometry = false;
	}
}

// only "point" is expected to have characters around here
void CPointfile::saxCharacters( message_info_t *ctx, const xmlChar *ch, int len ){
	vec3_t v;

	sscanf( (char *)ch, "%f %f %f\n", &v[0], &v[1], &v[2] );
	PushPoint( v );
}

char * CPointfile::getName(){
	return const_cast<char*>("Map is leaked");
}
