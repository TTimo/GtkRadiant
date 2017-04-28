// Requries parts of the q3 tools source to compile
// Date: Oct 5, 2001
// Written by: Brad Whitehead (whiteheb@gamerstv.net)

#include "StdAfx.h"
#include "dialogs/dialogs-gtk.h"
#include "DWinding.h"
#include "bsploader.h"

typedef struct {
	int portalclusters;
	int leafbytes;           //leafbytes = ((portalclusters+63)&~63)>>3;
} vis_header;

// added because int shift = 32; i = 0xFFFFFFFF >> shift;
// then i = 0xFFFFFFFF, when it should = 0
const unsigned long bitmasks[33] =
{
	0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

int bsp_leafnumfororigin( vec3_t origin ){
	dnode_t     *node;
	dplane_t    *plane;
	float d;

	// TODO: check if origin is in the map??

	node = dnodes;
	while ( true )
	{
		plane = &dplanes[node->planeNum];
		d = DotProduct( origin, plane->normal ) - plane->dist;
		if ( d >= 0 ) {
			if ( node->children[0] < 0 ) {
				return -( node->children[0] + 1 );
			}
			else{
				node = &dnodes[node->children[0]];
			}
		}
		else
		if ( node->children[1] < 0 ) {
			return -( node->children[1] + 1 );
		}
		else{
			node = &dnodes[node->children[1]];
		}
	}
	return 0;
}

int bsp_leafnumforcluster( int cluster ){
	dleaf_t *l;
	int i;

	for ( i = 0, l = dleafs; i < numleafs; i++, l++ )
		if ( l->cluster == cluster ) {
			return( i );
		}
	return( 0 );
}

// leaf1 = origin leaf
// leaf2 = leaf to test for
/*int bsp_InPVS(int cluster1, int cluster2)
   {
    vis_header		*vheader;
    byte			*visdata;

    vheader = (vis_header *) visBytes;
    visdata = visBytes + VIS_HEADER_SIZE;

    return( *( visdata + ( cluster1 * vheader->leafbytes ) + (cluster2 / 8) ) & ( 1 << ( cluster2 % 8 ) ) );
   }*/

void bsp_setbitvectorlength( byte *v, int length_bits, int length_vector ){
	int i;

	i = length_bits / 8;

	*( v + i ) = (byte) bitmasks[length_bits % 8];

	memset( ( v + i + 1 ), 0, length_vector - i - 1 );
}


void bsp_bitvectorsubtract( byte *first, byte *second, byte *out, int length ){

	int i;

	for ( i = 0; i < length; i++ )
		*( out + i ) = *( first + i ) & ~( *( second + i ) );
}

int bsp_countclusters( byte *bitvector, int length ){
	int i, j, c;

	c = 0;
	for ( i = 0; i < length; i++ )
		for ( j = 0; j < 8; j++ )
			if ( ( *( bitvector + i ) & ( 1 << j ) ) ) {
				c++;
			}
	return( c );
}

int bsp_countclusters_mask( byte *bitvector, byte *maskvector, int length ){
	int i, j, c;

	c = 0;
	for ( i = 0; i < length; i++ )
		for ( j = 0; j < 8; j++ )
			if ( ( *( bitvector + i ) & ( 1 << j ) ) && ( *( maskvector + i ) & ( 1 << j ) ) ) {
				c++;
			}
	return( c );
}

void AddCluster( list<DWinding*> *pointlist, dleaf_t *cl, qboolean* repeatlist, vec3_t clr ){
	DWinding*   w;

	int* leafsurf = &dleafsurfaces[cl->firstLeafSurface];
	for ( int k = 0; k < cl->numLeafSurfaces; k++, leafsurf++ )
	{
		if ( repeatlist[*leafsurf] ) {
			continue;
		}

		dsurface_t* surf = &drawSurfaces[*leafsurf];
		if ( surf->surfaceType != MST_PLANAR ) {
			continue;
		}

		qdrawVert_t* vert = &drawVerts[surf->firstVert];
		if ( surf->firstVert + surf->numVerts > numDrawVerts ) {
			DoMessageBox( "Warning", "Warning", MB_OK );
		}

		w = new DWinding();
		w->AllocWinding( surf->numVerts );

		for ( int l = 0; l < surf->numVerts; l++, vert++ )
		{
			( w->p[l] )[0] = vert->xyz[0];
			( w->p[l] )[1] = vert->xyz[1];
			( w->p[l] )[2] = vert->xyz[2];

			w->clr[0] = clr[0];
			w->clr[1] = clr[1];
			w->clr[2] = clr[2];
		}
		pointlist->push_back( w );

		repeatlist[*leafsurf] = true;
	}
}

/*
   =============
   CreateTrace
   =============
 */
list<DWinding*> *CreateTrace( dleaf_t *leaf, int c, vis_header *header, byte *visdata, byte *seen ){
	byte        *vis;
	int i, j, clusterNum;
	list<DWinding*> *pointlist = new list<DWinding*>;
	qboolean*   repeatlist = new qboolean[numDrawSurfaces];
	dleaf_t     *cl;

	vec3_t clrRnd[5] =  {
		{0.f, 0.f, 1.f},
		{0.f, 1.f, 1.f},
		{1.f, 0.f, 0.f},
		{1.f, 0.f, 1.f},
		{1.f, 1.f, 0.f},
	};

	vec3_t clrGreen =   {0.f, 1.f, 0.f};

	memset( repeatlist, 0, sizeof( qboolean ) * numDrawSurfaces );

	vis = visdata + ( c * header->leafbytes );

	clusterNum = 0;

	AddCluster( pointlist, &( dleafs[bsp_leafnumforcluster( c )] ), repeatlist, clrGreen );

	for ( i = 0; i < header->leafbytes; i++ )
	{
		for ( j = 0; j < 8; j++ )
		{
			cl = &( dleafs[bsp_leafnumforcluster( clusterNum )] );

			if ( ( *( vis + i ) & ( 1 << j ) ) && ( *( seen + i ) & ( 1 << j ) ) && ( leaf->area == cl->area ) ) {
				AddCluster( pointlist, cl, repeatlist, clrRnd[rand() % 5] );
			}
			clusterNum++;
		}
	}

	delete[] repeatlist;

	return pointlist;
}

/*
   =============
   TraceCluster

   setup for CreateTrace
   =============
 */
list<DWinding*> *TraceCluster( int leafnum ){
	byte seen[( MAX_MAP_LEAFS / 8 ) + 1];
	vis_header      *vheader;
	byte            *visdata;
	dleaf_t         *leaf;

	vheader = (vis_header *) visBytes;
	visdata = visBytes + sizeof( vis_header );

	memset( seen, 0xFF, sizeof( seen ) );
	bsp_setbitvectorlength( seen, vheader->portalclusters, sizeof( seen ) );

	leaf = &( dleafs[leafnum] );

	return CreateTrace( leaf, leaf->cluster, vheader, visdata, seen );
}

list<DWinding *>* BuildTrace( char* filename, vec3_t v_origin ){
	if ( !LoadBSPFile( filename ) ) {
		return NULL;
	}

	int leafnum = bsp_leafnumfororigin( v_origin );

	list<DWinding*> *pointlist = TraceCluster( leafnum );

	FreeBSPData();

	return pointlist;
}
