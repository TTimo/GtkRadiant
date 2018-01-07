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
#include <glib/gi18n.h>
#include <string.h>
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <unistd.h>
#endif
#include "preferences.h"
#include "mainframe.h"
#include "gtkmisc.h"
#include "filters.h"

extern MainFrame* g_pParentWnd;

int modified;   // for quit confirmation (0 = clean, 1 = unsaved,
// 2 = autosaved, but not regular saved)

char currentmap[1024];

brush_t active_brushes;     // brushes currently being displayed
brush_t selected_brushes;   // highlighted

face_t  *selected_face;
brush_t *selected_face_brush;

brush_t filtered_brushes;   // brushes that have been filtered or regioned

entity_t entities;          // head/tail of doubly linked list

entity_t    *world_entity = NULL; // "classname" "worldspawn" !

void Map_Init(){
	Map_Free();
}


bool g_bCancel_Map_LoadFile;  // Hydra: moved this here

// TTimo
// need that in a variable, will have to tweak depending on the game
vec_t g_MaxWorldCoord = 64 * 1024;
vec_t g_MinWorldCoord = -64 * 1024;

// the max size we allow on brushes, this is dependant on world coords too
// makes more sense to say smaller I think?
vec_t g_MaxBrushSize = ( g_MaxWorldCoord - 1 ) * 2;

void AddRegionBrushes( void );
void RemoveRegionBrushes( void );

/*
   =============================================================

   Cross map selection saving

   this could fuck up if you have only part of a complex entity selected...
   =============================================================
 */

brush_t between_brushes;
entity_t between_entities;

bool g_bRestoreBetween = false;

void Map_SaveBetween( void ){
	if ( g_pParentWnd->ActiveXY() ) {
		g_bRestoreBetween = true;
		g_pParentWnd->ActiveXY()->Copy();
	}
	return;
}

void Map_RestoreBetween( void ){
	if ( g_pParentWnd->ActiveXY() && g_bRestoreBetween ) {
		g_pParentWnd->ActiveXY()->Paste();
	}
}

//============================================================================

bool CheckForTinyBrush( brush_t* b, int n, float fSize ){
	bool bTiny = false;
	for ( int i = 0 ; i < 3 ; i++ )
	{
		if ( b->maxs[i] - b->mins[i] < fSize ) {
			bTiny = true;
		}
	}
	if ( bTiny ) {
		Sys_Printf( "Possible problem brush (too small) #%i ", n );
	}
	return bTiny;
}

void Map_BuildBrushData( void ){
	brush_t *b, *next;

	if ( active_brushes.next == NULL ) {
		return;
	}

	Sys_BeginWait(); // this could take a while

	int n = 0;
	for ( b = active_brushes.next ; b != NULL && b != &active_brushes ; b = next )
	{
		next = b->next;
		Brush_Build( b, true, false, false );
		if ( !b->brush_faces || ( g_PrefsDlg.m_bCleanTiny && CheckForTinyBrush( b, n++, g_PrefsDlg.m_fTinySize ) ) ) {
			Brush_Free( b );
			Sys_Printf( "Removed degenerate brush\n" );
		}
	}
	Sys_EndWait();
}

entity_t *Map_FindClass( const char *cname ){
	entity_t    *ent;

	for ( ent = entities.next ; ent != &entities ; ent = ent->next )
	{
		if ( !strcmp( cname, ValueForKey( ent, "classname" ) ) ) {
			return ent;
		}
	}
	return NULL;
}

/*
   ================
   Map_Free
   free all map elements, reinitialize the structures that depend on them
   ================
 */
void Map_Free( void ){
	g_bRestoreBetween = false;
	if ( selected_brushes.next &&
		 ( selected_brushes.next != &selected_brushes ) ) {
		if ( gtk_MessageBox( g_pParentWnd->m_pWidget, _( "Copy selection?" ), " ", MB_YESNO ) == IDYES ) {
			Map_SaveBetween();
		}
	}

	QERApp_ActiveShaders_SetInUse( false );
	Pointfile_Clear();
	g_qeglobals.d_num_entities = 0;

	if ( !active_brushes.next ) {
		// first map
		active_brushes.prev = active_brushes.next = &active_brushes;
		selected_brushes.prev = selected_brushes.next = &selected_brushes;
		filtered_brushes.prev = filtered_brushes.next = &filtered_brushes;
		entities.prev = entities.next = &entities;
	}
	else
	{
		// free selected faces array
		g_ptrSelectedFaces.RemoveAll();
		g_ptrSelectedFaceBrushes.RemoveAll();
		while ( active_brushes.next != &active_brushes )
			Brush_Free( active_brushes.next );
		while ( selected_brushes.next != &selected_brushes )
			Brush_Free( selected_brushes.next );
		while ( filtered_brushes.next != &filtered_brushes )
			Brush_Free( filtered_brushes.next );
		while ( entities.next != &entities )
			Entity_Free( entities.next );
	}

	if ( world_entity ) {
		Entity_Free( world_entity );
	}
	world_entity = NULL;
}

