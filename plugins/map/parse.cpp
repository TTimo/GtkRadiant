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

// Hydra - FIXME : TTimo, We need to know what game + engine we're using for
// the halflife (not Q2) specific stuff
// we need an API for modules to get this info!

//
// parses quake3 map format into internal objects
//

#include "plugin.h"

// cmdlib
extern void ExtractFileName( const char *path, char *dest );

extern int g_MapVersion;
int abortcode; // see imap.h for values.

// Start of half-life specific stuff

GSList *g_WadList; // halflife specific.
GSList *g_TextureNameCache; // halflife specific.

// NOTE TTimo: yuck..
void FreeGSList( GSList *l ){
	while ( l )
	{
		free( l->data );
		l = g_slist_remove( l, l->data );
	}
}

// NOTE TTimo: ideally, this would be using Str functions instead
void trim( char *str ){
	int len;
	len = strlen( str );
	while ( str[--len] == ' ' )
		str[len] = 0;
}

void BuildWadList( char *wadstr ){
	char wads[2048]; // change to CString usage ?
	wads[0] = 0;
	char *p1,*p2;
	char cleanwadname[QER_MAX_NAMELEN];

	g_WadList = NULL;

	strcpy( wads,wadstr );
	QE_ConvertDOSToUnixName( wads,wads );

	// ok, we got the list of ; delimited wads, now split it into a GSList that contains
	// just the wad names themselves.

	p1 = wads;

	do
	{
		p2 = strchr( p1,';' );
		if ( p2 ) {
			*p2 = 0; // swap the ; with a null terminator

		}
		if ( strchr( p1,'/' ) || strchr( p1,'\\' ) ) {
			ExtractFileName( p1,cleanwadname );

			trim( cleanwadname );

			if ( *cleanwadname ) {
				g_WadList = g_slist_append( g_WadList, strdup( cleanwadname ) );
				Sys_Printf( "wad: %s\n",cleanwadname );
			}
		}
		else
		{
			trim( p1 );
			if ( *p1 ) {
				g_WadList = g_slist_append( g_WadList, strdup( p1 ) );
				Sys_Printf( "wad: %s\n",p1 );
			}
		}
		if ( p2 ) {
			p1 = p2 + 1; // point back to the remainder of the string
		}
		else{
			p1 = NULL; // make it so we exit the loop.

		}
	} while ( p1 );

	// strip the ".wad" extensions.
	for ( GSList *l = g_WadList; l != NULL ; l = l->next )
	{
		p1 = (char *)l->data;

		if ( p1[strlen( p1 ) - 4] == '.' ) {
			p1[strlen( p1 ) - 4] = 0;
		}
	}
}

// FIXME: usefulness of this cache sounds very discutable
char *CheckCacheForTextureName( const char *cleantexturename ){
	char *str;
	int len;
	GSList *l;

	// search our little cache first to speed things up.
	// cache strings are stored as "<cleanname>;<actualnameshader>"
	len = strlen( cleantexturename );
	for ( l = g_TextureNameCache; l != NULL ; l = l->next )
	{
		str = (char *)l->data;
		if ( ( strnicmp( cleantexturename,str,len ) == 0 ) && ( str[len] == ';' ) ) { // must do in this order or we'll get an access violation, even though it's slower.
			return ( str + len + 1 ); // skip the delimiter ;
		}
	}
	return NULL;
}

char *AddToCache( const char *cleantexturename, const char *actualname ){
	char *cachestr;
	cachestr = (char *)malloc( strlen( cleantexturename ) + 1 + strlen( actualname ) + 1 ); // free()'d when g_TextureNameCache is freed
	sprintf( cachestr,"%s;%s",cleantexturename,actualname );
	g_TextureNameCache = g_slist_append( g_TextureNameCache, cachestr );
	return cachestr;
}

