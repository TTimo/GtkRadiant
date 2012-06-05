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
// parses xml tree format into internal objects
//


#include "plugin.h"

void Patch_XMLParse( patchMesh_t *pPatch, xmlNodePtr surface ){
	char *str, *content;
	int i, j;

	for ( xmlNodePtr current = surface->children; current != NULL; current = current->next )
	{
		if ( current->type != XML_ELEMENT_NODE ) {
			continue;
		}
		if ( !strcmp( (char *)current->name, "matrix" ) ) {
			str = (char *)xmlGetProp( current, (xmlChar *)"width" );
			pPatch->width = atoi( str );
			xmlFree( str );
			str = (char *)xmlGetProp( current, (xmlChar *)"height" );
			pPatch->height = atoi( str );
			xmlFree( str );

			content = Q_StrDup( (char *)current->children->content );

			str = strtok( content, " \n\r\t\v\0" );
			for ( i = 0; i < pPatch->width; i++ )
			{
				for ( j = 0; j < pPatch->height; j++ )
				{
					pPatch->ctrl[i][j].xyz[0] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
					pPatch->ctrl[i][j].xyz[1] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
					pPatch->ctrl[i][j].xyz[2] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
					pPatch->ctrl[i][j].st[0] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
					pPatch->ctrl[i][j].st[1] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
				}
			}

			delete [] content;
		}
		else if ( !strcmp( (char *)current->name, "shader" ) ) {
			pPatch->pShader = QERApp_Shader_ForName( (char*)current->children->content );
			pPatch->d_texture = pPatch->pShader->getTexture();
		}
	}
}