entity_t *AngledEntity(){
	entity_t *ent = Map_FindClass( "info_player_start" );
	if ( !ent ) {
		ent = Map_FindClass( "info_player_deathmatch" );
	}
	if ( !ent ) {
		ent = Map_FindClass( "info_player_deathmatch" );
	}
	if ( !ent ) {
		ent = Map_FindClass( "team_CTF_redplayer" );
	}
	if ( !ent ) {
		ent = Map_FindClass( "team_CTF_blueplayer" );
	}
	if ( !ent ) {
		ent = Map_FindClass( "team_CTF_redspawn" );
	}
	if ( !ent ) {
		ent = Map_FindClass( "team_CTF_bluespawn" );
	}
	return ent;
}

//
// move the view to a start position
//
void Map_StartPosition(){
	entity_t *ent = AngledEntity();

	g_pParentWnd->GetCamWnd()->Camera()->angles[PITCH] = 0;
	if ( ent ) {
		GetVectorForKey( ent, "origin", g_pParentWnd->GetCamWnd()->Camera()->origin );
		GetVectorForKey( ent, "origin", g_pParentWnd->GetXYWnd()->GetOrigin() );
		g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] = FloatForKey( ent, "angle" );
	}
	else
	{
		g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] = 0;
		VectorCopy( vec3_origin, g_pParentWnd->GetCamWnd()->Camera()->origin );
		VectorCopy( vec3_origin, g_pParentWnd->GetXYWnd()->GetOrigin() );
	}
}

void Map_FreeEntities( CPtrArray *ents ){
	int i, j, num_ents, num_brushes;
	entity_t* e;
	CPtrArray* brushes;

	num_ents = ents->GetSize();
	for ( i = 0; i < num_ents; i++ )
	{
		e = (entity_t*)ents->GetAt( i );
		brushes = (CPtrArray*)e->pData;
		num_brushes = brushes->GetSize();
		for ( j = 0; j < num_brushes; j++ )
			Brush_Free( (brush_t*)brushes->GetAt( j ) );
		brushes->RemoveAll();
		delete (CPtrArray*)e->pData;
		e->pData = NULL;
		Entity_Free( e );
	}
	ents->RemoveAll();
}