char *SearchWadsForTextureName( const char *cleantexturename ){
	char *str;
	char *wadname;
	char *actualtexturename = NULL;
	GSList *l;
	int count;

	actualtexturename = CheckCacheForTextureName( cleantexturename );
	if ( actualtexturename ) {
		return actualtexturename;
	}

	// still here ?  guess it's not in the cache then!

	// search the wads listed in the worldspawn "wad" key
	for ( l = g_WadList; l != NULL && actualtexturename == NULL ; l = l->next )
	{
		wadname = (char *)l->data;

		str = new char[strlen( wadname ) + strlen( cleantexturename ) + 9 + 1 + 4 + 1];

		// hlw here is ok as we never have anything other than hlw files in a wad.
		sprintf( str,"textures/%s/%s.hlw",wadname,cleantexturename );
		count = vfsGetFileCount( str, VFS_SEARCH_PAK ); // only search pack files
		// LordHavoc: hacked in .mip loading here
		if ( !count ) {
			sprintf( str,"textures/%s/%s.mip",wadname,cleantexturename );
			count = vfsGetFileCount( str, VFS_SEARCH_PAK ); // only search pack files
		}

		if ( count > 0 ) {
			// strip the extension, build the cache string and add the the cache
			str[strlen( str ) - 4] = 0;

			actualtexturename = AddToCache( cleantexturename,str );

			//point the return value to the actual name, not what we add to the cache
			actualtexturename += 1 + strlen( cleantexturename );
		}
		delete [] str;
	}
	return actualtexturename;
}
// End of half-life specific stuff

void Patch_Parse( patchMesh_t *pPatch ){
	int i, j;
	char *str;

	char *token = Token();

	GetToken( true ); //{

	// parse shader name
	GetToken( true );
	str = new char[strlen( token ) + 10];
	strcpy( str, "textures/" );
	strcpy( str + 9, token );
	pPatch->pShader = QERApp_Shader_ForName( str );
	pPatch->d_texture = pPatch->pShader->getTexture();
	delete [] str;

	GetToken( true ); //(

	// parse matrix dimensions
	GetToken( false );
	pPatch->width = atoi( token );
	if ( pPatch->width > MAX_PATCH_WIDTH ) {
		Syn_Printf( "ERROR: patch has too many planes, patch width > MAX_PATCH_WIDTH (%i > %i)\n", pPatch->width, MAX_PATCH_WIDTH );
		pPatch->width = MAX_PATCH_WIDTH;
		abortcode = MAP_ABORTED;
	}
	GetToken( false );
	pPatch->height = atoi( token );
	if ( pPatch->height > MAX_PATCH_HEIGHT ) {
		Syn_Printf( "ERROR: patch has too many plane points, patch height > MAX_PATCH_HEIGHT (%i > %i)\n", pPatch->height, MAX_PATCH_HEIGHT );
		pPatch->height = MAX_PATCH_HEIGHT;
		abortcode = MAP_ABORTED;
	}

	// ignore contents/flags/value
	GetToken( false );
	GetToken( false );
	GetToken( false );

	GetToken( false ); //)

	// parse matrix
	GetToken( true ); //(
	for ( i = 0; i < pPatch->width; i++ )
	{
		GetToken( true ); //(
		for ( j = 0; j < pPatch->height; j++ )
		{
			GetToken( false ); //(

			GetToken( false );
			pPatch->ctrl[i][j].xyz[0] = atof( token );
			GetToken( false );
			pPatch->ctrl[i][j].xyz[1] = atof( token );
			GetToken( false );
			pPatch->ctrl[i][j].xyz[2] = atof( token );
			GetToken( false );
			pPatch->ctrl[i][j].st[0] = atof( token );
			GetToken( false );
			pPatch->ctrl[i][j].st[1] = atof( token );

			GetToken( false ); //)
		}
		GetToken( false ); //)
	}
	GetToken( true ); //)

	GetToken( true ); //}
}

