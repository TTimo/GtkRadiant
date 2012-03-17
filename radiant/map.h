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

// map.h -- the state of the current world that all views are displaying

extern char currentmap[1024];

// head/tail of doubly linked lists
extern brush_t active_brushes;  // brushes currently being displayed
extern brush_t selected_brushes;    // highlighted

extern CPtrArray& g_ptrSelectedFaces;
extern CPtrArray& g_ptrSelectedFaceBrushes;

extern brush_t filtered_brushes;    // brushes that have been filtered or regioned

extern entity_t entities;
extern entity_t    *world_entity;   // the world entity is NOT included in
                                    // the entities chain

extern int modified;        // for quit confirmations

extern vec3_t region_mins, region_maxs;
extern qboolean region_active;

extern brush_t    *region_sides[6];

void Map_Init();

void    Map_LoadFile( const char *filename );
void    Map_SaveFile( const char *filename, qboolean use_region );

void    Map_New( void );
void  Map_Free( void );
void    Map_BuildBrushData( void );

void    Map_RegionOff( void );
void    Map_RegionXY( void );
void    Map_RegionTallBrush( void );
void    Map_RegionBrush( void );
void    Map_RegionSelectedBrushes( void );
qboolean Map_IsBrushFiltered( brush_t *b );

void Map_ImportFile( const char *filename );
void Map_SaveSelected( const char* filename );
//void Map_SaveSelected(MemStream* pMemFile, MemStream* pPatchFile = NULL);
//void Map_ImportBuffer (char* buf);

void  Map_StartPosition( void );
void Region_SpawnPoint( FILE *f );

void Map_Import( IDataStream *in, const char* type, bool bAddSelected = false );
void Map_Export( IDataStream *out, const char* type, bool bRegionOnly = false, bool bSelectedOnly = false );