/*!\todo Possibly make the import Undo-friendly by calling Undo_End for new brushes and ents */
void Map_ImportEntities( CPtrArray *ents, bool bAddSelected = false ){
	int num_ents, num_brushes;
	CPtrArray *brushes;
	vec3_t mins, maxs;
	entity_t *e;
	brush_t *b;
	face_t *f;
	int i,j;

	GPtrArray *new_ents = g_ptr_array_new();

	g_qeglobals.bPrimitBrushes = false;

	brush_t *pBrushList = ( bAddSelected ) ? &selected_brushes : &active_brushes;

	bool bDoneBPCheck = false;
	g_qeglobals.bNeedConvert = false;
	// HACK: find out if this map file was a BP one
	// check the first brush in the file that is NOT a patch
	// this will not be necessary when we allow both formats in the same file
	num_ents = ents->GetSize();
	for ( i = 0; !bDoneBPCheck && i < num_ents; i++ )
	{
		e = (entity_t*)ents->GetAt( i );
		brushes = (CPtrArray*)e->pData;
		num_brushes = brushes->GetSize();
		for ( j = 0; !bDoneBPCheck && j < num_brushes; j++ )
		{
			/*!todo Allow mixing texdef formats per-face. */
			b = (brush_t *)brushes->GetAt( j );
			if ( b->patchBrush ) {
				continue;
			}
			bDoneBPCheck = true;
			int BP_param = -1;
			if ( b->bBrushDef && !g_qeglobals.m_bBrushPrimitMode ) {
				BP_param = 0;
			}
			else if ( !b->bBrushDef && g_qeglobals.m_bBrushPrimitMode ) {
				BP_param = 1;
			}

			if ( BP_param != -1 ) {
				switch ( BP_MessageBox( BP_param ) )
				{
				case 0:
					Map_FreeEntities( ents );
					return;
				case 1:
					g_qeglobals.bNeedConvert = true;
					break;
				case 2:
					g_qeglobals.bNeedConvert = false;
					break;
				}
			}
		}
	}

	// process the entities into the world geometry
	num_ents = ents->GetSize();
	for ( i = 0; i < num_ents; i++ )
	{
		num_brushes = 0;
		e = (entity_t*)ents->GetAt( i );
		brushes = (CPtrArray*)e->pData;

		num_brushes = brushes->GetSize();
		// link brushes into entity
		for ( j = 0; j < num_brushes; j++ )
		{
			Entity_LinkBrush( e, (brush_t *)brushes->GetAt( j ) );
			g_qeglobals.d_parsed_brushes++;
		}
		brushes->RemoveAll();
		delete brushes;
		e->pData = NULL;

		// set entity origin
		GetVectorForKey( e, "origin", e->origin );
		// set entity eclass
		/*!\todo Make SetKeyValue check for "classname" change and assign appropriate eclass */
		e->eclass = Eclass_ForName( ValueForKey( e, "classname" ),
									( e->brushes.onext != &e->brushes ) );

		// go through all parsed brushes and build stuff
		for ( b = e->brushes.onext; b != &e->brushes; b = b->onext )
		{
			for ( f = b->brush_faces; f != NULL; f = f->next )
			{
				f->pShader = QERApp_Shader_ForName( f->texdef.GetName() );
				f->d_texture = f->pShader->getTexture();
			}

			// when brushes are in final state, build the planes and windings
			// NOTE: also converts BP brushes if g_qeglobals.bNeedConvert is true
			Brush_Build( b );
		}

//#define TERRAIN_HACK
#undef TERRAIN_HACK

#ifdef TERRAIN_HACK
		if ( ( strcmp( ValueForKey( e, "terrain" ),"1" ) == 0 && strcmp( e->eclass->name,"func_group" ) == 0 ) ) {

			// two aux pointers to the shaders used in the terrain entity
			// we don't keep refcount on them since they are only temporary
			// this avoids doing expensive lookups by name for all faces
			IShader *pTerrainShader, *pCaulk;

			pTerrainShader = NULL;
			pCaulk = QERApp_Shader_ForName( SHADER_CAULK );

			for ( b = e->brushes.onext; b != &e->brushes; b = b->onext )
			{
				if ( pTerrainShader == NULL ) {
					for ( f = b->brush_faces; f != NULL; f = f->next )
						if ( strcmp( f->texdef.GetName(), SHADER_CAULK ) != 0 ) {
							pTerrainShader = f->pShader;
						}
				}

				if ( pTerrainShader ) {
					for ( f = b->brush_faces; f != NULL; f = f->next )
					{
						if ( strcmp( f->texdef.GetName(), SHADER_CAULK ) != 0 ) { // not caulk
							Face_SetShader( f, pTerrainShader->getName() );
						}
						else{
							Face_SetShader( f, pCaulk->getName() );
						}
					}
				}
				else{
					Sys_FPrintf( SYS_WRN, "WARNING: no terrain shader found for brush\n" );
				}
			}
		}
#endif

#define PATCH_HACK
#ifdef PATCH_HACK
		for ( b = e->brushes.onext; b != &e->brushes; b = b->onext )
		{
			// patch hack, to be removed when dependency on brush_faces is removed
			if ( b->patchBrush ) {
				Patch_CalcBounds( b->pPatch, mins, maxs );
				for ( int i = 0; i < 3; i++ )
				{
					if ( (int)mins[i] == (int)maxs[i] ) {
						mins[i] -= 4;
						maxs[i] += 4;
					}
				}
				Brush_Resize( b, mins, maxs );
				Brush_Build( b );
			}
		}
#endif
		// add brush for fixedsize entity
		if ( e->eclass->fixedsize ) {
			vec3_t mins, maxs;
			VectorAdd( e->eclass->mins, e->origin, mins );
			VectorAdd( e->eclass->maxs, e->origin, maxs );
			b = Brush_Create( mins, maxs, &e->eclass->texdef );
			Entity_LinkBrush( e, b );
			Brush_Build( b );
		}

		for ( b = e->brushes.onext; b != &e->brushes; b = b->onext )
			Brush_AddToList( b, pBrushList );

		if ( strcmp( e->eclass->name, "worldspawn" ) == 0 ) {
			if ( world_entity ) {
				while ( e->brushes.onext != &e->brushes )
				{
					b = e->brushes.onext;
					Entity_UnlinkBrush( b );
					Entity_LinkBrush( world_entity, b );
				}
				Entity_Free( e );
			}
			else
			{
				world_entity = e;
			}
		}
		else if ( strcmp( e->eclass->name, "group_info" ) == 0 ) {
			// it's a group thing!
			Group_Add( e );
			Entity_Free( e );
		}
		else
		{
			// fix target/targetname collisions
			if ( ( g_PrefsDlg.m_bDoTargetFix ) && ( strcmp( ValueForKey( e, "target" ), "" ) != 0 ) ) {
				GPtrArray *t_ents = g_ptr_array_new();
				entity_t *e_target;
				const char *target = ValueForKey( e, "target" );
				qboolean bCollision = FALSE;

				// check the current map entities for an actual collision
				for ( e_target = entities.next; e_target != &entities; e_target = e_target->next )
				{
					if ( !strcmp( target, ValueForKey( e_target, "target" ) ) ) {
						bCollision = TRUE;
						// make sure the collision is not between two imported entities
						for ( j = 0; j < (int)new_ents->len; j++ )
						{
							if ( e_target == g_ptr_array_index( new_ents, j ) ) {
								bCollision = FALSE;
							}
						}
					}
				}

				// find the matching targeted entity(s)
				if ( bCollision ) {
					for ( j = num_ents - 1; j > 0; j-- )
					{
						e_target = (entity_t*)ents->GetAt( j );
						if ( e_target != NULL && e_target != e ) {
							const char *targetname = ValueForKey( e_target, "targetname" );
							if ( ( targetname != NULL ) && ( strcmp( target, targetname ) == 0 ) ) {
								g_ptr_array_add( t_ents, (gpointer)e_target );
							}
						}
					}
					if ( t_ents->len > 0 ) {
						// link the first to get a unique target/targetname
						Entity_Connect( e, (entity_t*)g_ptr_array_index( t_ents,0 ) );
						// set the targetname of the rest of them manually
						for ( j = 1; j < (int)t_ents->len; j++ )
							SetKeyValue( (entity_t*)g_ptr_array_index( t_ents, j ), "targetname", ValueForKey( e, "target" ) );
					}
					g_ptr_array_free( t_ents, FALSE );
				}
			}

			// add the entity to the end of the entity list
			Entity_AddToList( e, &entities );
			g_qeglobals.d_num_entities++;

			// keep a list of ents added to avoid testing collisions against them
			g_ptr_array_add( new_ents, (gpointer)e );
		}
	}
	g_ptr_array_free( new_ents, FALSE );

	ents->RemoveAll();

	g_qeglobals.bNeedConvert = false;
}