void Face_Parse( face_t *face, bool bAlternateTexdef = false ){
	int i, j;
	char *str;
	bool bworldcraft = false;

	char *token = Token();

	// parse planepts
	str = NULL;
	for ( i = 0; i < 3; i++ )
	{
		GetToken( true ); //(
		for ( j = 0; j < 3; j++ )
		{
			GetToken( false );
			face->planepts[i][j] = atof( token );
		}
		GetToken( false ); //)
	}

	if ( bAlternateTexdef ) {
		// parse alternate texdef
		GetToken( false ); // (
		GetToken( false ); // (
		for ( i = 0; i < 3; i++ )
		{
			GetToken( false );
			face->brushprimit_texdef.coords[0][i] = atof( token );
		}
		GetToken( false ); // )
		GetToken( false ); // (
		for ( i = 0; i < 3; i++ )
		{
			GetToken( false );
			face->brushprimit_texdef.coords[1][i] = atof( token );
		}
		GetToken( false ); // )
		GetToken( false ); // )
	}


	// parse shader name
	GetToken( false ); // shader

	// if we're loading a halflife map then we don't have a relative texture name
	// we just get <texturename>.  So we need to convert this to a relative name
	// like this: "textures/<wadname>/shader", so we use vfsFileFile to get the filename.

	// *** IMPORTANT ***
	// For Halflife we need to see if the texture is in wads listed in the
	// map's worldspawn "wad" e-pair.  If we don't then the image used will be the
	// first image with this texture name that is found in any of the wads on the
	// user's system.  this is not a huge problem, because the map compiler obeys
	// the "wad" epair when compiling the map, but the user might end up looking at
	// the wrong texture in the editor. (more of a problem if the texture we use
	// here has a different size from the one in the wad the map compiler uses...)

	// Hydra: - TTimo: I looked all over for other places to put this, but really it
	// is an issue with map loading (because of a limitation of halflife/q2 map format)
	// so it's gone in here, it also stops incorrect shader/texdef names getting used
	// in the radiant core and it's modules which we'd only have to change later on.
	// (either in map_importentities() or the shader module).  so it's actually cleaner
	// in the long run, even if a little odd.  And it keeps more game specific stuff
	// OUT of the core, which is a good thing.

	if ( g_MapVersion == MAPVERSION_HL ) {
		qboolean done = false;

		// FIXME: This bit is halflife specific.
		// look in the list of wads supplied in the worldspawn "wad" key/pair for the
		// texture first, if it's not in any then we carry on searching the vfs for it
		// as usual.

		// each time we find a texture, we add it to the a cache
		// so we don't have to hunt the vfs for it each time.
		// See SearchWadsForTextureName() and AddToCache() above for cache stuff

		char *wadname;
		wadname = SearchWadsForTextureName( token );

		if ( wadname ) {
			face->texdef.SetName( wadname );
			done = true;
		}
		else
		{
			// using the cache below means that this message is only ever printed out once!
			Sys_FPrintf( SYS_WRN, "WARNING: could not find \"%s\" in any listed wad files, searching all wad files instead!\n",token );
		}
		// end of half-life specific bit.

		// check the cache!
		if ( !done ) {
			str = CheckCacheForTextureName( token );
			if ( str ) {
				face->texdef.SetName( str );
				done = true;
			}
		}

		if ( !done ) {
			char *fullpath;

			str = new char[strlen( token ) + 4 + 1];

			// FIXME: halflife specific file extension, we'll have to support Q2/Q1 formats
			// and maybe tga texture format for HL here too..
			sprintf( str,"%s.hlw",token );
			fullpath = vfsGetFullPath( str,0,VFS_SEARCH_PAK | VFS_SEARCH_DIR );

			// MIP support for quake
			if ( !fullpath ) {
				sprintf( str,"%s.mip",token );
				fullpath = vfsGetFullPath( str, 0, 0 );
			}

			// TGA support in halflife ?
			/*
			   if (!fullpath)
			   {
			   sprintf(str,"%s.tga",token);
			   fullpath = vfsGetFullPath(str);
			   }
			 */
			delete [] str;

			if ( fullpath ) {
				// strip the extension.
				int len = strlen( fullpath );
				if ( fullpath[len - 4] == '.' ) {
					fullpath[len - 4] = '\0';
				}

				// and set the correct name!
				face->texdef.SetName( fullpath );
				AddToCache( token,fullpath );
			}
			else
			{
				Sys_FPrintf( SYS_WRN, "WARNING: could not find \"%s\" in the vfs search path\n",token );
				str = new char[strlen( token ) + 10];
				strcpy( str, "textures/" );
				strcpy( str + 9, token );
				face->texdef.SetName( str );
				AddToCache( token,str );
				delete [] str;
			}
		}
	}
	else // !MAPVERSION_HL
	{
		str = new char[strlen( token ) + 10];
		strcpy( str, "textures/" );
		strcpy( str + 9, token );
		face->texdef.SetName( str );
		delete [] str;
	}

	if ( !bAlternateTexdef ) {
		if ( g_MapVersion == MAPVERSION_HL ) { // Q1 as well ?
			GetToken( false );
			if ( token[0] == '[' && token[1] == '\0' ) {
				bworldcraft = true;

				GetToken( false ); // UAxis[0]
				GetToken( false ); // UAxis[1]
				GetToken( false ); // UAxis[2]

				GetToken( false ); // shift
				face->texdef.shift[0] = atof( token );

				GetToken( false ); // ]

				GetToken( false ); // [
				GetToken( false ); // VAxis[0]
				GetToken( false ); // VAxis[1]
				GetToken( false ); // VAxis[2]

				GetToken( false ); // shift
				face->texdef.shift[1] = atof( token );

				GetToken( false ); // ]

				// rotation is derived from the U and V axes.
				// ZHLT ignores this setting even if present in a .map file.
				GetToken( false );
				face->texdef.rotate = atof( token );

				// Scales
				GetToken( false );
				face->texdef.scale[0] = atof( token );
				GetToken( false );
				face->texdef.scale[1] = atof( token );
			}
			else
			{
				UnGetToken();
			}
		}

		if ( !bworldcraft ) { // !MAPVERSION_HL
			// parse texdef
			GetToken( false );
			face->texdef.shift[0] = atof( token );
			GetToken( false );
			face->texdef.shift[1] = atof( token );
			GetToken( false );
			face->texdef.rotate = atof( token );
			GetToken( false );
			face->texdef.scale[0] = atof( token );
			GetToken( false );
			face->texdef.scale[1] = atof( token );
		}
	}
	// parse the optional contents/flags/value
	if ( !bworldcraft && TokenAvailable() ) {
		GetToken( true );
		if ( isdigit( token[0] ) ) {
			face->texdef.contents = atoi( token );
			GetToken( false );
			face->texdef.flags = atoi( token );
			GetToken( false );
			face->texdef.value = atoi( token );
		}
		else
		{
			UnGetToken();
		}
	}
}

