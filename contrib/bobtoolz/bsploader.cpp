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
#include "./dialogs/dialogs-gtk.h"
#include "bsploader.h"
#include "../../libs/cmdlib.h"

int numnodes;
int numplanes;
int numleafs;
int numleafsurfaces;
int numVisBytes;
int numDrawVerts;
int numDrawSurfaces;
int numbrushes;
int numbrushsides;
int numleafbrushes;

byte                *visBytes =           NULL;
dnode_t           *dnodes =             NULL;
dplane_t          *dplanes =              NULL;
dleaf_t           *dleafs =             NULL;
qdrawVert_t   *drawVerts =        NULL;
dsurface_t    *drawSurfaces =       NULL;
int                 *dleafsurfaces =    NULL;
dbrush_t          *dbrushes =             NULL;
dbrushside_t    *dbrushsides =      NULL;
int                 *dleafbrushes =     NULL;

#define BSP_IDENT   ( ( 'P' << 24 ) + ( 'S' << 16 ) + ( 'B' << 8 ) + 'I' )
#define Q3_BSP_VERSION          46
#define WOLF_BSP_VERSION            47

/*
   ================
   FileLength
   ================
 */
int FileLength( FILE *f ){
	int pos;
	int end;

	pos = ftell( f );
	fseek( f, 0, SEEK_END );
	end = ftell( f );
	fseek( f, pos, SEEK_SET );

	return end;
}

/*
   ==============
   LoadFile
   ==============
 */
qboolean    LoadFile( const char *filename, byte **bufferptr ){
	FILE    *f;
	int length;
	byte    *buffer;

	f = fopen( filename, "rb" );
	if ( !f ) {
		return false;
	}

	length = FileLength( f );
	buffer = new byte[length + 1];
	buffer[length] = 0;
	fread( buffer, 1, length, f );
	fclose( f );

	*bufferptr = buffer;
	return true;
}

/*int    LittleLong (int l)
   {
    return l;
   }

   float	LittleFloat (float l)
   {
    return l;
   }*/

/*
   =============
   SwapBlock

   If all values are 32 bits, this can be used to swap everything
   =============
 */
void SwapBlock( int *block, int sizeOfBlock ) {
	int i;

	sizeOfBlock >>= 2;
	for ( i = 0 ; i < sizeOfBlock ; i++ ) {
		block[i] = LittleLong( block[i] );
	}
}

/*
   =============
   SwapBSPFile

   Byte swaps all data in a bsp file.
   =============
 */
void SwapBSPFile( void ) {
	int i;

	// models
//	SwapBlock( (int *)dmodels, nummodels * sizeof( dmodels[0] ) );

	// shaders (don't swap the name)
//	for ( i = 0 ; i < numShaders ; i++ ) {
//		dshaders[i].contentFlags = LittleLong( dshaders[i].contentFlags );
//		dshaders[i].surfaceFlags = LittleLong( dshaders[i].surfaceFlags );
//	}

	// planes
	SwapBlock( (int *)dplanes, numplanes * sizeof( dplanes[0] ) );

	// nodes
	SwapBlock( (int *)dnodes, numnodes * sizeof( dnodes[0] ) );

	// leafs
	SwapBlock( (int *)dleafs, numleafs * sizeof( dleafs[0] ) );

	// leaffaces
	SwapBlock( (int *)dleafsurfaces, numleafsurfaces * sizeof( dleafsurfaces[0] ) );

	// leafbrushes
	SwapBlock( (int *)dleafbrushes, numleafbrushes * sizeof( dleafbrushes[0] ) );

	// brushes
	SwapBlock( (int *)dbrushes, numbrushes * sizeof( dbrushes[0] ) );

	// brushsides
	SwapBlock( (int *)dbrushsides, numbrushsides * sizeof( dbrushsides[0] ) );

	// vis
	( (int *)&visBytes )[0] = LittleLong( ( (int *)&visBytes )[0] );
	( (int *)&visBytes )[1] = LittleLong( ( (int *)&visBytes )[1] );

	// drawverts (don't swap colors )
	for ( i = 0 ; i < numDrawVerts ; i++ ) {
		drawVerts[i].lightmap[0] = LittleFloat( drawVerts[i].lightmap[0] );
		drawVerts[i].lightmap[1] = LittleFloat( drawVerts[i].lightmap[1] );
		drawVerts[i].st[0] = LittleFloat( drawVerts[i].st[0] );
		drawVerts[i].st[1] = LittleFloat( drawVerts[i].st[1] );
		drawVerts[i].xyz[0] = LittleFloat( drawVerts[i].xyz[0] );
		drawVerts[i].xyz[1] = LittleFloat( drawVerts[i].xyz[1] );
		drawVerts[i].xyz[2] = LittleFloat( drawVerts[i].xyz[2] );
		drawVerts[i].normal[0] = LittleFloat( drawVerts[i].normal[0] );
		drawVerts[i].normal[1] = LittleFloat( drawVerts[i].normal[1] );
		drawVerts[i].normal[2] = LittleFloat( drawVerts[i].normal[2] );
	}

	// drawindexes
//	SwapBlock( (int *)drawIndexes, numDrawIndexes * sizeof( drawIndexes[0] ) );

	// drawsurfs
	SwapBlock( (int *)drawSurfaces, numDrawSurfaces * sizeof( drawSurfaces[0] ) );

	// fogs
//	for ( i = 0 ; i < numFogs ; i++ ) {
//		dfogs[i].brushNum = LittleLong( dfogs[i].brushNum );
//		dfogs[i].visibleSide = LittleLong( dfogs[i].visibleSide );
//	}
}