void Map_Import( IDataStream *in, const char *type, bool bAddSelected ){
	CPtrArray ents;

	g_pParentWnd->GetSynapseClient().ImportMap( in, &ents, type );
	Map_ImportEntities( &ents, bAddSelected );
}

/*
   ================
   Map_LoadFile
   ================
 */
void Map_LoadFile( const char *filename ){
	clock_t start, finish;
	double elapsed_time;
	start = clock();

	Sys_BeginWait();
	Select_Deselect();
	/*!
	   \todo FIXME TTimo why is this commented out?
	   stability issues maybe? or duplicate feature?
	   forcing to show the console during map load was a good thing IMO
	 */
	//SetInspectorMode(W_CONSOLE);
	Sys_Printf( "Loading map from %s\n", filename );

	Map_Free();
	//++timo FIXME: maybe even easier to have Group_Init called from Map_Free?
	Group_Init();
	g_qeglobals.d_num_entities = 0;
	g_qeglobals.d_parsed_brushes = 0;


	// cancel the map loading process
	// used when conversion between standard map format and BP format is required and the user cancels the process
	g_bCancel_Map_LoadFile = false;

	strcpy( currentmap, filename );

	g_bScreenUpdates = false; // leo: avoid redraws while loading the map (see fenris:1952)

	// prepare to let the map module do the parsing
	FileStream file;
	const char* type = strrchr( filename,'.' );
	if ( type != NULL ) {
		type++;
	}
	// NOTE TTimo opening has binary doesn't make a lot of sense
	// but opening as text confuses the scriptlib parser
	// this may be a problem if we "rb" and use the XML parser, might have an incompatibility
	if ( file.Open( filename, "rb" ) ) {
		Map_Import( &file, type );
	}
	else{
		Sys_FPrintf( SYS_ERR, "ERROR: failed to open %s for read\n", filename );
	}
	file.Close();

	g_bScreenUpdates = true;

	if ( g_bCancel_Map_LoadFile ) {
		Sys_Printf( "Map_LoadFile canceled\n" );
		Map_New();
		Sys_EndWait();
		return;
	}

	if ( !world_entity ) {
		Sys_Printf( "No worldspawn in map.\n" );
		Map_New();
		Sys_EndWait();
		return;
	}
	finish = clock();
	elapsed_time = (double)( finish - start ) / CLOCKS_PER_SEC;

	Sys_Printf( "--- LoadMapFile ---\n" );
	Sys_Printf( "%s\n", filename );

	Sys_Printf( "%5i brushes\n",  g_qeglobals.d_parsed_brushes );
	Sys_Printf( "%5i entities\n", g_qeglobals.d_num_entities );
	Sys_Printf( "%5.2f second(s) load time\n", elapsed_time );

	Sys_EndWait();

	Map_RestoreBetween();

	//
	// move the view to a start position
	//
	Map_StartPosition();

	Map_RegionOff();

	modified = false;
	Sys_SetTitle( filename );

	Texture_ShowInuse();
	QERApp_SortActiveShaders();

	Sys_UpdateWindows( W_ALL );
}

/*!
   ===========
   Supporting functions for Map_SaveFile, builds a CPtrArray with the filtered / non filtered brushes
   ===========
 */
void CleanFilter( entity_t *ent ){
	if ( ent->pData ) {
		delete static_cast<CPtrArray*>( ent->pData );
		ent->pData = NULL;
	}
}

/*!
   filters out the region brushes if necessary
   returns true if this entity as a whole is out of the region
   (if all brushes are filtered out, then the entity will be completely dropped .. except if it's worldspawn of course)
 */