bool Primitive_Parse( brush_t *pBrush ){
	char *token = Token();

	GetToken( true );
	if ( !strcmp( token, "patchDef2" ) ) {
		pBrush->patchBrush = true;
		pBrush->pPatch = Patch_Alloc();
		pBrush->pPatch->pSymbiot = pBrush;
		Patch_Parse( pBrush->pPatch );
		GetToken( true ); //}

		// A patchdef should never be loaded from a quake2 map file
		// so we just return false and the brush+patch gets freed
		// and the user gets told.
		if ( g_MapVersion != MAPVERSION_Q3 ) {
			// FIXME: Hydra - I wanted to write out a line number here, but I can't because there's no API to access the core's "scriptline" variable.
			Syn_Printf( "ERROR: patchDef2's are not supported in Quake%d format .map files!\n",g_MapVersion );
			abortcode = MAP_WRONGVERSION;
			return false;
		}
	}
	else if ( !strcmp( token, "brushDef" ) ) {
		pBrush->bBrushDef = true;
		GetToken( true ); // {
		while ( 1 )
		{
			face_t    *f = pBrush->brush_faces;
			pBrush->brush_faces = Face_Alloc();
			Face_Parse( pBrush->brush_faces, true );
			pBrush->brush_faces->next = f;
			// check for end of brush
			GetToken( true );
			if ( strcmp( token,"}" ) == 0 ) {
				break;
			}
			UnGetToken();
		}
		GetToken( true ); // }
	}
	else
	{
		UnGetToken();
		while ( 1 )
		{
			face_t    *f = pBrush->brush_faces;
			pBrush->brush_faces = Face_Alloc();
			Face_Parse( pBrush->brush_faces );
			pBrush->brush_faces->next = f;

			// check for end of brush
			GetToken( true );
			if ( strcmp( token,"}" ) == 0 ) {
				break;
			}
			UnGetToken();
		}
	}
	return true;
}