void Face_XMLParse( face_t *face, xmlNodePtr surface ){
	char *str, *content;
	int i, j;

	for ( xmlNodePtr current = surface->children; current != NULL; current = current->next )
	{
		if ( current->type != XML_ELEMENT_NODE ) {
			continue;
		}
		if ( !strcmp( (char *)current->name, "planepts" ) ) {
			content = Q_StrDup( (char *)current->children->content );

			str = strtok( content, " \n\r\t\v\0" );
			for ( i = 0 ; i < 3 ; i++ )
			{
				for ( j = 0 ; j < 3 ; j++ )
				{
					face->planepts[i][j] = atof( str );
					str = strtok( NULL, " \n\r\t\v\0" );
				}
			}

			delete [] content;
		}
		else if ( !strcmp( (char *)current->name, "texdef" ) ) {
			content = Q_StrDup( (char *)current->children->content );

			str = strtok( content, " \n\r\t\v\0" );
			face->texdef.shift[0] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.shift[1] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.rotate = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.scale[0] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.scale[1] = atof( str );

			delete [] content;
		}
		else if ( !strcmp( (char *)current->name, "bpmatrix" ) ) {
			content = Q_StrDup( (char *)current->children->content );

			str = strtok( content, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[0][0] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[0][1] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[0][2] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[1][0] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[1][1] = atof( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->brushprimit_texdef.coords[1][2] = atof( str );

			delete [] content;
		}
		else if ( !strcmp( (char *)current->name, "flags" ) ) {
			content = Q_StrDup( (char *)current->children->content );

			str = strtok( content, " \n\r\t\v\0" );
			face->texdef.contents = atoi( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.flags = atoi( str );
			str = strtok( NULL, " \n\r\t\v\0" );
			face->texdef.value = atoi( str );

			delete [] content;
		}
		else if ( !strcmp( (char *)current->name, "shader" ) ) {
			face->texdef.SetName( (char *)current->children->content );
		}
	}
}

void Brush_XMLParse( brush_t *pBrush, xmlNodePtr primitive ){
	face_t    *f;

	for ( xmlNodePtr current = primitive->children; current != NULL; current = current->next )
	{
		if ( current->type != XML_ELEMENT_NODE ) {
			continue;
		}
		f = pBrush->brush_faces;
		pBrush->brush_faces = Face_Alloc();
		Face_XMLParse( pBrush->brush_faces, current );
		pBrush->brush_faces->next = f;
	}
}

void Entity_XMLParse( entity_t *pEntity, xmlNodePtr entity ){
	brush_t *pBrush;

	for ( xmlNodePtr current = entity->children; current != NULL; current = current->next )
	{
		if ( current->type != XML_ELEMENT_NODE ) {
			continue;
		}
		if ( !strcmp( (char *)current->name, "epair" ) ) {
			char *key = (char *)xmlGetProp( current, (xmlChar *)"key" );
			char *value = (char *)xmlGetProp( current, (xmlChar *)"value" );
			SetKeyValue( pEntity, key, value );
			xmlFree( key );
			xmlFree( value );
		}
		else if ( strcmp( (char *)current->name, "brush" ) == 0 ) {
			pBrush = Brush_Alloc();
			Brush_XMLParse( pBrush, current );
			( (CPtrArray*)pEntity->pData )->Add( pBrush );
		}
		else if ( strcmp( (char *)current->name, "patch" ) == 0 ) {
			pBrush = Brush_Alloc();
			pBrush->patchBrush = true;
			pBrush->pPatch = Patch_Alloc();
			pBrush->pPatch->pSymbiot = pBrush;
			Patch_XMLParse( pBrush->pPatch, current );
			( (CPtrArray*)pEntity->pData )->Add( pBrush );
		}
	}
}

void Map_XMLRead( CPtrArray *map, xmlNodePtr map_node ){
	entity_t *pEntity;
	xmlNodePtr current;

	for ( current = map_node->children; current != NULL; current = current->next )
	{
		if ( current->type != XML_ELEMENT_NODE ) {
			continue;
		}
		pEntity = Entity_Alloc();
		pEntity->pData = new CPtrArray;
		Entity_XMLParse( pEntity, current );
		map->Add( pEntity );
	}
}

// SPoG
// temporarily copied from qe3.cpp
// duplicate code starts here (note: g_strAppPath swapped for g_FuncTable.m_pfnGetQERPath())

void HandleXMLError( void* ctxt, const char* text, ... ){
	va_list argptr;
	static char buf[32768];

	va_start( argptr,text );
	vsprintf( buf, text, argptr );
	Sys_FPrintf( SYS_ERR, "XML %s\n", buf );
	va_end( argptr );
}

#define DTD_BUFFER_LENGTH 1024
xmlDocPtr ParseXMLStream( IDataStream *stream, bool validate = false ){
	xmlDocPtr doc = NULL;
	bool wellFormed = false, valid = false;
	int res, size = 1024;
	char chars[1024];
	xmlParserCtxtPtr ctxt;

	// SPoG
	// HACK: use AppPath to resolve DTD location
	// do a buffer-safe string copy and concatenate
	int i;
	char* w;
	const char* r;
	char buf[DTD_BUFFER_LENGTH];

	w = buf;
	i = 0;
	// copy
	//assert(g_FuncTable.m_pfnGetQERPath() != NULL);
	for ( r = g_FuncTable.m_pfnGetQERPath(); i < DTD_BUFFER_LENGTH && *r != '\0'; i++, r++ ) w[i] = *r;
	// concatenate
	for ( r = "dtds/"; i < DTD_BUFFER_LENGTH && *r != '\0'; i++, r++ ) w[i] = *r;
	// terminate
	w[i] = '\0';

	if ( i == DTD_BUFFER_LENGTH ) {
		HandleXMLError( NULL, "ERROR: buffer overflow: DTD path length too large\n" );
		return NULL;
	}

	//if(validate)
	//  xmlDoValidityCheckingDefaultValue = 1;
	//else
	xmlDoValidityCheckingDefaultValue = 0;

	xmlSetGenericErrorFunc( NULL, HandleXMLError );

	res = stream->Read( chars, 4 );
	if ( res > 0 ) {
		ctxt = xmlCreatePushParserCtxt( NULL, NULL, chars, res, buf );

		while ( ( res = stream->Read( chars, size ) ) > 0 )
		{
			xmlParseChunk( ctxt, chars, res, 0 );
		}
		xmlParseChunk( ctxt, chars, 0, 1 );
		doc = ctxt->myDoc;

		wellFormed = ( ctxt->wellFormed == 1 );
		valid = ( ctxt->valid == 1 );

		xmlFreeParserCtxt( ctxt );
	}

	if ( wellFormed && ( !validate || ( validate && valid ) ) ) {
		return doc;
	}

	if ( doc != NULL ) {
		xmlFreeDoc( doc );
	}

	return NULL;
}

// duplicate code ends here

void Map_Read( IDataStream *in, CPtrArray *map ){
	xmlDocPtr doc;

	doc = ParseXMLStream( in, false ); // quick hack while dtd validation is broken

	if ( doc != NULL ) {
		xmlNodePtr node = doc->children;
		while ( node != NULL && node->type != XML_ELEMENT_NODE ) node = node->next;
		if ( node != NULL ) {
			Map_XMLRead( map, node );
		}
	}

	xmlFreeDoc( doc );
}
