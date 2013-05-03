/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

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

#define SINGLE
#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gensurf.h"
#include "triangle.h"

typedef struct
{
	float error;
	int node;
} TRITABLE;

double dh, dv;
int NVP1;

#define Absolute( a )  ( ( a ) >= 0.0 ? ( a ) : -( a ) )

void MakeDecimatedMap( int *NumNodes, int *NumTris, NODE **pNode, TRI **pTri ){
	int compare( TRITABLE *, TRITABLE * );
	int Bisect( NODE *, int, int, int );
	void CalcAngles( NODE *, int *, float * );
	void EdgeOnSide( int *, int *, int * );
	int tricall( int, NODE *, int *, TRI * *, TRI * *, const char * );
	int CheckBorders( int *,int,NODE *,int *,TRI * * );

	float biggesterror;
	int i, j, N;
	int j0, j1, j2;
	int NumNodesToSave;
	int NumNodesUsed;
	NODE        *Node;
	TRI         *Tri;
	TRITABLE    *TriTable;

	if ( Decimate <= 0 ) {
		return;
	}
	/*
	   ghCursorCurrent = LoadCursor(NULL,IDC_WAIT);
	   SetCursor(ghCursorCurrent);
	 */
	dh = ( Hur - Hll ) / NH;
	dv = ( Vur - Vll ) / NV;
	NVP1 = NV + 1;

	NumNodes[0] = ( NH + 1 ) * ( NVP1 );
	*pNode = (NODE *) malloc( NumNodes[0] * sizeof( NODE ) );
	Node = *pNode;
	memset( Node,0,NumNodes[0] * sizeof( NODE ) );

	// Copy [NH][NV] vertex array to our working node array
	for ( i = 0,N = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++, N++ )
		{
			Node[N].p[0]  = (float)xyz[i][j].p[0];
			Node[N].p[1]  = (float)xyz[i][j].p[1];
			Node[N].p[2]  = (float)xyz[i][j].p[2];
			Node[N].fixed = xyz[i][j].fixed;
		}
	}
	// Start things off with the corner values
	Node[ 0].used           = 1;
	Node[NV].used           = 1;
	Node[NH * NVP1].used    = 1;
	Node[NH * NVP1 + NV].used = 1;
	NumNodesUsed = 4;
	tricall( NumNodes[0], Node, NumTris, NULL, pTri, "cnzBNPY" );
	Tri = *pTri;

	// Which coordinates are we triangulating on?
	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		j0 = 1;
		j1 = 0;
		j2 = 2;
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		j0 = 0;
		j1 = 1;
		j2 = 2;
		break;
	default:
		j0 = 2;
		j1 = 0;
		j2 = 1;
	}

	// TriTable stores the largest error in a triangle and the node where that
	// error occurs
	TriTable = (TRITABLE *) malloc( NH * NV * 2 * sizeof( TRITABLE ) );
	NumNodesToSave = min( NumNodes[0], (int)( 0.01 * ( 100 - Decimate ) * ( NumNodes[0] - NumNodesUsed ) + NumNodesUsed ) );

	while ( NumNodesUsed < NumNodesToSave )
	{
		for ( i = 0; i < NumTris[0]; i++ )
			Tri[i].flag = 0;

		// For every node that's not currently used, find what triangle it
		// lies on, and the error at this node
		for ( i = 0, biggesterror = 0; i < NumNodes[0]; i++ )
		{
			if ( Node[i].used ) {
				continue;
			}
			for ( j = 0, Node[i].tri = -1; ( j < NumTris[0] ) && ( Node[i].tri == -1 ); j++ )
			{
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[0]].p[j1],Node[Tri[j].v[0]].p[j2],
						   Node[Tri[j].v[1]].p[j1],Node[Tri[j].v[1]].p[j2] ) < 0. ) {
					continue;
				}
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[1]].p[j1],Node[Tri[j].v[1]].p[j2],
						   Node[Tri[j].v[2]].p[j1],Node[Tri[j].v[2]].p[j2] ) < 0. ) {
					continue;
				}
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[2]].p[j1],Node[Tri[j].v[2]].p[j2],
						   Node[Tri[j].v[0]].p[j1],Node[Tri[j].v[0]].p[j2] ) < 0. ) {
					continue;
				}
				Node[i].tri = j;
			}
			if ( Node[i].tri < 0 ) {
				/*
				   ghCursorCurrent = ghCursorDefault;
				   SetCursor(ghCursorCurrent);
				 */
				g_FuncTable.m_pfnMessageBox( g_pRadiantWnd,
											 "Error: Couldn't find the triangle bounding a point.",
											 "Decimation Error",MB_ICONEXCLAMATION, NULL );
				return;
			}
			if ( !Tri[Node[i].tri].flag ) {
				PlaneFromPoints( Node[Tri[Node[i].tri].v[0]].p,
								 Node[Tri[Node[i].tri].v[1]].p,
								 Node[Tri[Node[i].tri].v[2]].p,
								 &Tri[Node[i].tri].plane );
				Tri[Node[i].tri].flag = 1;
			}
			Node[i].error =
				Node[i].p[j0] - ( Tri[Node[i].tri].plane.dist -
								  Tri[Node[i].tri].plane.normal[j1] * Node[i].p[j1] -
								  Tri[Node[i].tri].plane.normal[j2] * Node[i].p[j2]  ) /
				Tri[Node[i].tri].plane.normal[j0];
			biggesterror = max( biggesterror,Absolute( Node[i].error ) );
		}
		if ( biggesterror == 0 ) {
			NumNodesToSave = NumNodesUsed;
		}
		else
		{
			// For all current triangles, build a list of worst-case nodes
			memset( TriTable,0,NH * NV * 2 * sizeof( TRITABLE ) );
			for ( i = 0; i < NumNodes[0]; i++ )
			{
				if ( Node[i].used ) {
					continue;
				}
				if ( Absolute( Node[i].error ) > TriTable[Node[i].tri].error ) {
					TriTable[Node[i].tri].error = (float)( Absolute( Node[i].error ) );
					TriTable[Node[i].tri].node  = i;
				}
			}
			qsort( (void *)TriTable, (size_t)( NumTris[0] ), sizeof( TRITABLE ), ( int ( * )( const void *, const void * ) )compare );
			for ( i = 0; i < NumTris[0] && NumNodesUsed < NumNodesToSave && TriTable[i].error > 0.5 * biggesterror; i++ )
			{
				if ( Node[TriTable[i].node].used ) {
					continue;                              // shouldn't happen
				}
				NumNodesUsed++;
				Node[TriTable[i].node].used++;
			}
			free( Tri );
			tricall( NumNodes[0], Node, NumTris, NULL, pTri, "cnzBNPY" );
			Tri = *pTri;
			// Sliver-check along borders. Since borders are often linear, the errors
			// along borders will often be zero, so no new points will be added. This
			// tends to produce long, thin brushes. For all border triangles, check
			// that minimum angle isn't less than SLIVER_ANGLE. If it is, add another
			// vertex.
			while ( CheckBorders( &NumNodesUsed,NumNodes[0],Node,NumTris,pTri ) > 0 )
			{
			}
			Tri = *pTri;
		}
	}
	free( TriTable );
	// One last time (because we're pessimistic), check border triangles