/*
   =============
   CopyLump
   =============
 */
int CopyLump( dheader_t *header, int lump, void **dest, int size ) {
	int length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if ( length == 0 ) {
		return 0;
	}

	*dest = new byte[length];
	memcpy( *dest, (byte *)header + ofs, length );

	return length / size;
}

/*
   =============
   LoadBSPFile
   =============
 */
qboolean    LoadBSPFile( const char *filename ) {
	dheader_t   *header;

	// load the file header
	if ( !LoadFile( filename, (byte **)&header ) ) {
		return false;
	}

	// swap the header
	SwapBlock( (int *)header, sizeof( *header ) );

	if ( header->ident != BSP_IDENT ) {
		DoMessageBox( "Cant find a valid IBSP file", "Error", MB_OK );
		return false;
	}
	if ( ( header->version != Q3_BSP_VERSION ) &&
		 ( header->version != WOLF_BSP_VERSION ) ) {
		DoMessageBox( "File is incorrect version", "Error", MB_OK );
		return false;
	}

	numbrushsides =     CopyLump( header, LUMP_BRUSHES,         (void**)&dbrushsides,   sizeof( dbrushside_t ) );
	numbrushes =        CopyLump( header, LUMP_BRUSHES,         (void**)&dbrushes,      sizeof( dbrush_t ) );
	numplanes =         CopyLump( header, LUMP_PLANES,          (void**)&dplanes,       sizeof( dplane_t ) );
	numleafs =          CopyLump( header, LUMP_LEAFS,           (void**)&dleafs,        sizeof( dleaf_t ) );
	numnodes =          CopyLump( header, LUMP_NODES,           (void**)&dnodes,        sizeof( dnode_t ) );
	numDrawVerts =      CopyLump( header, LUMP_DRAWVERTS,       (void**)&drawVerts,     sizeof( qdrawVert_t ) );
	numDrawSurfaces =   CopyLump( header, LUMP_SURFACES,        (void**)&drawSurfaces,  sizeof( dsurface_t ) );
	numleafsurfaces =   CopyLump( header, LUMP_LEAFSURFACES,    (void**)&dleafsurfaces, sizeof( int ) );
	numVisBytes =       CopyLump( header, LUMP_VISIBILITY,      (void**)&visBytes,      1 );
	numleafbrushes =    CopyLump( header, LUMP_LEAFBRUSHES,     (void**)&dleafbrushes,  sizeof( int ) );

	delete header;      // everything has been copied out

	// swap everything
	SwapBSPFile();

	return true;
}

void FreeBSPData(){
	if ( visBytes ) {
		delete visBytes;
	}
	if ( dnodes ) {
		delete dnodes;
	}
	if ( dplanes ) {
		delete dplanes;
	}
	if ( dleafs ) {
		delete dleafs;
	}
	if ( drawVerts ) {
		delete drawVerts;
	}
	if ( drawSurfaces ) {
		delete drawSurfaces;
	}
	if ( dleafsurfaces ) {
		delete dleafsurfaces;
	}
	if ( dleafbrushes ) {
		delete dleafbrushes;
	}
	if ( dbrushes ) {
		delete dbrushes;
	}
	if ( dbrushsides ) {
		delete dbrushsides;
	}
}
