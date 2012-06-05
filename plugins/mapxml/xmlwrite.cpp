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
// writes xml tree format from internal objects
//


#include "plugin.h"

char *str_append_token( char *str1, const char *str2 ){
	char *str;
	if ( str1 != NULL ) {
		str = new char[strlen( str1 ) + strlen( str2 ) + 2];
		sprintf( str, "%s %s", str1, str2 );
		delete [] str1;
	}
	else
	{
		str = new char[strlen( str2 ) + 1];
		strcpy( str, str2 );
	}
	return str;
}

void str_from_float( char *buf, float f ){
	if ( f == (int)f ) {
		sprintf( buf, "%i", (int)f );
	}
	else{ sprintf( buf, "%f", f ); }
}

void Patch_XMLWrite( patchMesh_t *pPatch, xmlNodePtr surface ){
	char buf[16];
	char *str;
	int i, j;
	xmlNodePtr node;

	// write shader
	node = xmlNewChild( surface, NULL, (xmlChar *)"shader", (xmlChar *)pPatch->pShader->getName() );

	// write matrix
	str = NULL;
	for ( i = 0; i < pPatch->width; i++ )
	{
		for ( j = 0; j < pPatch->height; j++ )
		{
			str_from_float( buf, pPatch->ctrl[i][j].xyz[0] );
			str = str_append_token( str, buf );
			str_from_float( buf, pPatch->ctrl[i][j].xyz[1] );
			str = str_append_token( str, buf );
			str_from_float( buf, pPatch->ctrl[i][j].xyz[2] );
			str = str_append_token( str, buf );
			str_from_float( buf, pPatch->ctrl[i][j].st[0] );
			str = str_append_token( str, buf );
			str_from_float( buf, pPatch->ctrl[i][j].st[1] );
			str = str_append_token( str, buf );
		}
	}

	node = xmlNewChild( surface, NULL, (xmlChar *)"matrix", (xmlChar *)str );
	delete [] str;
	sprintf( buf, "%i", pPatch->width );
	xmlSetProp( node, (xmlChar *)"width", (xmlChar *)buf );
	sprintf( buf, "%i", pPatch->height );
	xmlSetProp( node, (xmlChar *)"height", (xmlChar *)buf );
}

void Face_XMLWrite( face_t *face, xmlNodePtr surface, bool bAlternateTexdef = false ){
	char buf[16];
	xmlNodePtr node;
	int i, j;
	char *str;

	// write shader
	node = xmlNewChild( surface, NULL, (xmlChar *)"shader", (xmlChar *)face->texdef.GetName() );

	// write planepts
	str = NULL;
	for ( i = 0 ; i < 3 ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			str_from_float( buf, face->planepts[i][j] );
			str = str_append_token( str, buf );
		}
	}

	node = xmlNewChild( surface, NULL, (xmlChar *)"planepts", (xmlChar *)str );
	delete [] str;

	if ( !bAlternateTexdef ) {
		// write texdef
		sprintf( buf, "%i", (int)face->texdef.shift[0] );
		str = str_append_token( NULL, buf );
		sprintf( buf, "%i", (int)face->texdef.shift[1] );
		str = str_append_token( str, buf );
		sprintf( buf, "%i", (int)face->texdef.rotate );
		str = str_append_token( str, buf );
		sprintf( buf, "%f", face->texdef.scale[0] );
		str = str_append_token( str, buf );
		sprintf( buf, "%f", face->texdef.scale[1] );
		str = str_append_token( str, buf );

		node = xmlNewChild( surface, NULL, (xmlChar *)"texdef", (xmlChar *)str );
		delete [] str;
	}
	else
	{
		// write matrix texdef
		str = NULL;
		for ( i = 0 ; i < 2 ; i++ )
		{
			for ( j = 0 ; j < 3 ; j++ )
			{
				str_from_float( buf, face->brushprimit_texdef.coords[i][j] );
				str = str_append_token( str, buf );
			}
		}
		node = xmlNewChild( surface, NULL, (xmlChar *)"bpmatrix", (xmlChar *)str );
		delete [] str;
	}

	// write flags
	sprintf( buf, "%i", face->texdef.contents );
	str = str_append_token( NULL, buf );
	sprintf( buf, "%i", face->texdef.flags );
	str = str_append_token( str, buf );
	sprintf( buf, "%i", face->texdef.value );
	str = str_append_token( str, buf );

	node = xmlNewChild( surface, NULL, (xmlChar *)"flags", (xmlChar *)str );
	delete [] str;
}

void Brush_XMLWrite( brush_t *brush, xmlNodePtr primitive ){
	xmlNodePtr node;

	for ( face_t *face = brush->brush_faces; face != NULL; face = face->next )
	{
		node = xmlNewChild( primitive, NULL, (xmlChar *)"plane", NULL );
		Face_XMLWrite( face, node, brush->bBrushDef );
	}
}

void Epair_XMLWrite( epair_t *pEpair, xmlNodePtr epair ){
	xmlSetProp( epair, (xmlChar *)"key", (xmlChar *)pEpair->key );
	xmlSetProp( epair, (xmlChar *)"value", (xmlChar *)pEpair->value );
}

void Entity_XMLWrite( entity_t *pEntity, xmlNodePtr entity ){
	brush_t *pBrush;
	epair_t *pEpair;
	xmlNodePtr node;

	CPtrArray *brushes = (CPtrArray*)pEntity->pData;

	for ( pEpair = pEntity->epairs; pEpair != NULL; pEpair = pEpair->next )
	{
		node = xmlNewChild( entity, NULL, (xmlChar *)"epair", NULL );
		Epair_XMLWrite( pEpair, node );
	}

	for ( int i = 0; i < brushes->GetSize(); i++ )
	{
		pBrush = (brush_t*)brushes->GetAt( i );

		if ( pBrush->patchBrush ) {
			node = xmlNewChild( entity, NULL, (xmlChar *)"patch", NULL );
			Patch_XMLWrite( pBrush->pPatch, node );
		}
		else
		{
			node = xmlNewChild( entity, NULL, (xmlChar *)"brush", NULL );
			Brush_XMLWrite( pBrush, node );
		}
	}
}

void Map_XMLWrite( CPtrArray *map, xmlNodePtr map_node ){
	entity_t *pEntity;
	xmlNodePtr node;

	for ( int i = 0; i < map->GetSize(); i++ )
	{
		pEntity = (entity_t*)map->GetAt( i );

		node = xmlNewChild( map_node, NULL, (xmlChar *)"entity", NULL );
		Entity_XMLWrite( pEntity, node );
	}
}

void Map_Write( CPtrArray *map, IDataStream *out ){
	xmlChar* buf;
	int len;

	xmlDocPtr doc = xmlNewDoc( (xmlChar *)"1.0" );
	xmlCreateIntSubset( doc, (xmlChar *)"mapq3", NULL, (xmlChar *)"mapq3.dtd" );
	doc->children->next = xmlNewDocNode( doc, NULL, (xmlChar *)"mapq3", NULL );

	Map_XMLWrite( map, doc->children->next );

	// xmlDocDumpMemory(doc, &buf, &len);
	xmlDocDumpFormatMemory( doc, &buf, &len, 1 );
	xmlFreeDoc( doc );

	out->Write( buf, len );

	xmlFree( buf );
}