bool FilterChildren( entity_t *ent, bool bRegionOnly = false, bool bSelectedOnly = false ){
	if ( ent->brushes.onext == &ent->brushes ) {
		return false;
	}
	// entity without a brush, ignore it... this can be caused by Undo

	// filter fixedsize ents by their eclass bounding box
	// don't add their brushes
	if ( ent->eclass->fixedsize ) {
		if ( bSelectedOnly && !IsBrushSelected( ent->brushes.onext ) ) {
			return false;
		}

		if ( bRegionOnly && region_active ) {
			for ( int i = 0 ; i < 3 ; i++ )
			{
				if ( ( ent->origin[i] + ent->eclass->mins[i] ) > region_maxs[i] ) {
					return false;
				}
				if ( ( ent->origin[i] + ent->eclass->maxs[i] ) < region_mins[i] ) {
					return false;
				}
			}
		}
	}
	else
	{
		for ( brush_t *b = ent->brushes.onext ; b != &ent->brushes ; b = b->onext )
		{
			// set flag to use brushprimit_texdef
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				b->bBrushDef = true;
			}
			else{
				b->bBrushDef = false;
			}

			// add brush, unless it's excluded by region
			if ( !( bRegionOnly && Map_IsBrushFiltered( b ) ) &&
				 !( bSelectedOnly && !IsBrushSelected( b ) ) ) {
				( (CPtrArray*)ent->pData )->Add( b );
			}
		}

		if ( ( (CPtrArray*)ent->pData )->GetSize() <= 0 ) {
			return false;
		}
	}
	return true;
}

entity_t *region_startpoint = NULL;
void Map_ExportEntities( CPtrArray* ents, bool bRegionOnly = false, bool bSelectedOnly = false ){
	int i;
	entity_t *e;

	/*!
	   \todo the entity_t needs to be reworked and asbtracted some more

	   keeping the entity_t as the struct providing access to a list of map objects, a list of epairs and various other info?
	   but separating some more the data that belongs to the entity_t and the 'sons' data
	   on a side note, I don't think that doing that with linked list would be a good thing

	   for now, we use the blind void* in entity_t casted to a CPtrArray of brush_t* to hand out a list of the brushes for map write
	   the next step is very likely to be a change of the brush_t* to a more abstract object?
	 */

	FilterChildren( world_entity, bRegionOnly, bSelectedOnly );
	ents->Add( world_entity );

	for ( e = entities.next ; e != &entities ; e = e->next )
	{
		// not sure this still happens, probably safe to leave it in
		if ( ( !strcmp( ValueForKey( e, "classname" ), "worldspawn" ) ) && ( e != world_entity ) ) {
			Sys_FPrintf( SYS_ERR, "Dropping parasite worldspawn entity\n" );
			continue;
		}

		// entities which brushes are completely filtered out by regioning are not printed to the map
		if ( FilterChildren( e, bRegionOnly, bSelectedOnly ) ) {
			ents->Add( e );
		}
	}

	if ( bRegionOnly && region_active ) {
		for ( i = 0; i < 6; i++ )
			( (CPtrArray*)world_entity->pData )->Add( region_sides[i] );

		ents->Add( region_startpoint );
	}
}

void Map_Export( IDataStream *out, const char *type, bool bRegionOnly, bool bSelectedOnly ){
	entity_t  *e;

	CPtrArray ents;

	if ( bRegionOnly && region_active ) {
		AddRegionBrushes();
	}

	// create the filters
	world_entity->pData = new CPtrArray();
	for ( e = entities.next; e != &entities; e = e->next )
		e->pData = new CPtrArray();

	Map_ExportEntities( &ents, bRegionOnly, bSelectedOnly );

	g_pParentWnd->GetSynapseClient().ExportMap( &ents, out, type );

	// cleanup the filters
	CleanFilter( world_entity );
	for ( e = entities.next ; e != &entities ; e = e->next )
		CleanFilter( e );

	if ( bRegionOnly && region_active ) {
		RemoveRegionBrushes();
	}
}

const char* filename_get_extension( const char* filename ){
	const char* type = strrchr( filename,'.' );
	if ( type != NULL ) {
		return ++type;
	}
	return "";
}

/*
   ===========
   Map_SaveFile
   \todo FIXME remove the use_region, this is broken .. work with a global flag to set region mode or not
   ===========
 */