//	CheckBorders(&NumNodesUsed,NumNodes[0],Node,NumTris,pTri);
//	Tri = *pTri;

	// Check that all fixed points are exact. If not, add them to the mix.
	// First check to see if we have any fixed points that aren't already used.
	for ( i = 0, N = 0; i < NumNodes[0] && !N; i++ )
	{
		if ( Node[i].used ) {
			continue;
		}
		if ( Node[i].fixed ) {
			N++;
		}
	}
	if ( N ) {
		// Zero out the flag member of all triangles, indicating that
		// the plane equation has not been found.
		for ( i = 0; i < NumTris[0]; i++ )
			Tri[i].flag = 0;

		for ( i = 0; i < NumNodes[0]; i++ )
		{
			if ( Node[i].used ) {
				continue;
			}
			if ( !Node[i].fixed ) {
				continue;
			}
			Node[i].tri = -1;
			for ( j = 0; j < NumTris[0] && Node[i].tri == -1; j++ )
			{
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[0]].p[j1],Node[Tri[j].v[0]].p[j2],
						   Node[Tri[j].v[1]].p[j1],Node[Tri[j].v[1]].p[j2] ) < 0. ) {
					continue;
				}
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[1]].p[j1],Node[Tri[j].v[1]].p[j2],
						   Node[Tri[j].v[2]].p[j1],Node[Tri[j].v[2]].p[j2] ) < 0. ) {
					continue;
				}
				if ( side( Node[i].p[j1],          Node[i].p[j2],
						   Node[Tri[j].v[2]].p[j1],Node[Tri[j].v[2]].p[j2],
						   Node[Tri[j].v[0]].p[j1],Node[Tri[j].v[0]].p[j2] ) < 0. ) {
					continue;
				}
				Node[i].tri = j;
			}
			if ( Node[i].tri < 0 ) {
				/*
				   ghCursorCurrent = ghCursorDefault;
				   SetCursor(ghCursorCurrent);
				 */
				g_FuncTable.m_pfnMessageBox( g_pRadiantWnd,
											 "Error: Couldn't find the triangle bounding a point.",
											 "Decimation Error",MB_ICONEXCLAMATION, NULL );
				return;
			}
			if ( !Tri[Node[i].tri].flag ) {
				PlaneFromPoints( Node[Tri[Node[i].tri].v[0]].p,
								 Node[Tri[Node[i].tri].v[1]].p,
								 Node[Tri[Node[i].tri].v[2]].p,
								 &Tri[Node[i].tri].plane );
				Tri[Node[i].tri].flag = 1;
			}
			Node[i].error =
				Node[i].p[j0] - ( Tri[Node[i].tri].plane.dist -
								  Tri[Node[i].tri].plane.normal[j1] * Node[i].p[j1] -
								  Tri[Node[i].tri].plane.normal[j2] * Node[i].p[j2]  ) /
				Tri[Node[i].tri].plane.normal[j0];
			if ( Absolute( Node[i].error ) > 0.5 ) {
				NumNodesUsed++;
				Node[i].used++;
				free( Tri );
				tricall( NumNodes[0], Node, NumTris, NULL, pTri, "cnzBNPY" );
				Tri = *pTri;
			}
		}
	}

	// Swap node orders for surfaces facing down, north or west so that
	// they are counterclockwise when facing the surface

	if ( ( Plane == PLANE_XY1 ) || ( Plane == PLANE_XZ0 ) || ( Plane == PLANE_YZ1 ) ) {
		for ( i = 0; i < NumTris[0]; i++ )
		{
			j = Tri[i].v[1];
			Tri[i].v[1] = Tri[i].v[2];
			Tri[i].v[2] = j;
		}
	}

	// Store bounding box coords
	for ( i = 0; i < NumTris[0]; i++ )
	{
		Tri[i].min[0] =                   Node[Tri[i].v[0]].p[0];
		Tri[i].min[0] = min( Tri[i].min[0],Node[Tri[i].v[1]].p[0] );
		Tri[i].min[0] = min( Tri[i].min[0],Node[Tri[i].v[2]].p[0] );
		Tri[i].min[1] =                   Node[Tri[i].v[0]].p[1];
		Tri[i].min[1] = min( Tri[i].min[1],Node[Tri[i].v[1]].p[1] );
		Tri[i].min[1] = min( Tri[i].min[1],Node[Tri[i].v[2]].p[1] );
		Tri[i].min[2] =                   Node[Tri[i].v[0]].p[2];
		Tri[i].min[2] = min( Tri[i].min[2],Node[Tri[i].v[1]].p[2] );
		Tri[i].min[2] = min( Tri[i].min[2],Node[Tri[i].v[2]].p[2] );
		Tri[i].max[0] =                   Node[Tri[i].v[0]].p[0];
		Tri[i].max[0] = max( Tri[i].max[0],Node[Tri[i].v[1]].p[0] );
		Tri[i].max[0] = max( Tri[i].max[0],Node[Tri[i].v[2]].p[0] );
		Tri[i].max[1] =                   Node[Tri[i].v[0]].p[1];
		Tri[i].max[1] = max( Tri[i].max[1],Node[Tri[i].v[1]].p[1] );
		Tri[i].max[1] = max( Tri[i].max[1],Node[Tri[i].v[2]].p[1] );
		Tri[i].max[2] =                   Node[Tri[i].v[0]].p[2];
		Tri[i].max[2] = max( Tri[i].max[2],Node[Tri[i].v[1]].p[2] );
		Tri[i].max[2] = max( Tri[i].max[2],Node[Tri[i].v[2]].p[2] );
	}
	/*
	   ghCursorCurrent = ghCursorDefault;
	   SetCursor(ghCursorCurrent);
	 */
}
/* end MakeDecimatedMap */