void Entity_Parse( entity_t *pEntity ){
	brush_t *pBrush;
//  CPtrArray *brushes = NULL;
	char temptoken[1024];

	char *token = Token();

	while ( 1 )
	{
		GetToken( true ); // { or } or epair
		if ( !strcmp( token, "}" ) ) {
			break;
		}
		else if ( !strcmp( token, "{" ) ) {

			pBrush = Brush_Alloc();
			if ( Primitive_Parse( pBrush ) ) {
				( (CPtrArray*)pEntity->pData )->Add( pBrush );
			}
			else {
				Brush_Free( pBrush, true );
			}

		}
		else {

			strcpy( temptoken, token );
			GetToken( false );

			SetKeyValue( pEntity, temptoken, token );

			if ( g_MapVersion == MAPVERSION_HL ) {
				// if we've not god a "wads" key/pair already, then break it into a list.
				if ( !g_WadList && ( stricmp( temptoken,"wad" ) == 0 ) ) {
					BuildWadList( token );
				}
			}

		}
	}
}

void Map_Read( IDataStream *in, CPtrArray *map ){
	entity_t *pEntity;
	char *buf;

	unsigned long len = in->GetLength();
	buf = new char[len + 1];
	in->Read( buf, len );
	buf[len] = '\0';
	StartTokenParsing( buf );
	abortcode = MAP_NOERROR;

	while ( abortcode == MAP_NOERROR )
	{
		if ( !GetToken( true ) ) { // { or NULL
			break;
		}
		pEntity = Entity_Alloc();
		pEntity->pData = new CPtrArray;
		Entity_Parse( pEntity );
		map->Add( pEntity );
	}

	delete [] buf;

	if ( abortcode != MAP_NOERROR ) {
		int num_ents, num_brushes,i,j;
		entity_t *e;
		CPtrArray *brushes;

		num_ents = map->GetSize();
		for ( i = 0; i < num_ents; i++ )
		{
			e = (entity_t*)map->GetAt( i );
			brushes = (CPtrArray*)e->pData;
			num_brushes = brushes->GetSize();
			for ( j = 0; j < num_brushes; j++ )
			{
				Brush_Free( (brush_t *)brushes->GetAt( j ), true );
			}
			brushes->RemoveAll();
			delete brushes;
			Entity_Free( e );
		}
		map->RemoveAll();
	}
}

void Map_ReadQ3( IDataStream *in, CPtrArray *map ){
	g_MapVersion = MAPVERSION_Q3;
	Map_Read( in,map );
}

void Map_ReadHL( IDataStream *in, CPtrArray *map ){
	g_WadList = NULL;
	g_TextureNameCache = NULL;

	g_MapVersion = MAPVERSION_HL;
	Map_Read( in,map );

	FreeGSList( g_TextureNameCache );
	FreeGSList( g_WadList );
}

void Map_ReadQ2( IDataStream *in, CPtrArray *map ){
	g_MapVersion = MAPVERSION_Q2;
	Map_Read( in,map );
}