void Map_SaveFile( const char *filename, qboolean use_region ){
	clock_t start, finish;
	double elapsed_time;
	start = clock();
	Sys_Printf( "Saving map to %s\n",filename );

	Pointfile_Clear();

	if ( !use_region ) {
		char backup[1024];

		// rename current to .bak
		strcpy( backup, filename );
		StripExtension( backup );
		strcat( backup, ".bak" );
		unlink( backup );
		rename( filename, backup );
	}

	Sys_Printf( "Map_SaveFile: %s\n", filename );

	// build the out data stream
	FileStream file;
	if ( !file.Open( filename,"w" ) ) {
		Sys_FPrintf( SYS_ERR, "ERROR: couldn't open %s for write\n", filename );
		return;
	}

	// extract filetype
	Map_Export( &file, filename_get_extension( filename ), use_region );

	file.Close();

	finish = clock();
	elapsed_time = (double)( finish - start ) / CLOCKS_PER_SEC;

	Sys_Printf( "Saved in %-.2f second(s).\n",elapsed_time );
	modified = false;

	if ( !strstr( filename, "autosave" ) ) {
		Sys_SetTitle( filename );
	}

	if ( !use_region ) {
		time_t timer;

		time( &timer );

		if( g_PrefsDlg.m_bSaveBeep ) {
			Sys_Beep();
		}
		Sys_Status( "Saved.", 0 );
	}
}

/*
   ===========
   Map_New

   ===========
 */
void Map_New( void ){
	Sys_Printf( "Map_New\n" );
	Map_Free();

	strcpy( currentmap, "unnamed.map" );
	Sys_SetTitle( currentmap );

	world_entity = (entity_s*)qmalloc( sizeof( *world_entity ) );
	world_entity->brushes.onext =
		world_entity->brushes.oprev = &world_entity->brushes;
	SetKeyValue( world_entity, "classname", "worldspawn" );
	world_entity->eclass = Eclass_ForName( "worldspawn", true );

	g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] = 0;
	g_pParentWnd->GetCamWnd()->Camera()->angles[PITCH] = 0;
	VectorCopy( vec3_origin, g_pParentWnd->GetCamWnd()->Camera()->origin );
	g_pParentWnd->GetCamWnd()->Camera()->origin[2] = 48;
	VectorCopy( vec3_origin, g_pParentWnd->GetXYWnd()->GetOrigin() );

	Map_RestoreBetween();

	Group_Init();

	Sys_UpdateWindows( W_ALL );
	modified = false;
}

/*
   ===========================================================

   REGION

   ===========================================================
 */
qboolean region_active;
vec3_t region_mins = {g_MinWorldCoord, g_MinWorldCoord, g_MinWorldCoord};
vec3_t region_maxs = {g_MaxWorldCoord, g_MaxWorldCoord, g_MaxWorldCoord};

brush_t *region_sides[6];

/*
   ===========
   AddRegionBrushes
   a regioned map will have temp walls put up at the region boundary
   \todo TODO TTimo old implementation of region brushes
   we still add them straight in the worldspawn and take them out after the map is saved
   with the new implementation we should be able to append them in a temporary manner to the data we pass to the map module
   ===========
 */
void AddRegionBrushes( void ){
	vec3_t mins, maxs;
	int i;
	texdef_t td;

	if ( !region_active ) {
#ifdef _DEBUG
		Sys_FPrintf( SYS_WRN, "Unexpected AddRegionBrushes call.\n" );
#endif
		return;
	}

	memset( &td, 0, sizeof( td ) );
	td.SetName( SHADER_NOT_FOUND );

	// set mins
	VectorSet( mins, region_mins[0] - 32, region_mins[1] - 32, region_mins[2] - 32 );

	// vary maxs
	for ( i = 0; i < 3; i++ )
	{
		VectorSet( maxs, region_maxs[0] + 32, region_maxs[1] + 32, region_maxs[2] + 32 );
		maxs[i] = region_mins[i];
		region_sides[i] = Brush_Create( mins, maxs, &td );
	}

	// set maxs
	VectorSet( maxs, region_maxs[0] + 32, region_maxs[1] + 32, region_maxs[2] + 32 );

	// vary mins
	for ( i = 0; i < 3; i++ )
	{
		VectorSet( mins, region_mins[0] - 32, region_mins[1] - 32, region_mins[2] - 32 );
		mins[i] = region_maxs[i];
		region_sides[i + 3] = Brush_Create( mins, maxs, &td );
	}


	// this is a safe check, but it should not really happen anymore
	vec3_t vOrig;
	VectorSet( vOrig,
			   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[0],
			   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[1],
			   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[2] );

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( vOrig[i] > region_maxs[i] || vOrig[i] < region_mins[i] ) {
			Sys_FPrintf( SYS_ERR, "Camera is NOT in the region, it's likely that the region won't compile correctly\n" );
		}
	}

	// write the info_playerstart
	region_startpoint = Entity_Alloc();
	SetKeyValue( region_startpoint, "classname", "info_player_start" );
	region_startpoint->eclass = Eclass_ForName( "info_player_start", false );
	char sTmp[1024];
	sprintf( sTmp, "%d %d %d", (int)vOrig[0], (int)vOrig[1], (int)vOrig[2] );
	SetKeyValue( region_startpoint, "origin", sTmp );
	sprintf( sTmp, "%d", (int)g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] );
	SetKeyValue( region_startpoint, "angle", sTmp );
	// empty array of children
	region_startpoint->pData = new CPtrArray;
}

