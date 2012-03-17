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

//
// writes quake3 map format from internal objects
//

static int g_count_entities;
static int g_count_brushes;

#include "plugin.h"
extern int g_MapVersion;

void Float_Write( float data, IDataStream *out ){
	if ( data == (int)data ) {
		out->printf( "%i ", (int)data );
	}
	else{
		out->printf( "%f ", data );
	}
}

void Patch_Write( patchMesh_t *pPatch, IDataStream *out ){
	int i, j;
	const char *str;

	// write shader name and matrix dimensions
	str = pPatch->pShader->getName();
	if ( strchr( str, ' ' ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Patch_Write: shader names with spaces are not allowed, ignoring '%s'\n", str );
		str = SHADER_NOT_FOUND;
	}
	if ( !strncmp( str, "textures/", 9 ) ) {
		str += 9;
	}
	out->printf( "patchDef2\n{\n%s\n( %i %i 0 0 0 )\n",
				 str, pPatch->width, pPatch->height );

	// write matrix
	out->printf( "(\n" );
	for ( i = 0; i < pPatch->width; i++ )
	{
		out->printf( "( " );
		for ( j = 0; j < pPatch->height; j++ )
		{
			out->printf( "( " );

			Float_Write( pPatch->ctrl[i][j].xyz[0], out );
			Float_Write( pPatch->ctrl[i][j].xyz[1], out );
			Float_Write( pPatch->ctrl[i][j].xyz[2], out );
			Float_Write( pPatch->ctrl[i][j].st[0], out );
			Float_Write( pPatch->ctrl[i][j].st[1], out );

			out->printf( ") " );
		}
		out->printf( ")\n" );
	}
	out->printf( ")\n}\n" );
}

void Face_Write( face_t *face, IDataStream *out, bool bAlternateTexdef = false ){
	int i, j;
	const char *str;

	// write planepts
	for ( i = 0; i < 3; i++ )
	{
		out->printf( "( " );
		for ( j = 0; j < 3; j++ )
		{
			Float_Write( face->planepts[i][j], out );
		}
		out->printf( ") " );
	}

	if ( bAlternateTexdef ) {
		// write alternate texdef
		out->printf( "( ( " );
		for ( i = 0; i < 3; i++ )
			Float_Write( face->brushprimit_texdef.coords[0][i], out );
		out->printf( ") ( " );
		for ( i = 0; i < 3; i++ )
			Float_Write( face->brushprimit_texdef.coords[1][i], out );
		out->printf( ") ) " );
	}

	// write shader name
	str = face->texdef.GetName();
	if ( strchr( str, ' ' ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Face_Write: shader names with spaces are not allowed, ignoring '%s'\n", str );
		str = SHADER_NOT_FOUND;
	}
	if ( !strncmp( str, "textures/", 9 ) ) {
		str += 9;
	}

	// Strip all remaining paths.
	// FIXME: Hydra - this is actually a HalfLife specific bit, not Q2 map format specific.
	if ( g_MapVersion == MAPVERSION_HL ) {
		char *pos;
		while ( ( pos = (char*)strchr( str, '/' ) ) != NULL ) {
			str = pos + 1; // to speed optimize, change the "while" to an "if"
		}
	}
	out->printf( "%s ", str );

	if ( !bAlternateTexdef ) {
		// write texdef
		out->printf( "%i %i %i %f %f ",
					 (int)face->texdef.shift[0],
					 (int)face->texdef.shift[1],
					 (int)face->texdef.rotate,
					 face->texdef.scale[0],
					 face->texdef.scale[1] );
	}

	if ( g_MapVersion == MAPVERSION_Q3 ) {
		// write surface flags
		out->printf( "%i %i %i\n",
					 face->texdef.contents,
					 face->texdef.flags,
					 face->texdef.value );
	}

	if ( ( g_MapVersion == MAPVERSION_HL ) || ( g_MapVersion == MAPVERSION_Q2 ) ) {
		// write surface flags if non-zero values.
		if ( face->texdef.contents || face->texdef.flags || face->texdef.value ) {
			out->printf( "%i %i %i\n",
						 face->texdef.contents,
						 face->texdef.flags,
						 face->texdef.value );
		}
		else
		{
			out->printf( "\n" );
		}
	}

}

void Primitive_Write( brush_t *pBrush, IDataStream *out ){
	if ( ( g_MapVersion == MAPVERSION_Q2 ) && ( pBrush->patchBrush ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Primitive_Write: Patches are not supported in Quake2, ignoring Brush %d\n", g_count_brushes++ );
	}
	else
	{
		out->printf( "// brush %i\n", g_count_brushes++ );
		out->printf( "{\n" );
		if ( pBrush->patchBrush ) {
			Patch_Write( pBrush->pPatch, out );
		}
		else if ( pBrush->bBrushDef ) {
			out->printf( "brushDef\n{\n" );
			for ( face_t *face = pBrush->brush_faces; face != NULL; face = face->next )
				Face_Write( face, out, true );
			out->printf( "}\n" );
		}
		else{
			for ( face_t *face = pBrush->brush_faces; face != NULL; face = face->next )
				Face_Write( face, out );
		}
		out->printf( "}\n" );
	}
}

void Entity_Write( entity_t *pEntity, IDataStream *out ){
	epair_t *pEpair;
	CPtrArray *brushes = (CPtrArray*)pEntity->pData;
	out->printf( "// entity %i\n", g_count_entities++ );
	out->printf( "{\n" );
	for ( pEpair = pEntity->epairs; pEpair != NULL; pEpair = pEpair->next )
		out->printf( "\"%s\" \"%s\"\n", pEpair->key, pEpair->value );
	g_count_brushes = 0;
	for ( int i = 0; i < brushes->GetSize(); i++ )
		Primitive_Write( (brush_t*)brushes->GetAt( i ), out );
	out->printf( "}\n" );
}

void Map_Write( CPtrArray *map, IDataStream *out ){
	g_count_entities = 0;
	for ( int i = 0; i < map->GetSize(); i++ )
		Entity_Write( (entity_t*)map->GetAt( i ), out );
}

void Map_WriteQ3( CPtrArray *map, IDataStream *out ){
	g_MapVersion = MAPVERSION_Q3;
	Map_Write( map,out );
}

void Map_WriteHL( CPtrArray *map, IDataStream *out ){
	g_MapVersion = MAPVERSION_HL;
	Map_Write( map,out );
}

void Map_WriteQ2( CPtrArray *map, IDataStream *out ){
	g_MapVersion = MAPVERSION_Q2;
	Map_Write( map,out );
}