/*****************************************************************************/
/*                                                                           */
/*  tricall Takes an array of nodes, spits out an array of triangles         */
/*                                                                           */
/*****************************************************************************/
int tricall( int NumNodes, NODE *Node, int *NumTris, TRI **inTri, TRI **Tri, const char *Options ){
	struct triangulateio in, out;
	int i, N;
	int NumUsedNodes;
	int    *NodeTable;
	TRI    *ptri;

	/* Define input points. */

	for ( i = 0,NumUsedNodes = 0; i < NumNodes; i++ )
		if ( Node[i].used ) {
			NumUsedNodes++;
		}

	memset( &in, 0,sizeof( in ) );
	memset( &out,0,sizeof( out ) );

	NodeTable = (int *) malloc( NumUsedNodes * sizeof( int ) );

	in.numberofpoints = NumUsedNodes;
	in.numberofpointattributes = 0;
	in.pointlist = (REAL *) malloc( in.numberofpoints * 2 * sizeof( REAL ) );
	for ( i = 0,N = 0; i < NumNodes; i++ )
	{
		if ( Node[i].used ) {
			switch ( Plane )
			{
			case PLANE_XZ0:
			case PLANE_XZ1:
				in.pointlist[N * 2  ] = Node[i].p[0];
				in.pointlist[N * 2 + 1] = Node[i].p[2];
				break;
			case PLANE_YZ0:
			case PLANE_YZ1:
				in.pointlist[N * 2  ] = Node[i].p[1];
				in.pointlist[N * 2 + 1] = Node[i].p[2];
				break;
			default:
				in.pointlist[N * 2  ] = Node[i].p[0];
				in.pointlist[N * 2 + 1] = Node[i].p[1];
			}
			NodeTable[N] = i;
			N++;
		}
	}
	in.pointattributelist = (REAL *) NULL;
	in.pointmarkerlist    = (int *) NULL;

	if ( strstr( Options,"r" ) ) {
		int    *TriTable;
		TriTable = (int *) malloc( NumNodes * sizeof( int ) );
		for ( i = 0,N = 0; i < NumNodes; i++ )
		{
			if ( Node[i].used ) {
				TriTable[i] = N;
				N++;
			}
		}
		in.numberoftriangles          = NumTris[0];
		in.numberofcorners            = 3;
		in.numberoftriangleattributes = 0;
		in.trianglelist               = (int *) malloc( in.numberofcorners * in.numberoftriangles * sizeof( int ) );
		in.triangleattributelist      = (REAL *) NULL;
		in.trianglearealist           = (REAL *) NULL;
		ptri = *inTri;
		for ( i = 0; i < in.numberoftriangles; i++ )
		{
			in.trianglelist[i * in.numberofcorners  ] = TriTable[ptri[i].v[0]];
			in.trianglelist[i * in.numberofcorners + 1] = TriTable[ptri[i].v[1]];
			in.trianglelist[i * in.numberofcorners + 2] = TriTable[ptri[i].v[2]];
		}
		free( TriTable );
	}
	else
	{
		in.numberoftriangles          = 0;
		in.numberofcorners            = 3;
		in.numberoftriangleattributes = 0;
		in.trianglelist               = (int *) NULL;
		in.triangleattributelist      = (REAL *) NULL;
		in.trianglearealist           = (REAL *) NULL;
	}

	in.numberofsegments   = 0;
	in.segmentlist        = (int *) NULL;
	in.segmentmarkerlist  = (int *) NULL;

	in.numberofholes      = 0;
	in.holelist           = (REAL *) NULL;

	in.numberofregions    = 0;
	in.regionlist         = (REAL *) NULL;

	in.numberofedges      = 0;
	in.edgelist           = (int *) NULL;
	in.edgemarkerlist     = (int *) NULL;
	in.normlist           = (REAL *) NULL;

	/* Make necessary initializations */
	out.pointlist          = (REAL *) NULL;  /* Not needed if -N switch used. */
	out.pointattributelist = (REAL *) NULL;  /* Not needed if -N switch used or
	                                            number of point attributes is zero: */
	out.pointmarkerlist    = (int *) NULL;   /* Not needed if -N or -B switch used. */
	out.trianglelist       = (int *) NULL;   /* Not needed if -E switch used. */
	out.triangleattributelist = (REAL *) NULL;   /* Not needed if -E switch used or
	                                                number of triangle attributes is
	                                                zero: */
	out.trianglearealist   = (REAL *) NULL;
	out.neighborlist       = (int *) NULL;   /* Needed only if -n switch used. */
	out.segmentlist        = (int *) NULL;   /* Needed only if segments are output
	                                            (-p or -c) and -P not used: */
	out.segmentmarkerlist  = (int *) NULL;   /* Needed only if segments are output
	                                            (-p or -c) and -P and -B not used: */
	out.edgelist           = (int *) NULL;   /* Needed only if -e switch used. */
	out.edgemarkerlist     = (int *) NULL;   /* Needed if -e used and -B not used. */

	triangulate( (char *) Options, &in, &out, NULL );

	NumTris[0] = out.numberoftriangles;
	*Tri = (TRI *) malloc( NumTris[0] * sizeof( TRI ) );
	ptri = *Tri;

	for ( i = 0; i < NumTris[0]; i++ )
	{
		ptri[i].v[0] = NodeTable[out.trianglelist[i * out.numberofcorners  ]];
		ptri[i].v[1] = NodeTable[out.trianglelist[i * out.numberofcorners + 1]];
		ptri[i].v[2] = NodeTable[out.trianglelist[i * out.numberofcorners + 2]];
		ptri[i].n[0] = out.neighborlist[i * 3  ];
		ptri[i].n[1] = out.neighborlist[i * 3 + 1];
		ptri[i].n[2] = out.neighborlist[i * 3 + 2];
	}

	/* Free all allocated arrays, including those allocated by Triangle. */
	if ( in.pointlist ) {
		free( in.pointlist );
	}
	if ( in.pointattributelist ) {
		free( in.pointattributelist );
	}
	if ( in.pointmarkerlist ) {
		free( in.pointmarkerlist );
	}
	if ( in.trianglelist ) {
		free( in.trianglelist );
	}
	if ( in.triangleattributelist ) {
		free( in.triangleattributelist );
	}
	if ( in.trianglearealist ) {
		free( in.trianglearealist );
	}
	if ( in.neighborlist ) {
		free( in.neighborlist );
	}
	if ( in.segmentlist ) {
		free( in.segmentlist );
	}
	if ( in.segmentmarkerlist ) {
		free( in.segmentmarkerlist );
	}
	if ( in.holelist ) {
		free( in.holelist );
	}
	if ( in.regionlist ) {
		free( in.regionlist );
	}
	if ( in.edgelist ) {
		free( in.edgelist );
	}
	if ( in.edgemarkerlist ) {
		free( in.edgemarkerlist );
	}
	if ( in.normlist ) {
		free( in.normlist );
	}
	if ( out.pointlist ) {
		free( out.pointlist );
	}
	if ( out.pointattributelist ) {
		free( out.pointattributelist );
	}
	if ( out.pointmarkerlist ) {
		free( out.pointmarkerlist );
	}
	if ( out.trianglelist ) {
		free( out.trianglelist );
	}
	if ( out.triangleattributelist ) {
		free( out.triangleattributelist );
	}
	if ( out.trianglearealist ) {
		free( out.trianglearealist );
	}
	if ( out.neighborlist ) {
		free( out.neighborlist );
	}
	if ( out.segmentlist ) {
		free( out.segmentlist );
	}
	if ( out.segmentmarkerlist ) {
		free( out.segmentmarkerlist );
	}
	if ( out.holelist ) {
		free( out.holelist );
	}
	if ( out.regionlist ) {
		free( out.regionlist );
	}
	if ( out.edgelist ) {
		free( out.edgelist );
	}
	if ( out.edgemarkerlist ) {
		free( out.edgemarkerlist );
	}
	if ( out.normlist ) {
		free( out.normlist );
	}

	free( NodeTable );
	return 0;
}