void RemoveRegionBrushes( void ){
	int i;

	if ( !region_active ) {
		return;
	}
	for ( i = 0 ; i < 6 ; i++ )
		Brush_Free( region_sides[i] );

	CleanFilter( region_startpoint );
	Entity_Free( region_startpoint );
}

qboolean Map_IsBrushFiltered( brush_t *b ){
	int i;

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( b->mins[i] > region_maxs[i] ) {
			return true;
		}
		if ( b->maxs[i] < region_mins[i] ) {
			return true;
		}
	}
	return false;
}

/*
   ===========
   Map_RegionOff

   Other filtering options may still be on
   ===========
 */
void Map_RegionOff( void ){
	brush_t *b, *next;
	int i;

	region_active = false;
	for ( i = 0 ; i < 3 ; i++ )
	{
		region_maxs[i] = g_MaxWorldCoord - 64;
		region_mins[i] = g_MinWorldCoord + 64;
	}

	for ( b = filtered_brushes.next ; b != &filtered_brushes ; b = next )
	{
		next = b->next;
		if ( Map_IsBrushFiltered( b ) ) {
			continue;       // still filtered
		}
		Brush_RemoveFromList( b );
		if ( active_brushes.next == NULL || active_brushes.prev == NULL ) {
			active_brushes.next = &active_brushes;
			active_brushes.prev = &active_brushes;
		}
		Brush_AddToList( b, &active_brushes );
		b->bFiltered = FilterBrush( b );
	}
	Sys_UpdateWindows( W_ALL );
}

void Map_ApplyRegion( void ){
	brush_t *b, *next;

	region_active = true;
	for ( b = active_brushes.next ; b != &active_brushes ; b = next )
	{
		next = b->next;
		if ( !Map_IsBrushFiltered( b ) ) {
			continue;       // still filtered
		}
		Brush_RemoveFromList( b );
		Brush_AddToList( b, &filtered_brushes );
	}

	Sys_UpdateWindows( W_ALL );
}


/*
   ========================
   Map_RegionSelectedBrushes
   ========================
 */
void Map_RegionSelectedBrushes( void ){
	Map_RegionOff();

	if ( selected_brushes.next == &selected_brushes ) { // nothing selected
		Sys_Printf( "Tried to region with no selection...\n" );
		return;
	}
	region_active = true;
	Select_GetBounds( region_mins, region_maxs );

#ifdef _DEBUG
	if ( filtered_brushes.next != &filtered_brushes ) {
		Sys_FPrintf( SYS_WRN, "WARNING: filtered_brushes list may not be empty in Map_RegionSelectedBrushes\n" );
	}
#endif

	if ( active_brushes.next == &active_brushes ) {
		// just have an empty filtered_brushes list
		// this happens if you set region after selecting all the brushes in your map (some weird people do that, ask MrE!)
		filtered_brushes.next = filtered_brushes.prev = &filtered_brushes;
	}
	else
	{
		// move the entire active_brushes list to filtered_brushes
		filtered_brushes.next = active_brushes.next;
		filtered_brushes.prev = active_brushes.prev;
		filtered_brushes.next->prev = &filtered_brushes;
		filtered_brushes.prev->next = &filtered_brushes;
	}

	// move the entire selected_brushes list to active_brushes
	active_brushes.next = selected_brushes.next;
	active_brushes.prev = selected_brushes.prev;
	active_brushes.next->prev = &active_brushes;
	active_brushes.prev->next = &active_brushes;

	// deselect patches
	for ( brush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
		if ( b->patchBrush ) {
			b->pPatch->bSelected = false;
		}

	// clear selected_brushes
	selected_brushes.next = selected_brushes.prev = &selected_brushes;

	Sys_UpdateWindows( W_ALL );
}


/*
   ===========
   Map_RegionXY
   ===========
 */
void Map_RegionXY( void ){
	Map_RegionOff();

	region_mins[0] = g_pParentWnd->GetXYWnd()->GetOrigin()[0] - 0.5 * g_pParentWnd->GetXYWnd()->Width() / g_pParentWnd->GetXYWnd()->Scale();
	region_maxs[0] = g_pParentWnd->GetXYWnd()->GetOrigin()[0] + 0.5 * g_pParentWnd->GetXYWnd()->Width() / g_pParentWnd->GetXYWnd()->Scale();
	region_mins[1] = g_pParentWnd->GetXYWnd()->GetOrigin()[1] - 0.5 * g_pParentWnd->GetXYWnd()->Height() / g_pParentWnd->GetXYWnd()->Scale();
	region_maxs[1] = g_pParentWnd->GetXYWnd()->GetOrigin()[1] + 0.5 * g_pParentWnd->GetXYWnd()->Height() / g_pParentWnd->GetXYWnd()->Scale();
	region_mins[2] = g_MinWorldCoord + 64;
	region_maxs[2] = g_MaxWorldCoord - 64;
	Map_ApplyRegion();
}

/*
   ===========
   Map_RegionTallBrush
   ===========
 */
void Map_RegionTallBrush( void ){
	brush_t *b;

	if ( !QE_SingleBrush() ) {
		return;
	}

	b = selected_brushes.next;

	Map_RegionOff();

	VectorCopy( b->mins, region_mins );
	VectorCopy( b->maxs, region_maxs );
	region_mins[2] = g_MinWorldCoord + 64;
	region_maxs[2] = g_MaxWorldCoord - 64;

	Undo_Start( "delete" );
	Undo_AddBrushList( &selected_brushes );
	Undo_AddEntity( b->owner );
	Select_Delete();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();

	Map_ApplyRegion();
}

/*
   ===========
   Map_RegionBrush
   ===========
 */
void Map_RegionBrush( void ){
	brush_t *b;

	if ( !QE_SingleBrush() ) {
		return;
	}

	b = selected_brushes.next;

	Map_RegionOff();

	VectorCopy( b->mins, region_mins );
	VectorCopy( b->maxs, region_maxs );

	Undo_Start( "delete" );
	Undo_AddBrushList( &selected_brushes );
	Undo_AddEntity( b->owner );
	Select_Delete();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();

	Map_ApplyRegion();
}

GList *find_string( GList *glist, const char *buf ){
	while ( glist )
	{
		if ( strcmp( (char *)glist->data, buf ) == 0 ) {
			break; // this name is in our list already
		}
		glist = glist->next;
	}
	return glist;
}

void Map_ImportBuffer( char *buf ){
	Select_Deselect();

	Undo_Start( "import buffer" );

	MemStream stream;

	stream.Write( buf, strlen( buf ) );
	Map_Import( &stream, "xmap" );
	stream.Close();

	Sys_UpdateWindows( W_ALL );
	Sys_MarkMapModified();

	Undo_End();
}


//
//================
//Map_ImportFile
//================
//
void Map_ImportFile( const char *filename ){
	FileStream file;
	Sys_BeginWait();

	Sys_Printf( "Importing map from %s\n",filename );

	const char* type = strrchr( filename,'.' );
	if ( type != NULL ) {
		type++;
	}
	/*!\todo Resolve "r" problem in scriptlib" */
	if ( file.Open( filename, "rb" ) ) {
		Map_Import( &file, type, true );
	}
	else{
		Sys_FPrintf( SYS_ERR, "ERROR: couldn't open %s for read\n", filename );
	}

	file.Close();

	Sys_UpdateWindows( W_ALL );
	modified = true;
	Sys_EndWait();
}

//
//===========
//Map_SaveSelected
//===========
//
// Saves selected world brushes and whole entities with partial/full selections
//
void Map_SaveSelected( const char* filename ){
	FileStream file;

	Sys_Printf( "Saving selection to %s\n",filename );

	const char* type = strrchr( filename,'.' );
	if ( type != NULL ) {
		type++;
	}
	if ( file.Open( filename, "w" ) ) {
		Map_Export( &file, type, false, true );
	}
	else{
		Sys_FPrintf( SYS_ERR, "ERROR: failed to open %s for write\n", filename );
	}

	file.Close();

}

//
//===========
//Map_SaveSelected
//===========
//
// Saves selected world brushes and whole entities with partial/full selections
//
void Map_SaveSelected( MemStream* pMemFile, MemStream* pPatchFile ){
	Map_Export( pMemFile, "xmap", false, true );

	/*
	   // write world entity first
	   Entity_WriteSelected(world_entity, pMemFile);

	   // then write all other ents
	   count = 1;
	   for (e=entities.next ; e != &entities ; e=next)
	   {
	       MemFile_fprintf(pMemFile, "// entity %i\n", count);
	       count++;
	       Entity_WriteSelected(e, pMemFile);
	       next = e->next;
	   }

	   //if (pPatchFile)
	   //  Patch_WriteFile(pPatchFile);
	 */
}


void MemFile_fprintf( MemStream* pMemFile, const char* pText, ... ){
	char Buffer[4096];
	va_list args;
	va_start( args,pText );
	vsprintf( Buffer, pText, args );
	pMemFile->Write( Buffer, strlen( Buffer ) );
}

/*!
   ==============
   Region_SpawnPoint
   push the region spawn point
   \todo FIXME TTimo this was in the #1 MAP module implementation (in the core)
   not sure it has any use anymore, should prolly drop it
   ==============
 */
void Region_SpawnPoint( FILE *f ){
	// write the info_player_start, we use the camera position
	fprintf( f, "{\n" );
	fprintf( f, "\"classname\" \"info_player_start\"\n" );
	fprintf( f, "\"origin\" \"%i %i %i\"\n",
			 (int)g_pParentWnd->GetCamWnd()->Camera()->origin[0],
			 (int)g_pParentWnd->GetCamWnd()->Camera()->origin[1],
			 (int)g_pParentWnd->GetCamWnd()->Camera()->origin[2] );
	fprintf( f, "\"angle\" \"%i\"\n", (int)g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] );
	fprintf( f, "}\n" );
}