void EdgeOnSide( int *v, int *edge, int *border ){
	int R;
	int k0, k1, N;
	float Ndv;

	border[0] = -1;

	if ( ( v[0] <= NV ) && ( v[1] <= NV ) ) {
		edge[0]   = 0;
		border[0] = 0;
	}
	if ( ( v[1] <= NV ) && ( v[2] <= NV ) ) {
		edge[0]   = 1;
		border[0] = 0;
	}
	if ( ( v[2] <= NV ) && ( v[0] <= NV ) ) {
		edge[0]   = 2;
		border[0] = 0;
	}

	R = NH * NVP1;

	if ( ( v[0] >= R ) && ( v[1] >= R ) ) {
		edge[0]   = 0;
		border[0] = 1;
	}
	if ( ( v[1] >= R ) && ( v[2] >= R ) ) {
		edge[0]   = 1;
		border[0] = 1;
	}
	if ( ( v[2] >= R ) && ( v[0] >= R ) ) {
		edge[0]   = 2;
		border[0] = 1;
	}

	if ( border[0] >= 0 ) {
		k0  = edge[0];
		k1  = ( k0 + 1 ) % 3;
		N   = Absolute( v[k0] - v[k1] );
		Ndv = (float)( N * dv );
	}
	if ( ( ( v[0] % NVP1 ) == 0 )  && ( ( v[1] % NVP1 ) == 0 ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[0] - v[1] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 0;
		border[0] = 2;
		return;
	}
	if ( ( ( v[1] % NVP1 ) == 0 )  && ( ( v[2] % NVP1 ) == 0 ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[1] - v[2] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 1;
		border[0] = 2;
		return;
	}
	if ( ( ( v[2] % NVP1 ) == 0 )  && ( ( v[0] % NVP1 ) == 0 ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[2] - v[0] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 2;
		border[0] = 2;
		return;
	}

	if ( ( ( v[0] % NVP1 ) == NV ) && ( ( v[1] % NVP1 ) == NV ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[0] - v[1] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 0;
		border[0] = 3;
		return;
	}
	if ( ( ( v[1] % NVP1 ) == NV ) && ( ( v[2] % NVP1 ) == NV ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[1] - v[2] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 1;
		border[0] = 3;
		return;
	}
	if ( ( ( v[2] % NVP1 ) == NV ) && ( ( v[0] % NVP1 ) == NV ) ) {
		if ( border[0] >= 0 ) {
			if ( Ndv > ( Absolute( v[2] - v[0] ) * dh ) ) {
				return;
			}
		}
		edge[0]   = 2;
		border[0] = 3;
		return;
	}
	return;
}

void CalcAngles( NODE *node, int *v, float *angle ){
	int i, j, k;
	vec l;
	vec x0, x1, x2, y0, y1, y2;
	vec2 vv[3];
	vec dot;

	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		i = 0;
		j = 2;
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		i = 1;
		j = 2;
		break;
	default:
		i = 0;
		j = 1;
	}
	x0 = node[v[0]].p[i];
	x1 = node[v[1]].p[i];
	x2 = node[v[2]].p[i];
	y0 = node[v[0]].p[j];
	y1 = node[v[1]].p[j];
	y2 = node[v[2]].p[j];

	vv[0][0] = x1 - x0;
	vv[0][1] = y1 - y0;
	vv[1][0] = x2 - x1;
	vv[1][1] = y2 - y1;
	vv[2][0] = x0 - x2;
	vv[2][1] = y0 - y2;

	for ( k = 0; k < 3; k++ )
	{
		l = (vec)( sqrt( vv[k][0] * vv[k][0] + vv[k][1] * vv[k][1] ) );
		if ( l > 0. ) {
			vv[k][0] /= l;
			vv[k][1] /= l;
		}
	}

	dot = -( vv[0][0] * vv[2][0] + vv[0][1] * vv[2][1] );
	angle[0] = (float)( acos( dot ) );
	dot = -( vv[1][0] * vv[0][0] + vv[1][1] * vv[0][1] );
	angle[1] = (float)( acos( dot ) );
	dot = -( vv[2][0] * vv[1][0] + vv[2][1] * vv[1][1] );
	angle[2] = (float)( acos( dot ) );
}
//=================================================================
int Bisect( NODE *node, int border, int j0, int j1 ){
	int k;

	switch ( border )
	{
	case 0:
		k = ( j0 + j1 ) / 2;
		break;
	case 1:
		k = ( j0 + j1 ) / 2;
		break;
	case 2:
		k = (int)( ( j0 + j1 ) / ( 2 * NVP1 ) ) * NVP1;
		break;
	case 3:
		k = (int)( ( j0 + j1 + 2 ) / ( 2 * NVP1 ) ) * NVP1 - 1;
		break;
	}
	return( ( ( k != j0 ) && ( k != j1 ) ) ? k : 0 );
}
//=================================================================
int compare( TRITABLE *t1, TRITABLE *t2 ){
	if ( t1->error > t2->error ) {
		return -1;
	}
	if ( t1->error < t2->error ) {
		return 1;
	}
	return 0;
}

void MakeBrushes( int NumTris, NODE *Node, TRI *Tri,bool surf,
				  int offset,char *texture0, char *texture1, char *texture2 ){
	extern double backface;
	BRUSH brush;
	int contents;
	int i, j;
	float Steep;
	vec3_t PlaneNormal,SurfNormal;
	bool CheckAngle;
	vec3_t t[2];

	// if texture2 is identical to texture0, there's no need to
	// check surface angle
	if ( !g_ascii_strcasecmp( texture0,texture2 ) || !strlen( texture2 ) ) {
		CheckAngle = FALSE;
	}
	else
	{
		CheckAngle = TRUE;
		Steep = (float)cos( (double)SlantAngle / 57.2957795 );
		switch ( Plane )
		{
		case PLANE_XY0: PlaneNormal[0] = 0.; PlaneNormal[1] = 0.; PlaneNormal[2] = 1.; break;
		case PLANE_XY1: PlaneNormal[0] = 0.; PlaneNormal[1] = 0.; PlaneNormal[2] = -1.; break;
		case PLANE_XZ0: PlaneNormal[0] = 0.; PlaneNormal[1] = 1.; PlaneNormal[2] = 1.; break;
		case PLANE_XZ1: PlaneNormal[0] = 0.; PlaneNormal[1] = -1.; PlaneNormal[2] = 1.; break;
		case PLANE_YZ0: PlaneNormal[0] = 1.; PlaneNormal[1] = 0.; PlaneNormal[2] = 1.; break;
		case PLANE_YZ1: PlaneNormal[0] = -1.; PlaneNormal[1] = 0.; PlaneNormal[2] = 1.; break;
		}
	}

	contents = 0;
	if ( surf ) {
		if ( UseDetail ) {
			contents += CONTENTS_DETAIL;
		}
		if ( UseLadder ) {
			contents += CONTENTS_LADDER;
		}
	}

	OpenFuncGroup();
	for ( i = 0; i < NumTris; i++ )
	{
		brush.Number   = i;
		brush.NumFaces = 5;
		// front
		brush.face[0].v[0][0] = Node[Tri[i].v[0]].p[0];
		brush.face[0].v[0][1] = Node[Tri[i].v[0]].p[1];
		brush.face[0].v[0][2] = Node[Tri[i].v[0]].p[2];

		brush.face[0].v[1][0] = Node[Tri[i].v[2]].p[0];
		brush.face[0].v[1][1] = Node[Tri[i].v[2]].p[1];
		brush.face[0].v[1][2] = Node[Tri[i].v[2]].p[2];

		brush.face[0].v[2][0] = Node[Tri[i].v[1]].p[0];
		brush.face[0].v[2][1] = Node[Tri[i].v[1]].p[1];
		brush.face[0].v[2][2] = Node[Tri[i].v[1]].p[2];

		if ( offset != 0 ) {
			switch ( Plane )
			{
			case PLANE_XY0:
				brush.face[0].v[0][2] += offset;
				brush.face[0].v[1][2] += offset;
				brush.face[0].v[1][2] += offset;
				break;
			case PLANE_XY1:
				brush.face[0].v[0][2] -= offset;
				brush.face[0].v[1][2] -= offset;
				brush.face[0].v[1][2] -= offset;
				break;
			case PLANE_XZ0:
				brush.face[0].v[0][1] += offset;
				brush.face[0].v[1][1] += offset;
				brush.face[0].v[1][1] += offset;
				break;
			case PLANE_XZ1:
				brush.face[0].v[0][1] -= offset;
				brush.face[0].v[1][1] -= offset;
				brush.face[0].v[1][1] -= offset;
				break;
			case PLANE_YZ0:
				brush.face[0].v[0][0] += offset;
				brush.face[0].v[1][0] += offset;
				brush.face[0].v[1][0] += offset;
				break;
			case PLANE_YZ1:
				brush.face[0].v[0][0] -= offset;
				brush.face[0].v[1][0] -= offset;
				brush.face[0].v[1][0] -= offset;
				break;
			}
		}
		switch ( Plane )
		{
		case PLANE_XZ0:
		case PLANE_XZ1:
			// back
			brush.face[1].v[0][0] = Node[Tri[i].v[0]].p[0];
			brush.face[1].v[0][1] = (float)backface;
			brush.face[1].v[0][2] = Node[Tri[i].v[0]].p[2];

			brush.face[1].v[1][0] = Node[Tri[i].v[1]].p[0];
			brush.face[1].v[1][1] = (float)backface;
			brush.face[1].v[1][2] = Node[Tri[i].v[1]].p[2];

			brush.face[1].v[2][0] = Node[Tri[i].v[2]].p[0];
			brush.face[1].v[2][1] = (float)backface;
			brush.face[1].v[2][2] = Node[Tri[i].v[2]].p[2];

			// 0-1 side
			brush.face[2].v[0][0] = Node[Tri[i].v[0]].p[0];
			brush.face[2].v[0][1] = Node[Tri[i].v[0]].p[1];
			brush.face[2].v[0][2] = Node[Tri[i].v[0]].p[2];

			brush.face[2].v[1][0] = Node[Tri[i].v[1]].p[0];
			brush.face[2].v[1][1] = Node[Tri[i].v[1]].p[1];
			brush.face[2].v[1][2] = Node[Tri[i].v[1]].p[2];

			brush.face[2].v[2][0] = Node[Tri[i].v[1]].p[0];
			brush.face[2].v[2][1] = (float)backface;
			brush.face[2].v[2][2] = Node[Tri[i].v[1]].p[2];

			// 1-2 side
			brush.face[3].v[0][0] = Node[Tri[i].v[1]].p[0];
			brush.face[3].v[0][1] = Node[Tri[i].v[1]].p[1];
			brush.face[3].v[0][2] = Node[Tri[i].v[1]].p[2];

			brush.face[3].v[1][0] = Node[Tri[i].v[2]].p[0];
			brush.face[3].v[1][1] = Node[Tri[i].v[2]].p[1];
			brush.face[3].v[1][2] = Node[Tri[i].v[2]].p[2];

			brush.face[3].v[2][0] = Node[Tri[i].v[2]].p[0];
			brush.face[3].v[2][1] = (float)backface;
			brush.face[3].v[2][2] = Node[Tri[i].v[2]].p[2];

			// 2-0 side
			brush.face[4].v[0][0] = Node[Tri[i].v[2]].p[0];
			brush.face[4].v[0][1] = Node[Tri[i].v[2]].p[1];
			brush.face[4].v[0][2] = Node[Tri[i].v[2]].p[2];

			brush.face[4].v[1][0] = Node[Tri[i].v[0]].p[0];
			brush.face[4].v[1][1] = Node[Tri[i].v[0]].p[1];
			brush.face[4].v[1][2] = Node[Tri[i].v[0]].p[2];

			brush.face[4].v[2][0] = Node[Tri[i].v[0]].p[0];
			brush.face[4].v[2][1] = (float)backface;
			brush.face[4].v[2][2] = Node[Tri[i].v[0]].p[2];
			break;
		case PLANE_YZ0:
		case PLANE_YZ1:
			// back
			brush.face[1].v[0][0] = (float)backface;
			brush.face[1].v[0][1] = Node[Tri[i].v[0]].p[1];
			brush.face[1].v[0][2] = Node[Tri[i].v[0]].p[2];

			brush.face[1].v[1][0] = (float)backface;
			brush.face[1].v[1][1] = Node[Tri[i].v[1]].p[1];
			brush.face[1].v[1][2] = Node[Tri[i].v[1]].p[2];

			brush.face[1].v[2][0] = (float)backface;
			brush.face[1].v[2][1] = Node[Tri[i].v[2]].p[1];
			brush.face[1].v[2][2] = Node[Tri[i].v[2]].p[2];

			// 0-1 side
			brush.face[2].v[0][0] = Node[Tri[i].v[0]].p[0];
			brush.face[2].v[0][1] = Node[Tri[i].v[0]].p[1];
			brush.face[2].v[0][2] = Node[Tri[i].v[0]].p[2];

			brush.face[2].v[1][0] = Node[Tri[i].v[1]].p[0];
			brush.face[2].v[1][1] = Node[Tri[i].v[1]].p[1];
			brush.face[2].v[1][2] = Node[Tri[i].v[1]].p[2];

			brush.face[2].v[2][0] = (float)backface;
			brush.face[2].v[2][1] = Node[Tri[i].v[1]].p[1];
			brush.face[2].v[2][2] = Node[Tri[i].v[1]].p[2];

			// 1-2 side
			brush.face[3].v[0][0] = Node[Tri[i].v[1]].p[0];
			brush.face[3].v[0][1] = Node[Tri[i].v[1]].p[1];
			brush.face[3].v[0][2] = Node[Tri[i].v[1]].p[2];

			brush.face[3].v[1][0] = Node[Tri[i].v[2]].p[0];
			brush.face[3].v[1][1] = Node[Tri[i].v[2]].p[1];
			brush.face[3].v[1][2] = Node[Tri[i].v[2]].p[2];

			brush.face[3].v[2][0] = (float)backface;
			brush.face[3].v[2][1] = Node[Tri[i].v[2]].p[1];
			brush.face[3].v[2][2] = Node[Tri[i].v[2]].p[2];

			// 2-0 side
			brush.face[4].v[0][0] = Node[Tri[i].v[2]].p[0];
			brush.face[4].v[0][1] = Node[Tri[i].v[2]].p[1];
			brush.face[4].v[0][2] = Node[Tri[i].v[2]].p[2];

			brush.face[4].v[1][0] = Node[Tri[i].v[0]].p[0];
			brush.face[4].v[1][1] = Node[Tri[i].v[0]].p[1];
			brush.face[4].v[1][2] = Node[Tri[i].v[0]].p[2];

			brush.face[4].v[2][0] = (float)backface;
			brush.face[4].v[2][1] = Node[Tri[i].v[0]].p[1];
			brush.face[4].v[2][2] = Node[Tri[i].v[0]].p[2];
			break;
		default:
			// back
			brush.face[1].v[0][0] = Node[Tri[i].v[0]].p[0];
			brush.face[1].v[0][1] = Node[Tri[i].v[0]].p[1];
			brush.face[1].v[0][2] = (float)backface;

			brush.face[1].v[1][0] = Node[Tri[i].v[1]].p[0];
			brush.face[1].v[1][1] = Node[Tri[i].v[1]].p[1];
			brush.face[1].v[1][2] = (float)backface;

			brush.face[1].v[2][0] = Node[Tri[i].v[2]].p[0];
			brush.face[1].v[2][1] = Node[Tri[i].v[2]].p[1];
			brush.face[1].v[2][2] = (float)backface;

			// 0-1 side
			brush.face[2].v[0][0] = Node[Tri[i].v[0]].p[0];
			brush.face[2].v[0][1] = Node[Tri[i].v[0]].p[1];
			brush.face[2].v[0][2] = Node[Tri[i].v[0]].p[2];

			brush.face[2].v[1][0] = Node[Tri[i].v[1]].p[0];
			brush.face[2].v[1][1] = Node[Tri[i].v[1]].p[1];
			brush.face[2].v[1][2] = Node[Tri[i].v[1]].p[2];

			brush.face[2].v[2][0] = Node[Tri[i].v[1]].p[0];
			brush.face[2].v[2][1] = Node[Tri[i].v[1]].p[1];
			brush.face[2].v[2][2] = (float)backface;

			// 1-2 side
			brush.face[3].v[0][0] = Node[Tri[i].v[1]].p[0];
			brush.face[3].v[0][1] = Node[Tri[i].v[1]].p[1];
			brush.face[3].v[0][2] = Node[Tri[i].v[1]].p[2];

			brush.face[3].v[1][0] = Node[Tri[i].v[2]].p[0];
			brush.face[3].v[1][1] = Node[Tri[i].v[2]].p[1];
			brush.face[3].v[1][2] = Node[Tri[i].v[2]].p[2];

			brush.face[3].v[2][0] = Node[Tri[i].v[2]].p[0];
			brush.face[3].v[2][1] = Node[Tri[i].v[2]].p[1];
			brush.face[3].v[2][2] = (float)backface;

			// 2-0 side
			brush.face[4].v[0][0] = Node[Tri[i].v[2]].p[0];
			brush.face[4].v[0][1] = Node[Tri[i].v[2]].p[1];
			brush.face[4].v[0][2] = Node[Tri[i].v[2]].p[2];

			brush.face[4].v[1][0] = Node[Tri[i].v[0]].p[0];
			brush.face[4].v[1][1] = Node[Tri[i].v[0]].p[1];
			brush.face[4].v[1][2] = Node[Tri[i].v[0]].p[2];

			brush.face[4].v[2][0] = Node[Tri[i].v[0]].p[0];
			brush.face[4].v[2][1] = Node[Tri[i].v[0]].p[1];
			brush.face[4].v[2][2] = (float)backface;
		}

		for ( j = 0; j < 5; j++ )
		{
			strcpy( brush.face[j].texture,
					( strlen( texture1 ) ? texture1 : texture0 ) );
			brush.face[j].Shift[0] = (float)TexOffset[0];
			brush.face[j].Shift[1] = (float)TexOffset[1];
			brush.face[j].Rotate   = 0.;
			brush.face[j].Scale[0] = (float)TexScale[0];
			brush.face[j].Scale[1] = (float)TexScale[1];
			brush.face[j].Contents = contents;
			if ( surf ) {
				brush.face[j].Surface = 0;
			}
			else{
				brush.face[j].Surface = SURF_HINT;
			}
			brush.face[j].Value    = 0;
		}

		if ( CheckAngle ) {
			XYZVectorSubtract( brush.face[0].v[2],brush.face[0].v[0],t[0] );
			XYZVectorSubtract( brush.face[0].v[1],brush.face[0].v[2],t[1] );
			CrossProduct( t[0],t[1],SurfNormal );
			VectorNormalize( SurfNormal,SurfNormal );
			if ( DotProduct( SurfNormal,PlaneNormal ) < Steep ) {
				strcpy( brush.face[0].texture,texture2 );
			}
			else{
				strcpy( brush.face[0].texture,texture0 );
			}
		}
		else{
			strcpy( brush.face[0].texture,texture0 );
		}

		if ( surf ) {
			brush.face[0].Value    = ArghRad2;
		}
		MakeBrush( &brush );
	}
	CloseFuncGroup();

} // end MakeBrushes
//=================================================================
void MapOut( int NumNodes,int NumTris, NODE *Node, TRI *Tri ){
	extern double backface;
	extern double xmin, xmax, ymin, ymax, zmin, zmax;
	BRUSH brush;
	char hint[32], skip[32];
	int i, j;
	int face;
	/*
	   ghCursorCurrent = LoadCursor(NULL,IDC_WAIT);
	   SetCursor(ghCursorCurrent);
	 */
	UseDetail = 1; // this is temporary
	MakeBrushes( NumTris,Node,Tri,TRUE,0,Texture[Game][0],Texture[Game][1],Texture[Game][2] );

	if ( AddHints || GimpHints ) {
		switch ( Game )
		{
		case SIN:
			strcpy( hint,"generic/misc/hint" );
			strcpy( skip,"generic/misc/skip" );
			break;
		case HALFLIFE:
			strcpy( hint,"HINT" );
			strcpy( skip,"HINT" );
			break;
		case HERETIC2:
			strcpy( hint,"general/hint" );
			strcpy( skip,"general/skip" );
			break;
		case KINGPIN:
			strcpy( hint,"common/0_hint" );
			strcpy( skip,"common/0_skip" );
			break;
		case QUAKE3:
			strcpy( hint,"common/hint" );
			strcpy( skip,"common/skip" );
			break;
		default:
			strcpy( hint,"e1u1/hint" );
			strcpy( skip,"e1u1/skip" );
		}
	}

	if ( GimpHints ) {
		MakeBrushes( NumTris,Node,Tri,FALSE,HINT_OFFSET,hint,hint,hint );
	}

	if ( AddHints == 1 ) {
		int j0, j1, j2, k, k0, k1;
		int q[4];
		int w,h,h0,h1,t,OK;
		float s[3];
		double front;
		int MaxHints;   // We don't want a whole slew of hint brushes, which we'd get
		                // with low decimation values and our current placement scheme.
		                // Limit number of hint brushes to number of undecimated grid
		                // squares.

		switch ( Plane )
		{
		case PLANE_XY1:
			front  = LessThan( zmin,32. );
			break;
		case PLANE_XZ0:
			front  = MoreThan( ymax,32. );
			break;
		case PLANE_XZ1:
			front  = LessThan( ymin,32. );
			break;
		case PLANE_YZ0:
			front  = MoreThan( xmax,32. );
			break;
		case PLANE_YZ1:
			front  = LessThan( xmin,32. );
			break;
		default:
			front  = MoreThan( zmax,32. );
		}

		for ( i = 0; i < NumTris; i++ )
			Tri[i].flag = 0;

		switch ( Plane )
		{
		case PLANE_XZ0:
		case PLANE_XZ1:
			j0 = 1;
			j1 = 0;
			j2 = 2;
			break;
		case PLANE_YZ0:
		case PLANE_YZ1:
			j0 = 0;
			j1 = 1;
			j2 = 2;
			break;
		default:
			j0 = 2;
			j1 = 0;
			j2 = 1;
		}

		brush.Number = 0;
		brush.NumFaces = 6;
		MaxHints = NH * NV - 1;
		for ( w = 1; w < min( 16,NH ) && brush.Number < MaxHints; w++ )
		{
			for ( h = max( 1,w / 2 ); h < min( 16,NV ) && brush.Number < MaxHints; h++ )
			{
				for ( i = 0; i <= NH - w && brush.Number < MaxHints; i++ )
				{
					for ( j = 0; j <= NV - h && brush.Number < MaxHints; j++ )
					{
						q[0] = i * NVP1 + j;
						q[2] = q[0] + w * NVP1 + h;
						switch ( Plane )
						{
						case PLANE_XY1:
						case PLANE_XZ0:
						case PLANE_YZ1:
							q[1] = q[0] + h;
							q[3] = q[2] - h;
							break;
						default:
							q[1] = q[2] - h;
							q[3] = q[0] + h;
						}
						for ( k = 0, OK = 1; k < NumTris && OK; k++ )
						{
							if ( Tri[k].min[j1] >= max( Node[q[0]].p[j1],Node[q[2]].p[j1] ) ) {
								continue;
							}
							if ( Tri[k].min[j2] >= max( Node[q[0]].p[j2],Node[q[2]].p[j2] ) ) {
								continue;
							}
							if ( Tri[k].max[j1] <= min( Node[q[0]].p[j1],Node[q[2]].p[j1] ) ) {
								continue;
							}
							if ( Tri[k].max[j2] <= min( Node[q[0]].p[j2],Node[q[2]].p[j2] ) ) {
								continue;
							}

							for ( h0 = 0; h0 < 4 && OK; h0++ )
							{
								h1 = ( h0 + 1 ) % 4;
								for ( t = 0; t < 3 && OK; t++ )
								{
									s[t] = side( Node[q[h0]].p[j1],Node[q[h0]].p[j2],
												 Node[q[h1]].p[j1],Node[q[h1]].p[j2],
												 Node[Tri[k].v[t]].p[j1],Node[Tri[k].v[t]].p[j2] );
								}
								if ( ( s[1] > 0 || s[2] > 0 ) && s[0] < 0 ) {
									OK = 0;
								}
								if ( ( s[2] > 0 || s[0] > 0 ) && s[1] < 0 ) {
									OK = 0;
								}
								if ( ( s[0] > 0 || s[1] > 0 ) && s[2] < 0 ) {
									OK = 0;
								}
							}
						}
						if ( !OK ) {
							continue;
						}
						switch ( Plane )
						{
						case PLANE_XZ0:
						case PLANE_XZ1:
							// front
							brush.face[0].v[0][0] = Node[q[2]].p[0];
							brush.face[0].v[0][1] = (float)front;
							brush.face[0].v[0][2] = Node[q[2]].p[2];

							brush.face[0].v[1][0] = Node[q[1]].p[0];
							brush.face[0].v[1][1] = (float)front;
							brush.face[0].v[1][2] = Node[q[1]].p[2];

							brush.face[0].v[2][0] = Node[q[0]].p[0];
							brush.face[0].v[2][1] = (float)front;
							brush.face[0].v[2][2] = Node[q[0]].p[2];

							// back
							brush.face[1].v[0][0] = Node[q[0]].p[0];
							brush.face[1].v[0][1] = (float)backface;
							brush.face[1].v[0][2] = Node[q[0]].p[2];

							brush.face[1].v[1][0] = Node[q[1]].p[0];
							brush.face[1].v[1][1] = (float)backface;
							brush.face[1].v[1][2] = Node[q[1]].p[2];

							brush.face[1].v[2][0] = Node[q[2]].p[0];
							brush.face[1].v[2][1] = (float)backface;
							brush.face[1].v[2][2] = Node[q[2]].p[2];

							for ( k0 = 0; k0 < brush.NumFaces - 2; k0++ )
							{
								k = k0 + 2;
								k1 = ( k0 + 1 ) % ( brush.NumFaces - 2 );

								brush.face[k].v[0][0] = Node[q[k0]].p[0];
								brush.face[k].v[0][1] = (float)front;
								brush.face[k].v[0][2] = Node[q[k0]].p[2];

								brush.face[k].v[1][0] = Node[q[k1]].p[0];
								brush.face[k].v[1][1] = (float)front;
								brush.face[k].v[1][2] = Node[q[k1]].p[2];

								brush.face[k].v[2][0] = Node[q[k1]].p[0];
								brush.face[k].v[2][1] = (float)backface;
								brush.face[k].v[2][2] = Node[q[k1]].p[2];
							}
							break;
						case PLANE_YZ0:
						case PLANE_YZ1:
							// front
							brush.face[0].v[0][0] = (float)front;
							brush.face[0].v[0][1] = Node[q[2]].p[1];
							brush.face[0].v[0][2] = Node[q[2]].p[2];

							brush.face[0].v[1][0] = (float)front;
							brush.face[0].v[1][1] = Node[q[1]].p[1];
							brush.face[0].v[1][2] = Node[q[1]].p[2];

							brush.face[0].v[2][0] = (float)front;
							brush.face[0].v[2][1] = Node[q[0]].p[1];
							brush.face[0].v[2][2] = Node[q[0]].p[2];

							// back
							brush.face[1].v[0][0] = (float)backface;
							brush.face[1].v[0][1] = Node[q[0]].p[1];
							brush.face[1].v[0][2] = Node[q[0]].p[2];

							brush.face[1].v[1][0] = (float)backface;
							brush.face[1].v[1][1] = Node[q[1]].p[1];
							brush.face[1].v[1][2] = Node[q[1]].p[2];

							brush.face[1].v[2][0] = (float)backface;
							brush.face[1].v[2][1] = Node[q[2]].p[1];
							brush.face[1].v[2][2] = Node[q[2]].p[2];

							for ( k0 = 0; k0 < brush.NumFaces - 2; k0++ )
							{
								k = k0 + 2;
								k1 = ( k0 + 1 ) % ( brush.NumFaces - 2 );

								brush.face[k].v[0][0] = (float)front;
								brush.face[k].v[0][1] = Node[q[k0]].p[1];
								brush.face[k].v[0][2] = Node[q[k0]].p[2];

								brush.face[k].v[1][0] = (float)front;
								brush.face[k].v[1][1] = Node[q[k1]].p[1];
								brush.face[k].v[1][2] = Node[q[k1]].p[2];

								brush.face[k].v[2][0] = (float)backface;
								brush.face[k].v[2][1] = Node[q[k1]].p[1];
								brush.face[k].v[2][2] = Node[q[k1]].p[2];
							}
							break;
						default:
							// front
							brush.face[0].v[0][0] = Node[q[2]].p[0];
							brush.face[0].v[0][1] = Node[q[2]].p[1];
							brush.face[0].v[0][2] = (float)front;

							brush.face[0].v[1][0] = Node[q[1]].p[0];
							brush.face[0].v[1][1] = Node[q[1]].p[1];
							brush.face[0].v[1][2] = (float)front;

							brush.face[0].v[2][0] = Node[q[0]].p[0];
							brush.face[0].v[2][1] = Node[q[0]].p[1];
							brush.face[0].v[2][2] = (float)front;

							// back
							brush.face[1].v[0][0] = Node[q[0]].p[0];
							brush.face[1].v[0][1] = Node[q[0]].p[1];
							brush.face[1].v[0][2] = (float)backface;

							brush.face[1].v[1][0] = Node[q[1]].p[0];
							brush.face[1].v[1][1] = Node[q[1]].p[1];
							brush.face[1].v[1][2] = (float)backface;

							brush.face[1].v[2][0] = Node[q[2]].p[0];
							brush.face[1].v[2][1] = Node[q[2]].p[1];
							brush.face[1].v[2][2] = (float)backface;

							for ( k0 = 0; k0 < brush.NumFaces - 2; k0++ )
							{
								k = k0 + 2;
								k1 = ( k0 + 1 ) % ( brush.NumFaces - 2 );

								brush.face[k].v[0][0] = Node[q[k0]].p[0];
								brush.face[k].v[0][1] = Node[q[k0]].p[1];
								brush.face[k].v[0][2] = (float)front;

								brush.face[k].v[1][0] = Node[q[k1]].p[0];
								brush.face[k].v[1][1] = Node[q[k1]].p[1];
								brush.face[k].v[1][2] = (float)front;

								brush.face[k].v[2][0] = Node[q[k1]].p[0];
								brush.face[k].v[2][1] = Node[q[k1]].p[1];
								brush.face[k].v[2][2] = (float)backface;
							}
							break;
						} // switch (Plane)
						for ( face = 0; face < 6; face++ )
						{
							strcpy( brush.face[face].texture,( face <= 1 ? skip : hint ) );
							brush.face[face].Shift[0] = 0;
							brush.face[face].Shift[1] = 0;
							brush.face[face].Rotate   = 0.;
							brush.face[face].Scale[0] = 1;
							brush.face[face].Scale[1] = 1;
							brush.face[face].Contents = CONTENTS_DETAIL;
							brush.face[face].Surface  = ( face <= 1 ? SURF_SKIP : SURF_HINT );
							brush.face[face].Value    = 0;
						}
						if ( !brush.Number ) {
							OpenFuncGroup();
						}
						MakeBrush( &brush );
						brush.Number++;
					} // for(j=
				}     // for(i=
			}         // for(h=
		}             // for(w=
		if ( brush.Number ) {
			CloseFuncGroup();
		}
	}
	/*
	   ghCursorCurrent = ghCursorDefault;
	   SetCursor(ghCursorCurrent);
	 */
}
//===========================================================================
int CheckBorders( int *NumNodesUsed, int NumNodes, NODE *Node, int *NumTris, TRI **pTri ){
	int border;
	int i, j, k0, k1, N;
	float angle[3];
	TRI *Tri;

	N = NumNodesUsed[0];
	Tri = *pTri;
	for ( i = 0; i < NumTris[0]; i++ )
	{
		EdgeOnSide( Tri[i].v,&k0,&border );
		if ( border < 0 ) {
			continue;
		}
		CalcAngles( Node, Tri[i].v, angle );
		k1 = ( k0 + 1 ) % 3;
		if ( ( angle[k0] < SLIVER_ANGLE ) || ( angle[k1] < SLIVER_ANGLE ) ) {
			j = Bisect( Node, border, Tri[i].v[k0], Tri[i].v[k1] );
			if ( j >= 0 ) {
				if ( !Node[j].used ) {  // Shouldn't be used, but...
					NumNodesUsed[0]++;
					Node[j].used++;
				}
			}
		}
	}
	if ( NumNodesUsed[0] > N ) {
		free( *pTri );
		tricall( NumNodes, Node, NumTris, NULL, pTri, "cnzBNPY" );
		Tri = *pTri;
	}
	return ( NumNodesUsed[0] - N );
}
