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

#include "qd_fmodel.h"
#include "animcomp.h"
#include "qd_skeletons.h"
#include "skeletons.h"
#include "qdata.h"
#include "flex.h"
#include "reference.h"

#include <assert.h>

/*
   ========================================================================

   .FM triangle flexible model file format

   ========================================================================
 */

//=================================================================

#define NUMVERTEXNORMALS    162

extern float avertexnormals[NUMVERTEXNORMALS][3];

#define MAX_GROUPS  128

typedef struct
{
	triangle_t triangle;
	int group;
} trigroup_t;

#define TRIVERT_DIST    .1

typedef struct
{
	int start_frame;
	int num_frames;
	int degrees;
	char *mat;
	char *ccomp;
	char *cbase;
	float *cscale;
	float *coffset;
	float trans[3];
	float scale[3];
	float bmin[3];
	float bmax[3];
} fmgroup_t;

//================================================================

// Initial
fmheader_t fmheader;

// Skin
extern char g_skins[MAX_FM_SKINS][64];

// ST Coord
extern fmstvert_t base_st[MAX_FM_VERTS];

// Triangles
extern fmtriangle_t triangles[MAX_FM_TRIANGLES];

// Frames
fmframe_t g_frames[MAX_FM_FRAMES];
//fmframe_t	*g_FMframes;

// GL Commands
extern int commands[16384];
extern int numcommands;


//
// varibles set by commands
//
extern float scale_up;                              // set by $scale
extern vec3_t adjust;                               // set by $origin
extern int g_fixedwidth, g_fixedheight;             // set by $skinsize
extern char modelname[64];                          // set by $modelname


extern char        *g_outputDir;


// Mesh Nodes
mesh_node_t *pmnodes = NULL;
fmmeshnode_t mesh_nodes[MAX_FM_MESH_NODES];

fmgroup_t groups[MAX_GROUPS];
int num_groups;
int frame_to_group[MAX_FM_FRAMES];

//
// variables set by command line arguments
//
qboolean g_no_opimizations = false;


//
// base frame info
//
static int triangle_st[MAX_FM_TRIANGLES][3][2];


// number of gl vertices
extern int numglverts;
// indicates if a triangle has already been used in a glcmd
extern int used[MAX_FM_TRIANGLES];
// indicates if a triangle has translucency in it or not
static qboolean translucent[MAX_FM_TRIANGLES];

// main output file handle
extern FILE         *headerouthandle;
// output sizes of buildst()
static int skin_width, skin_height;


// statistics
static int total_skin_pixels;
static int skin_pixels_used;

int ShareVertex( trigroup_t trione, trigroup_t tritwo );
float DistBetween( vec3_t point1, vec3_t point2 );
int GetNumTris( trigroup_t *tris, int group );
void GetOneGroup( trigroup_t *tris, int grp, triangle_t* triangles );
void ScaleTris( vec3_t min, vec3_t max, int Width, int Height, float* u, float* v, int verts );
void NewDrawLine( int x1, int y1, int x2, int y2, unsigned char* picture, int width, int height );

#ifndef _WIN32

void strupr( char *string ){
	int i;

	for ( i = 0 ; i < strlen( string ); i++ )
		toupper( string[i] );

	return;
}

#endif
//==============================================================

/*
   ===============
   ClearModel
   ===============
 */
static void ClearModel( void ){
	memset( &fmheader, 0, sizeof( fmheader ) );

	modelname[0] = 0;
	scale_up = 1.0;
	VectorCopy( vec3_origin, adjust );
	g_fixedwidth = g_fixedheight = 0;
	g_skipmodel = false;
	num_groups = 0;

	if ( pmnodes ) {
		free( pmnodes );
		pmnodes = NULL;
	}

	ClearSkeletalModel();
}


extern void H_printf( char *fmt, ... );


void WriteHeader( FILE *FH, char *Ident, int Version, int Size, void *Data ){
	header_t header;
	static long pos = -1;
	long CurrentPos;

	if ( Size == 0 ) { // Don't write out empty packets
		return;
	}

	if ( pos != -1 ) {
		CurrentPos = ftell( FH );
		Size = CurrentPos - pos + sizeof( header_t );
		fseek( FH, pos, SEEK_SET );
		pos = -2;
	}
	else if ( Size == -1 ) {
		pos = ftell( FH );
	}

	memset( &header,0,sizeof( header ) );
	strcpy( header.ident,Ident );
	header.version = Version;
	header.size = Size;

	SafeWrite( FH, &header, sizeof( header ) );

	if ( Data ) {
		SafeWrite( FH, Data, Size );
	}

	if ( pos == -2 ) {
		pos = -1;
		fseek( FH, 0, SEEK_END );
	}
}

/*
   ============
   WriteModelFile
   ============
 */
static void WriteModelFile( FILE *modelouthandle ){
	int i;
	int j, k;
	fmframe_t       *in;
	fmaliasframe_t  *out;
	byte buffer[MAX_FM_VERTS * 4 + 128];
	float v;
	int c_on, c_off;
	IntListNode_t   *current, *toFree;
	qboolean framesWritten = false;
	size_t temp,size = 0;

	// probably should do this dynamically one of these days
	struct
	{
		float scale[3];         // multiply byte verts by this
		float translate[3];         // then add this
	} outFrames[MAX_FM_FRAMES];

#define DATA_SIZE 0x60000       // 384K had better be enough, particularly for the reference points
	byte data[DATA_SIZE];
	byte data2[DATA_SIZE];

	fmheader.num_glcmds = numcommands;
	fmheader.framesize = (int)&( (fmaliasframe_t *)0 )->verts[fmheader.num_xyz];

	WriteHeader( modelouthandle, FM_HEADER_NAME, FM_HEADER_VER, sizeof( fmheader ), &fmheader );

	//
	// write out the skin names
	//

	WriteHeader( modelouthandle, FM_SKIN_NAME, FM_SKIN_VER, fmheader.num_skins * MAX_FM_SKINNAME, g_skins );

	//
	// write out the texture coordinates
	//
	c_on = c_off = 0;
	for ( i = 0 ; i < fmheader.num_st ; i++ )
	{
		base_st[i].s = LittleShort( base_st[i].s );
		base_st[i].t = LittleShort( base_st[i].t );
	}

	WriteHeader( modelouthandle, FM_ST_NAME, FM_ST_VER, fmheader.num_st * sizeof( base_st[0] ), base_st );

	//
	// write out the triangles
	//
	WriteHeader( modelouthandle, FM_TRI_NAME, FM_TRI_VER, fmheader.num_tris * sizeof( fmtriangle_t ), NULL );

	for ( i = 0 ; i < fmheader.num_tris ; i++ )
	{
		int j;
		fmtriangle_t tri;

		for ( j = 0 ; j < 3 ; j++ )
		{
			tri.index_xyz[j] = LittleShort( triangles[i].index_xyz[j] );
			tri.index_st[j] = LittleShort( triangles[i].index_st[j] );
		}

		SafeWrite( modelouthandle, &tri, sizeof( tri ) );
	}

	if ( !num_groups ) {
		//
		// write out the frames
		//
		WriteHeader( modelouthandle, FM_FRAME_NAME, FM_FRAME_VER, fmheader.num_frames * fmheader.framesize, NULL );
		//	WriteHeader(modelouthandle, FM_FRAME_NAME, FM_FRAME_VER, -1, NULL);

		for ( i = 0 ; i < fmheader.num_frames ; i++ )
		{
			in = &g_frames[i];
			out = (fmaliasframe_t *)buffer;

			strcpy( out->name, in->name );
			for ( j = 0 ; j < 3 ; j++ )
			{
				out->scale[j] = ( in->maxs[j] - in->mins[j] ) / 255;
				out->translate[j] = in->mins[j];

				outFrames[i].scale[j] = out->scale[j];
				outFrames[i].translate[j] = out->translate[j];
			}

			for ( j = 0 ; j < fmheader.num_xyz ; j++ )
			{
				// all of these are byte values, so no need to deal with endianness
				out->verts[j].lightnormalindex = in->v[j].lightnormalindex;

				for ( k = 0 ; k < 3 ; k++ )
				{
					// scale to byte values & min/max check
					v = Q_rint( ( in->v[j].v[k] - out->translate[k] ) / out->scale[k] );

					// clamp, so rounding doesn't wrap from 255.6 to 0
					if ( v > 255.0 ) {
						v = 255.0;
					}
					if ( v < 0 ) {
						v = 0;
					}
					out->verts[j].v[k] = v;
				}
			}

			for ( j = 0 ; j < 3 ; j++ )
			{
				out->scale[j] = LittleFloat( out->scale[j] );
				out->translate[j] = LittleFloat( out->translate[j] );
			}

			SafeWrite( modelouthandle, out, fmheader.framesize );
		}

		// Go back and finish the header
		//	WriteHeader(modelouthandle, FM_FRAME_NAME, FM_FRAME_VER, -1, NULL);
	}
	else
	{
		WriteHeader( modelouthandle, FM_SHORT_FRAME_NAME, FM_SHORT_FRAME_VER,FRAME_NAME_LEN * fmheader.num_frames, NULL );
		for ( i = 0 ; i < fmheader.num_frames ; i++ )
		{
			in = &g_frames[i];
			SafeWrite( modelouthandle,in->name,FRAME_NAME_LEN );
		}
		WriteHeader( modelouthandle, FM_NORMAL_NAME, FM_NORMAL_VER,fmheader.num_xyz, NULL );
		in = &g_frames[0];
		for ( j = 0 ; j < fmheader.num_xyz ; j++ )
			SafeWrite( modelouthandle,&in->v[j].lightnormalindex,1 );
	}

	//
	// write out glcmds
	//
	WriteHeader( modelouthandle, FM_GLCMDS_NAME, FM_GLCMDS_VER, numcommands * 4, commands );

	//
	// write out mesh nodes
	//
	for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
	{
		memcpy( mesh_nodes[i].tris, pmnodes[i].tris, sizeof( mesh_nodes[i].tris ) );
		memcpy( mesh_nodes[i].verts, pmnodes[i].verts, sizeof( mesh_nodes[i].verts ) );
		mesh_nodes[i].start_glcmds = LittleShort( (short)pmnodes[i].start_glcmds );
		mesh_nodes[i].num_glcmds = LittleShort( (short)pmnodes[i].num_glcmds );
	}

	WriteHeader( modelouthandle, FM_MESH_NAME, FM_MESH_VER, sizeof( fmmeshnode_t ) * fmheader.num_mesh_nodes, mesh_nodes );

	if ( num_groups ) {

/*
   typedef struct
   {
    int			start_frame;
    int			num_frames;
    int			degrees;
    char *mat; fmheader.num_xyz*3*g->degrees*sizeof(char)
    char *ccomp; g->num_frames*g->degrees*sizeof(char)
    char *cbase; fmheader.num_xyz*3*sizeof(unsigned char)
    float *cscale; g->degrees*sizeof(float)
    float *coffset; g->degrees*sizeof(float)
    float trans[3]; 3*sizeof(float)
    float scale[3]; 3*sizeof(float)
   } fmgroup_t;
 */
		int tmp,k;
		fmgroup_t *g;
		size = sizeof( int ) + fmheader.num_frames * sizeof( int );
		for ( k = 0; k < num_groups; k++ )
		{
			g = &groups[k];
			size += sizeof( int ) * 3;
			size += fmheader.num_xyz * 3 * g->degrees * sizeof( char );
			size += g->num_frames * g->degrees * sizeof( char );
			size += fmheader.num_xyz * 3 * sizeof( unsigned char );
			size += g->degrees * sizeof( float );
			size += g->degrees * sizeof( float );
			size += 12 * sizeof( float );
		}
		WriteHeader( modelouthandle, FM_COMP_NAME, FM_COMP_VER,size, NULL );
		SafeWrite( modelouthandle,&num_groups,sizeof( int ) );
		SafeWrite( modelouthandle,frame_to_group,sizeof( int ) * fmheader.num_frames );

		for ( k = 0; k < num_groups; k++ )
		{
			g = &groups[k];
			tmp = LittleLong( g->start_frame );
			SafeWrite( modelouthandle,&tmp,sizeof( int ) );
			tmp = LittleLong( g->num_frames );
			SafeWrite( modelouthandle,&tmp,sizeof( int ) );
			tmp = LittleLong( g->degrees );
			SafeWrite( modelouthandle,&tmp,sizeof( int ) );

			SafeWrite( modelouthandle,g->mat,fmheader.num_xyz * 3 * g->degrees * sizeof( char ) );
			SafeWrite( modelouthandle,g->ccomp,g->num_frames * g->degrees * sizeof( char ) );
			SafeWrite( modelouthandle,g->cbase,fmheader.num_xyz * 3 * sizeof( unsigned char ) );
			SafeWrite( modelouthandle,g->cscale,g->degrees * sizeof( float ) );
			SafeWrite( modelouthandle,g->coffset,g->degrees * sizeof( float ) );
			SafeWrite( modelouthandle,g->trans,3 * sizeof( float ) );
			SafeWrite( modelouthandle,g->scale,3 * sizeof( float ) );
			SafeWrite( modelouthandle,g->bmin,3 * sizeof( float ) );
			SafeWrite( modelouthandle,g->bmax,3 * sizeof( float ) );
			free( g->mat );
			free( g->ccomp );
			free( g->cbase );
			free( g->cscale );
			free( g->coffset );
		}
	}

	// write the skeletal info
	if ( g_skelModel.type != SKEL_NULL ) {
		size = 0;

		temp = sizeof( int );     // change this to a byte
		memcpy( data + size, &g_skelModel.type, temp );
		size += temp;

		// number of joints
		temp = sizeof( int );     // change this to a byte
		memcpy( data + size, &numJointsInSkeleton[g_skelModel.type], temp );
		size += temp;

		// number of verts in each joint cluster
		temp = sizeof( int ) * numJointsInSkeleton[g_skelModel.type]; // change this to shorts
		memcpy( data + size, &g_skelModel.new_num_verts[1], temp );
		size += temp;

		// cluster verts
		for ( i = 0; i < numJointsInSkeleton[g_skelModel.type]; ++i )
		{
			current = g_skelModel.vertLists[i];
			while ( current )
			{
				temp = sizeof( int ); // change this to a short
				memcpy( data + size, &current->data, temp );
				size += temp;
				toFree = current;
				current = current->next;
				free( toFree );  // freeing of memory allocated in ReplaceClusterIndex called in Cmd_Base
			}
		}

		if ( !num_groups ) { // joints are stored with regular verts for compressed models
			framesWritten = true;

			temp = sizeof( int ); // change this to a byte
			memcpy( data + size, &framesWritten, temp );
			size += temp;

			for ( i = 0; i < fmheader.num_frames; ++i )
			{
				in = &g_frames[i];

				for ( j = 0 ; j < numJointsInSkeleton[g_skelModel.type]; ++j )
				{
					for ( k = 0 ; k < 3 ; k++ )
					{
						// scale to byte values & min/max check
						v = Q_rint( ( in->joints[j].placement.origin[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data + size, &v, temp );
						size += temp;
					}

					for ( k = 0 ; k < 3 ; k++ )
					{
						v = Q_rint( ( in->joints[j].placement.direction[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data + size, &v, temp );
						size += temp;
					}

					for ( k = 0 ; k < 3 ; k++ )
					{
						v = Q_rint( ( in->joints[j].placement.up[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data + size, &v, temp );
						size += temp;
					}
				}
			}

		}
		else
		{
			temp = sizeof( int ); // change this to a byte
			memcpy( data + size, &framesWritten, temp );
			size += temp;
		}

		WriteHeader( modelouthandle, FM_SKELETON_NAME, FM_SKELETON_VER, size, data );
	}

	if ( g_skelModel.references != REF_NULL ) {
		int refnum;

		size = 0;
		if ( RefPointNum <= 0 ) { // Hard-coded labels
			refnum = numReferences[g_skelModel.references];
		}
		else
		{   // Labels indicated in QDT
			refnum = RefPointNum;
		}

		temp = sizeof( int ); // change this to a byte
		memcpy( data2 + size, &g_skelModel.references, temp );
		size += temp;

		if ( !num_groups ) {
			framesWritten = true;

			temp = sizeof( int ); // change this to a byte
			memcpy( data2 + size, &framesWritten, temp );
			size += temp;

			for ( i = 0; i < fmheader.num_frames; ++i )
			{
				in = &g_frames[i];

				for ( j = 0 ; j < refnum; ++j )
				{
					for ( k = 0 ; k < 3 ; k++ )
					{
						// scale to byte values & min/max check
						v = Q_rint( ( in->references[j].placement.origin[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data2 + size, &v, temp );
						size += temp;
					}

					for ( k = 0 ; k < 3 ; k++ )
					{
						v = Q_rint( ( in->references[j].placement.direction[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data2 + size, &v, temp );
						size += temp;
					}

					for ( k = 0 ; k < 3 ; k++ )
					{
						v = Q_rint( ( in->references[j].placement.up[k] - outFrames[i].translate[k] ) / outFrames[i].scale[k] );

						// write out origin as a float since they arn't clamped
						temp = sizeof( float );   // change this to a short
						assert( size + temp < DATA_SIZE );
						memcpy( data2 + size, &v, temp );
						size += temp;
					}
				}
			}
		}
		else    // FINISH ME: references need to be stored with regular verts for compressed models
		{
			framesWritten = false;

			temp = sizeof( int ); // change this to a byte
			memcpy( data2 + size, &framesWritten, temp );
			size += temp;
		}

		WriteHeader( modelouthandle, FM_REFERENCES_NAME, FM_REFERENCES_VER, size, data2 );
	}
}

static void CompressFrames(){
	fmgroup_t *g;
	int i,j,k;
	fmframe_t   *in;

	j = 0;
	for ( i = 0; i < fmheader.num_frames; i++ )
	{
		while ( i >= groups[j].start_frame + groups[j].num_frames && j < num_groups - 1 )
			j++;
		frame_to_group[i] = j;
	}

	for ( k = 0; k < num_groups; k++ )
	{
		g = &groups[k];

		printf( "\nCompressing Frames for group %i...\n", k );
		AnimCompressInit( g->num_frames,fmheader.num_xyz,g->degrees );
		for ( i = 0; i < g->num_frames; i++ )
		{
			in = &g_frames[i + g->start_frame];
			for ( j = 0; j < fmheader.num_xyz; j++ )
				AnimSetFrame( i,j,in->v[j].v[0],in->v[j].v[1],in->v[j].v[2] );
		}
		AnimCompressDoit();
		g->mat = (char *) SafeMalloc( fmheader.num_xyz * 3 * g->degrees * sizeof( char ), "CompressFrames" );
		g->ccomp = (char *) SafeMalloc( g->num_frames * g->degrees * sizeof( char ), "CompressFrames" );
		g->cbase = (char *) SafeMalloc( fmheader.num_xyz * 3 * sizeof( unsigned char ), "CompressFrames" );
		g->cscale = (float *) SafeMalloc( g->degrees * sizeof( float ), "CompressFrames" );
		g->coffset = (float *) SafeMalloc( g->degrees * sizeof( float ), "CompressFrames" );
		AnimCompressToBytes( g->trans,g->scale,g->mat,g->ccomp,g->cbase,g->cscale,g->coffset,g->bmin,g->bmax );
		AnimCompressEnd();
	}
}

static void OptimizeVertices( void ){
	qboolean vert_used[MAX_FM_VERTS];
	short vert_replacement[MAX_FM_VERTS];
	int i,j,k,l,pos,bit,set_pos,set_bit;
	fmframe_t   *in;
	qboolean Found;
	int num_unique;
	static IntListNode_t *newVertLists[NUM_CLUSTERS];
	static int newNum_verts[NUM_CLUSTERS];
	IntListNode_t *current, *next;

	printf( "Optimizing vertices..." );

	memset( vert_used, 0, sizeof( vert_used ) );

	if ( g_skelModel.clustered == true ) {
		memset( newNum_verts, 0, sizeof( newNum_verts ) );
		memset( newVertLists, 0, sizeof( newVertLists ) );
	}

	num_unique = 0;

	// search for common points among all the frames
	for ( i = 0 ; i < fmheader.num_frames ; i++ )
	{
		in = &g_frames[i];

		for ( j = 0; j < fmheader.num_xyz; j++ )
		{
			for ( k = 0,Found = false; k < j; k++ )
			{   // starting from the beginning always ensures vert_replacement points to the first point in the array
				if ( in->v[j].v[0] == in->v[k].v[0] &&
					 in->v[j].v[1] == in->v[k].v[1] &&
					 in->v[j].v[2] == in->v[k].v[2] ) {
					Found = true;
					vert_replacement[j] = k;
					break;
				}

			}

			if ( !Found ) {
				if ( !vert_used[j] ) {
					num_unique++;
				}
				vert_used[j] = true;
			}
		}
	}

	// recompute the light normals
	for ( i = 0 ; i < fmheader.num_frames ; i++ )
	{
		in = &g_frames[i];

		for ( j = 0; j < fmheader.num_xyz; j++ )
		{
			if ( !vert_used[j] ) {
				k = vert_replacement[j];

				VectorAdd( in->v[j].vnorm.normalsum, in->v[k].vnorm.normalsum, in->v[k].vnorm.normalsum );
				in->v[k].vnorm.numnormals += in->v[j].vnorm.numnormals++;
			}
		}

		for ( j = 0 ; j < fmheader.num_xyz ; j++ )
		{
			vec3_t v;
			float maxdot;
			int maxdotindex;
			int c;

			c = in->v[j].vnorm.numnormals;
			if ( !c ) {
				Error( "Vertex with no triangles attached" );
			}

			VectorScale( in->v[j].vnorm.normalsum, 1.0 / c, v );
			VectorNormalize( v, v );

			maxdot = -999999.0;
			maxdotindex = -1;

			for ( k = 0 ; k < NUMVERTEXNORMALS ; k++ )
			{
				float dot;

				dot = DotProduct( v, avertexnormals[k] );
				if ( dot > maxdot ) {
					maxdot = dot;
					maxdotindex = k;
				}
			}

			in->v[j].lightnormalindex = maxdotindex;
		}
	}

	// create substitution list
	num_unique = 0;
	for ( i = 0; i < fmheader.num_xyz; i++ )
	{
		if ( vert_used[i] ) {
			vert_replacement[i] = num_unique;
			num_unique++;
		}
		else
		{
			vert_replacement[i] = vert_replacement[vert_replacement[i]];
		}

		// vert_replacement[i] is the new index, i is the old index
		// need to add the new index to the cluster list if old index was in it
		if ( g_skelModel.clustered == true ) {
			for ( k = 0; k < numJointsInSkeleton[g_skelModel.type]; ++k )
			{
				for ( l = 0, current = g_skelModel.vertLists[k];
					  l < g_skelModel.new_num_verts[k + 1]; ++l, current = current->next )
				{
					if ( current->data == i ) {
						IntListNode_t *current2;
						int m;
						qboolean added = false;

						for ( m = 0, current2 = newVertLists[k]; m < newNum_verts[k + 1];
							  ++m, current2 = current2->next )
						{
							if ( current2->data == vert_replacement[i] ) {
								added = true;
								break;
							}
						}

						if ( !added ) {
							++newNum_verts[k + 1];

							next = newVertLists[k];

							newVertLists[k] = (IntListNode_t *) SafeMalloc( sizeof( IntListNode_t ), "OptimizeVertices" );
							// freed after model write out

							newVertLists[k]->data = vert_replacement[i];
							newVertLists[k]->next = next;
						}
						break;
					}
				}
			}
		}
	}

	// substitute
	for ( i = 0 ; i < fmheader.num_frames ; i++ )
	{
		in = &g_frames[i];

		for ( j = 0; j < fmheader.num_xyz; j++ )
		{
			in->v[vert_replacement[j]] = in->v[j];
		}

	}

	for ( i = 0; i < numJointsInSkeleton[g_skelModel.type]; ++i )
	{
		IntListNode_t *toFree;
		current = g_skelModel.vertLists[i];

		while ( current )
		{
			toFree = current;
			current = current->next;
			free( toFree );  // freeing of memory allocated in ReplaceClusterIndex called in Cmd_Base
		}

		g_skelModel.vertLists[i] = newVertLists[i];
		g_skelModel.new_num_verts[i + 1] = newNum_verts[i + 1];
	}

#ifndef NDEBUG
	for ( k = 0; k < numJointsInSkeleton[g_skelModel.type]; ++k )
	{
		for ( l = 0, current = g_skelModel.vertLists[k];
			  l < g_skelModel.new_num_verts[k + 1]; ++l, current = current->next )
		{
			IntListNode_t *current2;
			int m;

			for ( m = l + 1, current2 = current->next; m < newNum_verts[k + 1];
				  ++m, current2 = current2->next )
			{
				if ( current->data == current2->data ) {
					printf( "Warning duplicate vertex:  %d\n", current->data );
					break;
				}
			}
		}
	}
#endif

	for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
	{   // reset the vert bits
		memset( pmnodes[i].verts,0,sizeof( pmnodes[i].verts ) );
	}

	// repleace the master triangle list vertex indexes and update the vert bits for each mesh node
	for ( i = 0 ; i < fmheader.num_tris ; i++ )
	{
		pos = i >> 3;
		bit = 1 << ( i & 7 );

		for ( j = 0 ; j < 3 ; j++ )
		{
			set_bit = set_pos = triangles[i].index_xyz[j] = vert_replacement[triangles[i].index_xyz[j]];

			set_pos >>= 3;
			set_bit = 1 << ( set_bit & 7 );

			for ( k = 0; k < fmheader.num_mesh_nodes; k++ )
			{
				if ( !( pmnodes[k].tris[pos] & bit ) ) {
					continue;
				}
				pmnodes[k].verts[set_pos] |= set_bit;
			}
		}
	}

	for ( i = 0; i < numcommands; i++ )
	{
		j = commands[i];
		if ( !j ) {
			continue;
		}

		j = abs( j );
		for ( i++; j; j--,i += 3 )
		{
			commands[i + 2] = vert_replacement[commands[i + 2]];
		}
		i--;
	}

	printf( "Reduced by %d\n",fmheader.num_xyz - num_unique );

	fmheader.num_xyz = num_unique;
	if ( num_groups ) {
		// tack on the reference verts to the regular verts
		if ( g_skelModel.references != REF_NULL ) {
			fmframe_t   *in;
			int index;
			int refnum;

			if ( RefPointNum <= 0 ) { // Hard-coded labels
				refnum = numReferences[g_skelModel.references];
			}
			else
			{   // Labels indicated in QDT
				refnum = RefPointNum;
			}


			for ( i = 0; i < fmheader.num_frames; ++i )
			{
				in = &g_frames[i];
				index = fmheader.num_xyz;

				for ( j = 0 ; j < refnum; ++j )
				{
					VectorCopy( in->references[j].placement.origin, in->v[index].v );
					index++;

					VectorCopy( in->references[j].placement.direction, in->v[index].v );
					index++;

					VectorCopy( in->references[j].placement.up, in->v[index].v );
					index++;
				}
			}

			fmheader.num_xyz += refnum * 3;
		}

		// tack on the skeletal joint verts to the regular verts
		if ( g_skelModel.type != SKEL_NULL ) {
			fmframe_t   *in;
			int index;

			for ( i = 0; i < fmheader.num_frames; ++i )
			{
				in = &g_frames[i];
				index = fmheader.num_xyz;

				for ( j = 0 ; j < numJointsInSkeleton[g_skelModel.type]; ++j )
				{
					VectorCopy( in->joints[j].placement.origin, in->v[index].v );
					index++;

					VectorCopy( in->joints[j].placement.direction, in->v[index].v );
					index++;

					VectorCopy( in->joints[j].placement.up, in->v[index].v );
					index++;
				}
			}

			fmheader.num_xyz += numJointsInSkeleton[g_skelModel.type] * 3;
		}

		CompressFrames();
	}
}


/*
   ===============
   FinishModel
   ===============
 */
void FMFinishModel( void ){
	FILE        *modelouthandle;
	int i,j,length,tris,verts,bit,pos,total_tris,total_verts;
	char name[1024];
	int trans_count;

	if ( !fmheader.num_frames ) {
		return;
	}

//
// copy to release directory tree if doing a release build
//
	if ( g_release ) {
		if ( modelname[0] ) {
			sprintf( name, "%s", modelname );
		}
		else{
			sprintf( name, "%s/tris.fm", cdpartial );
		}
		ReleaseFile( name );

		for ( i = 0 ; i < fmheader.num_skins ; i++ )
		{
			ReleaseFile( g_skins[i] );
		}
		fmheader.num_frames = 0;
		return;
	}

	printf( "\n" );

	trans_count = 0;
	for ( i = 0; i < fmheader.num_tris; i++ )
		if ( translucent[i] ) {
			trans_count++;
		}

	if ( !g_no_opimizations ) {
		OptimizeVertices();
	}

//
// write the model output file
//
	if ( modelname[0] ) {
		sprintf( name, "%s%s", g_outputDir, modelname );
	}
	else{
		sprintf( name, "%s/tris.fm", g_outputDir );
	}
	printf( "saving to %s\n", name );
	CreatePath( name );
	modelouthandle = SafeOpenWrite( name );

	WriteModelFile( modelouthandle );

	printf( "%3dx%3d skin\n", fmheader.skinwidth, fmheader.skinheight );
	printf( "First frame boundaries:\n" );
	printf( "	minimum x: %3f\n", g_frames[0].mins[0] );
	printf( "	maximum x: %3f\n", g_frames[0].maxs[0] );
	printf( "	minimum y: %3f\n", g_frames[0].mins[1] );
	printf( "	maximum y: %3f\n", g_frames[0].maxs[1] );
	printf( "	minimum z: %3f\n", g_frames[0].mins[2] );
	printf( "	maximum z: %3f\n", g_frames[0].maxs[2] );
	printf( "%4d vertices\n", fmheader.num_xyz );
	printf( "%4d triangles, %4d of them translucent\n", fmheader.num_tris, trans_count );
	printf( "%4d frame\n", fmheader.num_frames );
	printf( "%4d glverts\n", numglverts );
	printf( "%4d glcmd\n", fmheader.num_glcmds );
	printf( "%4d skins\n", fmheader.num_skins );
	printf( "%4d mesh nodes\n", fmheader.num_mesh_nodes );
	printf( "wasted pixels: %d / %d (%5.2f Percent)\n",total_skin_pixels - skin_pixels_used,
			total_skin_pixels, (double)( total_skin_pixels - skin_pixels_used ) / (double)total_skin_pixels * 100.0 );

	printf( "file size: %d\n", (int)ftell( modelouthandle ) );
	printf( "---------------------\n" );

	if ( g_verbose ) {
		if ( fmheader.num_mesh_nodes ) {
			total_tris = total_verts = 0;
			printf( "Node Name                         Tris Verts\n" );
			printf( "--------------------------------- ---- -----\n" );
			for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
			{
				tris = 0;
				verts = 0;
				for ( j = 0; j < MAXTRIANGLES; j++ )
				{
					pos = ( j ) >> 3;
					bit = 1 << ( ( j ) & 7 );
					if ( pmnodes[i].tris[pos] & bit ) {
						tris++;
					}
				}
				for ( j = 0; j < MAX_FM_VERTS; j++ )
				{
					pos = ( j ) >> 3;
					bit = 1 << ( ( j ) & 7 );
					if ( pmnodes[i].verts[pos] & bit ) {
						verts++;
					}
				}

				printf( "%-33s %4d %5d\n",pmnodes[i].name,tris,verts );

				total_tris += tris;
				total_verts += verts;
			}
			printf( "--------------------------------- ---- -----\n" );
			printf( "%-33s %4d %5d\n","TOTALS",total_tris,total_verts );
		}
	}
	fclose( modelouthandle );

	// finish writing header file
	H_printf( "\n" );

	// scale_up is usefull to allow step distances to be adjusted
	H_printf( "#define MODEL_SCALE\t\t%f\n", scale_up );

	// mesh nodes
	if ( fmheader.num_mesh_nodes ) {
		H_printf( "\n" );
		H_printf( "#define NUM_MESH_NODES\t\t%d\n\n",fmheader.num_mesh_nodes );
		for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
		{
			strcpy( name, pmnodes[i].name );
			strupr( name );
			length = strlen( name );
			for ( j = 0; j < length; j++ )
			{
				if ( name[j] == ' ' ) {
					name[j] = '_';
				}
			}
			H_printf( "#define MESH_%s\t\t%d\n", name, i );
		}
	}

	fclose( headerouthandle );
	headerouthandle = NULL;
	free( pmnodes );
}


/*
   =================================================================

   ALIAS MODEL DISPLAY LIST GENERATION

   =================================================================
 */

extern int strip_xyz[128];
extern int strip_st[128];
extern int strip_tris[128];
extern int stripcount;

/*
   ================
   StripLength
   ================
 */
static int  StripLength( int starttri, int startv, int num_tris, int node ){
	int m1, m2;
	int st1, st2;
	int j;
	fmtriangle_t    *last, *check;
	int k;
	int pos, bit;

	used[starttri] = 2;

	last = &triangles[starttri];

	strip_xyz[0] = last->index_xyz[( startv ) % 3];
	strip_xyz[1] = last->index_xyz[( startv + 1 ) % 3];
	strip_xyz[2] = last->index_xyz[( startv + 2 ) % 3];
	strip_st[0] = last->index_st[( startv ) % 3];
	strip_st[1] = last->index_st[( startv + 1 ) % 3];
	strip_st[2] = last->index_st[( startv + 2 ) % 3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->index_xyz[( startv + 2 ) % 3];
	st1 = last->index_st[( startv + 2 ) % 3];
	m2 = last->index_xyz[( startv + 1 ) % 3];
	st2 = last->index_st[( startv + 1 ) % 3];

	// look for a matching triangle
nexttri:
	for ( j = starttri + 1, check = &triangles[starttri + 1]
		  ; j < num_tris ; j++, check++ )
	{
		pos = j >> 3;
		bit = 1 << ( j & 7 );
		if ( !( pmnodes[node].tris[pos] & bit ) ) {
			continue;
		}
		for ( k = 0 ; k < 3 ; k++ )
		{
			if ( check->index_xyz[k] != m1 ) {
				continue;
			}
			if ( check->index_st[k] != st1 ) {
				continue;
			}
			if ( check->index_xyz[ ( k + 1 ) % 3 ] != m2 ) {
				continue;
			}
			if ( check->index_st[ ( k + 1 ) % 3 ] != st2 ) {
				continue;
			}

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if ( used[j] || translucent[j] != translucent[starttri] ) {
				goto done;
			}

			// the new edge
			if ( stripcount & 1 ) {
				m2 = check->index_xyz[ ( k + 2 ) % 3 ];
				st2 = check->index_st[ ( k + 2 ) % 3 ];
			}
			else
			{
				m1 = check->index_xyz[ ( k + 2 ) % 3 ];
				st1 = check->index_st[ ( k + 2 ) % 3 ];
			}

			strip_xyz[stripcount + 2] = check->index_xyz[ ( k + 2 ) % 3 ];
			strip_st[stripcount + 2] = check->index_st[ ( k + 2 ) % 3 ];
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for ( j = starttri + 1 ; j < num_tris ; j++ )
		if ( used[j] == 2 ) {
			used[j] = 0;
		}

	return stripcount;
}


/*
   ===========
   FanLength
   ===========
 */
static int  FanLength( int starttri, int startv, int num_tris, int node ){
	int m1, m2;
	int st1, st2;
	int j;
	fmtriangle_t    *last, *check;
	int k;
	int pos, bit;

	used[starttri] = 2;

	last = &triangles[starttri];

	strip_xyz[0] = last->index_xyz[( startv ) % 3];
	strip_xyz[1] = last->index_xyz[( startv + 1 ) % 3];
	strip_xyz[2] = last->index_xyz[( startv + 2 ) % 3];
	strip_st[0] = last->index_st[( startv ) % 3];
	strip_st[1] = last->index_st[( startv + 1 ) % 3];
	strip_st[2] = last->index_st[( startv + 2 ) % 3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->index_xyz[( startv + 0 ) % 3];
	st1 = last->index_st[( startv + 0 ) % 3];
	m2 = last->index_xyz[( startv + 2 ) % 3];
	st2 = last->index_st[( startv + 2 ) % 3];


	// look for a matching triangle
nexttri:
	for ( j = starttri + 1, check = &triangles[starttri + 1]
		  ; j < num_tris ; j++, check++ )
	{
		pos = j >> 3;
		bit = 1 << ( j & 7 );
		if ( !( pmnodes[node].tris[pos] & bit ) ) {
			continue;
		}
		for ( k = 0 ; k < 3 ; k++ )
		{
			if ( check->index_xyz[k] != m1 ) {
				continue;
			}
			if ( check->index_st[k] != st1 ) {
				continue;
			}
			if ( check->index_xyz[ ( k + 1 ) % 3 ] != m2 ) {
				continue;
			}
			if ( check->index_st[ ( k + 1 ) % 3 ] != st2 ) {
				continue;
			}

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if ( used[j] || translucent[j] != translucent[starttri] ) {
				goto done;
			}

			// the new edge
			m2 = check->index_xyz[ ( k + 2 ) % 3 ];
			st2 = check->index_st[ ( k + 2 ) % 3 ];

			strip_xyz[stripcount + 2] = m2;
			strip_st[stripcount + 2] = st2;
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for ( j = starttri + 1 ; j < num_tris ; j++ )
		if ( used[j] == 2 ) {
			used[j] = 0;
		}

	return stripcount;
}



/*
   ================
   BuildGlCmds

   Generate a list of trifans or strips
   for the model, which holds for all frames
   ================
 */
static void BuildGlCmds( void ){
	int i, j, k, l;
	int startv;
	float s, t;
	int len, bestlen, besttype;
	int best_xyz[1024];
	int best_st[1024];
	int best_tris[1024];
	int type;
	int trans_check;
	int bit,pos;

	//
	// build tristrips
	//
	numcommands = 0;
	numglverts = 0;


	for ( l = 0; l < fmheader.num_mesh_nodes; l++ )
	{
		memset( used, 0, sizeof( used ) );

		pmnodes[l].start_glcmds = numcommands;

		for ( trans_check = 0; trans_check < 2; trans_check++ )
		{
			for ( i = 0 ; i < fmheader.num_tris ; i++ )
			{
				pos = i >> 3;
				bit = 1 << ( i & 7 );
				if ( !( pmnodes[l].tris[pos] & bit ) ) {
					continue;
				}

				// pick an unused triangle and start the trifan
				if ( used[i] || trans_check != translucent[i] ) {
					continue;
				}

				bestlen = 0;
				for ( type = 0 ; type < 2 ; type++ )
				//	type = 1;
				{
					for ( startv = 0 ; startv < 3 ; startv++ )
					{
						if ( type == 1 ) {
							len = StripLength( i, startv, fmheader.num_tris, l );
						}
						else{
							len = FanLength( i, startv, fmheader.num_tris, l );
						}
						if ( len > bestlen ) {
							besttype = type;
							bestlen = len;
							for ( j = 0 ; j < bestlen + 2 ; j++ )
							{
								best_st[j] = strip_st[j];
								best_xyz[j] = strip_xyz[j];
							}
							for ( j = 0 ; j < bestlen ; j++ )
								best_tris[j] = strip_tris[j];
						}
					}
				}

				// mark the tris on the best strip/fan as used
				for ( j = 0 ; j < bestlen ; j++ )
					used[best_tris[j]] = 1;

				if ( besttype == 1 ) {
					commands[numcommands++] = ( bestlen + 2 );
				}
				else{
					commands[numcommands++] = -( bestlen + 2 );
				}

				numglverts += bestlen + 2;

				for ( j = 0 ; j < bestlen + 2 ; j++ )
				{
					// emit a vertex into the reorder buffer
					k = best_st[j];

					// emit s/t coords into the commands stream
					s = base_st[k].s;
					t = base_st[k].t;

					s = ( s  ) / fmheader.skinwidth;
					t = ( t  ) / fmheader.skinheight;

					*(float *)&commands[numcommands++] = s;
					*(float *)&commands[numcommands++] = t;
					*(int *)&commands[numcommands++] = best_xyz[j];
				}
			}
		}
		commands[numcommands++] = 0;        // end of list marker
		pmnodes[l].num_glcmds = numcommands - pmnodes[l].start_glcmds;
	}
}


/*
   ===============================================================

   BASE FRAME SETUP

   ===============================================================
 */


#define LINE_NORMAL 1
#define LINE_FAT 2
#define LINE_DOTTED 3


#define ASCII_SPACE 32

int LineType = LINE_NORMAL;
extern unsigned char pic[SKINPAGE_HEIGHT * SKINPAGE_WIDTH], pic_palette[768];
unsigned char LineColor = 255;
int ScaleWidth, ScaleHeight;


static char *CharDefs[] =
{
	"-------------------------",
	"-------------------------", // !
	"-------------------------", // "
	"-------------------------", // #
	"-------------------------", // $
	"-------------------------", // %
	"-------------------------", // &
	"--*----*-----------------", // '
	"-*---*----*----*-----*---", // (
	"*-----*----*----*---*----", // )
	"-----*--*--**---**--*--*-", // *
	"-------------------------", // +
	"----------------**--**---", // ,
	"-------------------------", // -
	"----------------**---**--", // .
	"-------------------------", // /
	" *** *  *** * ***  * *** ", // 0
	"   *   **    *    *    * ",
	"****     * *** *    *****",
	"****     * ***     ***** ",
	"  **  * * *  * *****   * ",
	"**** *    ****     ***** ",
	" *** *    **** *   * *** ",
	"*****    *   *   *    *  ",
	" *** *   * *** *   * *** ",
	" *** *   * ****    * *** ", // 9
	"-**---**--------**---**--", // :
	"-------------------------", // ;
	"-------------------------", // <
	"-------------------------", // =
	"-------------------------", // >
	"-------------------------", // ?
	"-------------------------", // @
	"-***-*---*******---**---*", // A
	"****-*---*****-*---*****-",
	"-*****----*----*-----****",
	"****-*---**---**---*****-",
	"******----****-*----*****",
	"******----****-*----*----",
	"-*****----*--***---*-****",
	"*---**---*******---**---*",
	"-***---*----*----*---***-",
	"----*----*----**---*-***-",
	"-*--*-*-*--**---*-*--*--*",
	"-*----*----*----*----****",
	"*---***-***-*-**---**---*",
	"*---***--**-*-**--***---*",
	"-***-*---**---**---*-***-",
	"****-*---*****-*----*----",
	"-***-*---**---*-***----**",
	"****-*---*****-*-*--*--**",
	"-*****-----***-----*****-",
	"*****--*----*----*----*--",
	"*---**---**---**---******",
	"*---**---**---*-*-*---*--",
	"*---**---**-*-***-***---*",
	"*---*-*-*---*---*-*-*---*",
	"*---**---*-*-*---*----*--",
	"*****---*---*---*---*****" // Z
};

void DrawLine( int x1, int y1, int x2, int y2 ){
	int dx, dy;
	int adx, ady;
	int count;
	float xfrac, yfrac, xstep, ystep;
	unsigned sx, sy;
	float u, v;

	dx = x2 - x1;
	dy = y2 - y1;
	adx = abs( dx );
	ady = abs( dy );

	count = adx > ady ? adx : ady;
	count++;

	if ( count > 300 ) {
		printf( "Bad count\n" );
		return; // don't ever hang up on bad data
	}

	xfrac = x1;
	yfrac = y1;

	xstep = (float)dx / count;
	ystep = (float)dy / count;

	switch ( LineType )
	{
	case LINE_NORMAL:
		do
		{
			if ( xfrac < SKINPAGE_WIDTH && yfrac < SKINPAGE_HEIGHT ) {
				pic[(int)yfrac * SKINPAGE_WIDTH + (int)xfrac] = LineColor;
			}
			xfrac += xstep;
			yfrac += ystep;
			count--;
		} while ( count > 0 );
		break;
	case LINE_FAT:
		do
		{
			for ( u = -0.1 ; u <= 0.9 ; u += 0.999 )
			{
				for ( v = -0.1 ; v <= 0.9 ; v += 0.999 )
				{
					sx = xfrac + u;
					sy = yfrac + v;
					if ( sx < SKINPAGE_WIDTH && sy < SKINPAGE_HEIGHT ) {
						pic[sy * SKINPAGE_WIDTH + sx] = LineColor;
					}
				}
			}
			xfrac += xstep;
			yfrac += ystep;
			count--;
		} while ( count > 0 );
		break;
	case LINE_DOTTED:
		do
		{
			if ( count & 1 && xfrac < SKINPAGE_WIDTH &&
				 yfrac < SKINPAGE_HEIGHT ) {
				pic[(int)yfrac * SKINPAGE_WIDTH + (int)xfrac] = LineColor;
			}
			xfrac += xstep;
			yfrac += ystep;
			count--;
		} while ( count > 0 );
		break;
	default:
		Error( "Unknown <linetype> %d.\n", LineType );
	}
}

//==========================================================================
//
// DrawCharacter
//
//==========================================================================

static void DrawCharacter( int x, int y, int character ){
	int r, c;
	char *def;

	character = toupper( character );
	if ( character < ASCII_SPACE || character > 'Z' ) {
		character = ASCII_SPACE;
	}
	character -= ASCII_SPACE;
	for ( def = CharDefs[character], r = 0; r < 5; r++ )
	{
		for ( c = 0; c < 5; c++ )
		{
			pic[( y + r ) * SKINPAGE_WIDTH + x + c] = *def++ == '*' ? 255 : 0;
		}
	}
}

//==========================================================================
//
// DrawTextChar
//
//==========================================================================

void DrawTextChar( int x, int y, char *text ){
	int c;

	while ( ( c = *text++ ) != '\0' )
	{
		DrawCharacter( x, y, c );
		x += 6;
	}
}


extern void DrawScreen( float s_scale, float t_scale, float iwidth, float iheight );

//==========================================================================
// ExtractDigit

static int ExtractDigit( byte *pic, int x, int y ){
	int i;
	int r, c;
	char digString[32];
	char    *buffer;
	byte backColor;
	char    **DigitDefs;

	backColor = pic[( SKINPAGE_HEIGHT - 1 ) * SKINPAGE_WIDTH];
	DigitDefs = &CharDefs['0' - ASCII_SPACE];

	buffer = digString;
	for ( r = 0; r < 5; r++ )
	{
		for ( c = 0; c < 5; c++ )
		{
			*buffer++ = ( pic[( y + r ) * SKINPAGE_WIDTH + x + c] == backColor ) ? ' ' : '*';
		}
	}
	*buffer = '\0';
	for ( i = 0; i < 10; i++ )
	{
		if ( strcmp( DigitDefs[i], digString ) == 0 ) {
			return i;
		}
	}

	Error( "Unable to extract scaling info from skin PCX." );
	return 0;
}

//==========================================================================
// ExtractNumber

int ExtractNumber( byte *pic, int x, int y ){
	return ExtractDigit( pic, x, y ) * 100 + ExtractDigit( pic, x + 6, y ) * 10 + ExtractDigit( pic, x + 12, y );
}





/*
   ============
   BuildST

   Builds the triangle_st array for the base frame and
   fmheader.skinwidth / fmheader.skinheight

   FIXME: allow this to be loaded from a file for
   arbitrary mappings
   ============
 */
static void BuildST( triangle_t *ptri, int numtri, qboolean DrawSkin ){
	int backface_flag;
	int i, j;
	int width, height, iwidth, iheight, swidth;
	float basex, basey;
	float scale;
	vec3_t mins, maxs;
	float       *pbasevert;
	vec3_t vtemp1, vtemp2, normal;
	float s_scale, t_scale;
	float scWidth;
	float scHeight;
	int skinwidth;
	int skinheight;

	//
	// find bounds of all the verts on the base frame
	//
	ClearBounds( mins, maxs );
	backface_flag = false;

	if ( ptri[0].HasUV ) { // if we have the uv already, we don't want to double up or scale
		iwidth = ScaleWidth;
		iheight = ScaleHeight;

		t_scale = s_scale = 1.0;
	}
	else
	{
		for ( i = 0 ; i < numtri ; i++ )
			for ( j = 0 ; j < 3 ; j++ )
				AddPointToBounds( ptri[i].verts[j], mins, maxs );

		for ( i = 0 ; i < 3 ; i++ )
		{
			mins[i] = floor( mins[i] );
			maxs[i] = ceil( maxs[i] );
		}

		width = maxs[0] - mins[0];
		height = maxs[2] - mins[2];

		for ( i = 0 ; i < numtri ; i++ )
		{
			VectorSubtract( ptri[i].verts[0], ptri[i].verts[1], vtemp1 );
			VectorSubtract( ptri[i].verts[2], ptri[i].verts[1], vtemp2 );
			CrossProduct( vtemp1, vtemp2, normal );

			if ( normal[1] > 0 ) {
				backface_flag = true;
				break;
			}
		}
		scWidth = ScaleWidth * SCALE_ADJUST_FACTOR;
		if ( backface_flag ) {  //we are doubling
			scWidth /= 2;
		}

		scHeight = ScaleHeight * SCALE_ADJUST_FACTOR;

		scale = scWidth / width;

		if ( height * scale >= scHeight ) {
			scale = scHeight / height;
		}

		iwidth = ceil( width * scale ) + 4;
		iheight = ceil( height * scale ) + 4;

		s_scale = (float)( iwidth - 4 ) / width;
		t_scale = (float)( iheight - 4 ) / height;
		t_scale = s_scale;
	}
	if ( DrawSkin ) {
		if ( backface_flag ) {
			DrawScreen( s_scale, t_scale, iwidth * 2, iheight );
		}
		else{
			DrawScreen( s_scale, t_scale, iwidth, iheight );
		}
	}
	if ( backface_flag ) {
		skinwidth = iwidth * 2;
	}
	else{
		skinwidth = iwidth;
	}
	skinheight = iheight;


/*	if (!g_fixedwidth)
    {	// old style
        scale = 8;
        if (width*scale >= 150)
            scale = 150.0 / width;
        if (height*scale >= 190)
            scale = 190.0 / height;

        s_scale = t_scale = scale;

        iwidth = ceil(width*s_scale);
        iheight = ceil(height*t_scale);

        iwidth += 4;
        iheight += 4;
    }
    else
    {	// new style
        iwidth = g_fixedwidth / 2;
        iheight = g_fixedheight;

        s_scale = (float)(iwidth-4) / width;
        t_scale = (float)(iheight-4) / height;
    }*/

//
// determine which side of each triangle to map the texture to
//
	basey = 2;
	for ( i = 0 ; i < numtri ; i++ )
	{
		if ( ptri[i].HasUV ) {
			for ( j = 0 ; j < 3 ; j++ )
			{
				triangle_st[i][j][0] = Q_rint( ptri[i].uv[j][0] * skinwidth );
				triangle_st[i][j][1] = Q_rint( ( 1.0f - ptri[i].uv[j][1] ) * skinheight );
			}
		}
		else
		{
			VectorSubtract( ptri[i].verts[0], ptri[i].verts[1], vtemp1 );
			VectorSubtract( ptri[i].verts[2], ptri[i].verts[1], vtemp2 );
			CrossProduct( vtemp1, vtemp2, normal );

			if ( normal[1] > 0 ) {
				basex = iwidth + 2;
			}
			else
			{
				basex = 2;
			}

			for ( j = 0 ; j < 3 ; j++ )
			{
				pbasevert = ptri[i].verts[j];

				triangle_st[i][j][0] = Q_rint( ( pbasevert[0] - mins[0] ) * s_scale + basex );
				triangle_st[i][j][1] = Q_rint( ( maxs[2] - pbasevert[2] ) * t_scale + basey );
			}
		}

		if ( DrawSkin ) {
			DrawLine( triangle_st[i][0][0], triangle_st[i][0][1],
					  triangle_st[i][1][0], triangle_st[i][1][1] );
			DrawLine( triangle_st[i][1][0], triangle_st[i][1][1],
					  triangle_st[i][2][0], triangle_st[i][2][1] );
			DrawLine( triangle_st[i][2][0], triangle_st[i][2][1],
					  triangle_st[i][0][0], triangle_st[i][0][1] );
		}
	}

// make the width a multiple of 4; some hardware requires this, and it ensures
// dword alignment for each scan

	swidth = iwidth;
	if ( backface_flag ) {
		swidth *= 2;
	}
	fmheader.skinwidth = ( swidth + 3 ) & ~3;
	fmheader.skinheight = iheight;

	skin_width = iwidth;
	skin_height = iheight;
}


static void BuildNewST( triangle_t *ptri, int numtri, qboolean DrawSkin ){
	int i, j;

	for ( i = 0 ; i < numtri ; i++ )
	{
		if ( ptri[i].HasUV ) {
			for ( j = 0 ; j < 3 ; j++ )
			{
				triangle_st[i][j][0] = Q_rint( ptri[i].uv[j][0] * ( ScaleWidth - 1 ) );
				triangle_st[i][j][1] = Q_rint( ( 1.0f - ptri[i].uv[j][1] ) * ( ScaleHeight - 1 ) );
			}
		}

		if ( DrawSkin ) {
			DrawLine( triangle_st[i][0][0], triangle_st[i][0][1],
					  triangle_st[i][1][0], triangle_st[i][1][1] );
			DrawLine( triangle_st[i][1][0], triangle_st[i][1][1],
					  triangle_st[i][2][0], triangle_st[i][2][1] );
			DrawLine( triangle_st[i][2][0], triangle_st[i][2][1],
					  triangle_st[i][0][0], triangle_st[i][0][1] );
		}
	}

// make the width a multiple of 4; some hardware requires this, and it ensures
// dword alignment for each scan

	fmheader.skinwidth = ( ScaleWidth + 3 ) & ~3;
	fmheader.skinheight = ScaleHeight;

	skin_width = ScaleWidth;
	skin_height = ScaleHeight;
}




byte            *BasePalette;
byte            *BasePixels,*TransPixels;
int BaseWidth, BaseHeight, TransWidth, TransHeight;
qboolean BaseTrueColor;
static qboolean SetPixel = false;

int CheckTransRecursiveTri( int *lp1, int *lp2, int *lp3 ){
	int     *temp;
	int d;
	int new[2];

	d = lp2[0] - lp1[0];
	if ( d < -1 || d > 1 ) {
		goto split;
	}
	d = lp2[1] - lp1[1];
	if ( d < -1 || d > 1 ) {
		goto split;
	}

	d = lp3[0] - lp2[0];
	if ( d < -1 || d > 1 ) {
		goto split2;
	}
	d = lp3[1] - lp2[1];
	if ( d < -1 || d > 1 ) {
		goto split2;
	}

	d = lp1[0] - lp3[0];
	if ( d < -1 || d > 1 ) {
		goto split3;
	}
	d = lp1[1] - lp3[1];
	if ( d < -1 || d > 1 ) {
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return 0;           // entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new[0] = ( lp1[0] + lp2[0] ) >> 1;
	new[1] = ( lp1[1] + lp2[1] ) >> 1;

// draw the point if splitting a leading edge
	if ( lp2[1] > lp1[1] ) {
		goto nodraw;
	}
	if ( ( lp2[1] == lp1[1] ) && ( lp2[0] < lp1[0] ) ) {
		goto nodraw;
	}

	if ( SetPixel ) {
		assert( ( new[1] * BaseWidth ) + new[0] < BaseWidth * BaseHeight );

		if ( BaseTrueColor ) {
			BasePixels[( ( new[1] * BaseWidth ) + new[0] ) * 4] = 1;
		}
		else
		{
			BasePixels[( new[1] * BaseWidth ) + new[0]] = 1;
		}
	}
	else
	{
		if ( TransPixels ) {
			if ( TransPixels[( new[1] * TransWidth ) + new[0]] != 255 ) {
				return 1;
			}
		}
		else if ( BaseTrueColor ) {
			if ( BasePixels[( ( ( new[1] * BaseWidth ) + new[0] ) * 4 ) + 3] != 255 ) {
				return 1;
			}
		}
		else
		{
//			pixel = BasePixels[(new[1]*BaseWidth) + new[0]];
		}
	}

nodraw:
// recursively continue
	if ( CheckTransRecursiveTri( lp3, lp1, new ) ) {
		return 1;
	}

	return CheckTransRecursiveTri( lp3, new, lp2 );
}

static void ReplaceClusterIndex( int newIndex, int oldindex, int **clusters,
								 IntListNode_t **vertLists, int *num_verts, int *new_num_verts ){
	int i, j;
	IntListNode_t *next;

	for ( j = 0; j < numJointsInSkeleton[g_skelModel.type]; ++j )
	{
		if ( !clusters[j] ) {
			continue;
		}

		for ( i = 0; i < num_verts[j + 1]; ++i )
		{
			if ( clusters[j][i] == oldindex ) {
				++new_num_verts[j + 1];

				next = vertLists[j];

				vertLists[j] = (IntListNode_t *) SafeMalloc( sizeof( IntListNode_t ), "ReplaceClusterIndex" );
				// Currently freed in WriteJointedModelFile only

				vertLists[j]->data = newIndex;
				vertLists[j]->next = next;
			}
		}
	}
}

#define FUDGE_EPSILON   0.002

qboolean VectorFudgeCompare( vec3_t v1, vec3_t v2 ){
	int i;

	for ( i = 0 ; i < 3 ; i++ )
		if ( fabs( v1[i] - v2[i] ) > FUDGE_EPSILON ) {
			return false;
		}

	return true;
}

/*
   =================
   Cmd_Base
   =================
 */
void Cmd_FMBase( qboolean GetST ){
	triangle_t  *ptri, *st_tri;
	int num_st_tris;
	int i, j, k, l;
	int x,y,z;
//	int			time1;
	char file1[1024],file2[1024],trans_file[1024], stfile[1024], extension[256];
	vec3_t base_xyz[MAX_FM_VERTS];
	FILE        *FH;
	int pos,bit;
	qboolean NewSkin;

	GetScriptToken( false );

	if ( g_skipmodel || g_release || g_archive ) {
		return;
	}

	printf( "---------------------\n" );
	sprintf( file1, "%s/%s.%s", cdarchive, token, trifileext );
	printf( "%s ", file1 );

	ExpandPathAndArchive( file1 );

	// Use the input filepath for this one.
	sprintf( file1, "%s/%s", cddir, token );

//	time1 = FileTime (file1);
//	if (time1 == -1)
//		Error ("%s doesn't exist", file1);

//
// load the base triangles
//
	if ( do3ds ) {
		Load3DSTriangleList( file1, &ptri, &fmheader.num_tris, &pmnodes, &fmheader.num_mesh_nodes );
	}
	else{
		LoadTriangleList( file1, &ptri, &fmheader.num_tris, &pmnodes, &fmheader.num_mesh_nodes );
	}

	if ( g_ignoreTriUV ) {
		for ( i = 0; i < fmheader.num_tris; i++ )
		{
			ptri[i].HasUV = 0;
		}
	}

	GetScriptToken( false );
	sprintf( file2, "%s/%s", cddir, token );
	sprintf( trans_file, "%s/!%s_a.pcx", cddir, token );

	ExtractFileExtension( file2, extension );
	if ( extension[0] == 0 ) {
		strcat( file2, ".pcx" );
	}
	printf( "skin: %s\n", file2 );

	BaseTrueColor = LoadAnyImage( file2, &BasePixels, &BasePalette, &BaseWidth, &BaseHeight );

	NewSkin = false;
	if ( BaseWidth != SKINPAGE_WIDTH || BaseHeight != SKINPAGE_HEIGHT ) {
		if ( g_allow_newskin ) {
			ScaleWidth = BaseWidth;
			ScaleHeight = BaseHeight;
			NewSkin = true;
		}
		else
		{
			Error( "Invalid skin page size: (%d,%d) should be (%d,%d)",
				   BaseWidth,BaseHeight,SKINPAGE_WIDTH,SKINPAGE_HEIGHT );
		}
	}
	else if ( !BaseTrueColor ) {
		ScaleWidth = (float)ExtractNumber( BasePixels, ENCODED_WIDTH_X,
										   ENCODED_WIDTH_Y );
		ScaleHeight = (float)ExtractNumber( BasePixels, ENCODED_HEIGHT_X,
											ENCODED_HEIGHT_Y );
	}
	else
	{
		Error( "Texture coordinates not supported on true color image" );
	}

	if ( GetST ) {
		GetScriptToken( false );

		sprintf( stfile, "%s/%s.%s", cdarchive, token, trifileext );
		printf( "ST: %s ", stfile );

		sprintf( stfile, "%s/%s", cddir, token );

		if ( do3ds ) {
			Load3DSTriangleList( stfile, &st_tri, &num_st_tris, NULL, NULL );
		}
		else{
			LoadTriangleList( stfile, &st_tri, &num_st_tris, NULL, NULL );
		}

		if ( num_st_tris != fmheader.num_tris ) {
			Error( "num st tris mismatch: st %d / base %d", num_st_tris, fmheader.num_tris );
		}

		printf( "   matching triangles...\n" );
		for ( i = 0; i < fmheader.num_tris; i++ )
		{
			k = -1;
			for ( j = 0; j < num_st_tris; j++ )
			{
				for ( x = 0; x < 3; x++ )
				{
					for ( y = 0; y < 3; y++ )
					{
						if ( x == y ) {
							continue;
						}
						for ( z = 0; z < 3; z++ )
						{
							if ( z == x || z == y ) {
								continue;
							}

							if ( VectorFudgeCompare( ptri[i].verts[0], st_tri[j].verts[x] ) &&
								 VectorFudgeCompare( ptri[i].verts[1], st_tri[j].verts[y] ) &&
								 VectorFudgeCompare( ptri[i].verts[2], st_tri[j].verts[z] ) ) {
								if ( k == -1 ) {
									k = j;
									ptri[i].HasUV = st_tri[k].HasUV;
									ptri[i].uv[0][0] = st_tri[k].uv[x][0];
									ptri[i].uv[0][1] = st_tri[k].uv[x][1];
									ptri[i].uv[1][0] = st_tri[k].uv[y][0];
									ptri[i].uv[1][1] = st_tri[k].uv[y][1];
									ptri[i].uv[2][0] = st_tri[k].uv[z][0];
									ptri[i].uv[2][1] = st_tri[k].uv[z][1];
									x = y = z = 999;
								}
								else if ( k != j ) {
									printf( "Duplicate triangle %d found in st file: %d and %d\n",i,k,j );
									printf( "   (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f)\n",
											ptri[i].verts[0][0],ptri[i].verts[0][1],ptri[i].verts[0][2],
											ptri[i].verts[1][0],ptri[i].verts[1][1],ptri[i].verts[1][2],
											ptri[i].verts[2][0],ptri[i].verts[2][1],ptri[i].verts[2][2] );
									printf( "   (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f)\n",
											st_tri[k].verts[0][0],st_tri[k].verts[0][1],st_tri[k].verts[0][2],
											st_tri[k].verts[1][0],st_tri[k].verts[1][1],st_tri[k].verts[1][2],
											st_tri[k].verts[2][0],st_tri[k].verts[2][1],st_tri[k].verts[2][2] );
									printf( "   (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f) (%0.3f %0.3f %0.3f)\n",
											st_tri[j].verts[0][0],st_tri[j].verts[0][1],st_tri[j].verts[0][2],
											st_tri[j].verts[1][0],st_tri[j].verts[1][1],st_tri[j].verts[1][2],
											st_tri[j].verts[2][0],st_tri[j].verts[2][1],st_tri[j].verts[2][2] );
								}
							}
						}
					}
				}
			}
			if ( k == -1 ) {
				printf( "No matching triangle %d\n",i );
			}
		}
		free( st_tri );
	}

//
// get the ST values
//
	if ( ptri && ptri[0].HasUV ) {
		if ( !NewSkin ) {
			Error( "Base has UVs with old style skin page\nMaybe you want to use -ignoreUV" );
		}
		else
		{
			BuildNewST( ptri, fmheader.num_tris, false );
		}
	}
	else
	{
		if ( NewSkin ) {
			Error( "Base has new style skin without UVs" );
		}
		else
		{
			BuildST( ptri, fmheader.num_tris, false );
		}
	}

	TransPixels = NULL;
	if ( !BaseTrueColor ) {
		FH = fopen( trans_file,"rb" );
		if ( FH ) {
			fclose( FH );
			Load256Image( trans_file, &TransPixels, NULL, &TransWidth, &TransHeight );
			if ( TransWidth != fmheader.skinwidth || TransHeight != fmheader.skinheight ) {
				Error( "source image %s dimensions (%d,%d) are not the same as alpha image (%d,%d)\n",file2,fmheader.skinwidth,fmheader.skinheight,TransWidth,TransHeight );
			}
		}
	}

//
// run through all the base triangles, storing each unique vertex in the
// base vertex list and setting the indirect triangles to point to the base
// vertices
//
	for ( l = 0; l < fmheader.num_mesh_nodes; l++ )
	{
		for ( i = 0 ; i < fmheader.num_tris ; i++ )
		{
			pos = i >> 3;
			bit = 1 << ( i & 7 );
			if ( !( pmnodes[l].tris[pos] & bit ) ) {
				continue;
			}

			for ( j = 0 ; j < 3 ; j++ )
			{
				// get the xyz index
				for ( k = 0 ; k < fmheader.num_xyz ; k++ )
				{
					if ( VectorCompare( ptri[i].verts[j], base_xyz[k] ) ) {
						break;  // this vertex is already in the base vertex list
					}
				}

				if ( k == fmheader.num_xyz ) { // new index
					VectorCopy( ptri[i].verts[j], base_xyz[fmheader.num_xyz] );

					if ( pmnodes[l].clustered == true ) {
						ReplaceClusterIndex( k, ptri[i].indicies[j], (int **)&pmnodes[l].clusters, (IntListNode_t **)&g_skelModel.vertLists, (int *)&pmnodes[l].num_verts, (int *)&g_skelModel.new_num_verts );
					}

					fmheader.num_xyz++;
				}

				pos = k >> 3;
				bit = 1 << ( k & 7 );
				pmnodes[l].verts[pos] |= bit;

				triangles[i].index_xyz[j] = k;

				// get the st index
				for ( k = 0 ; k < fmheader.num_st ; k++ )
				{
					if ( triangle_st[i][j][0] == base_st[k].s
						 && triangle_st[i][j][1] == base_st[k].t ) {
						break;  // this vertex is already in the base vertex list
					}
				}

				if ( k == fmheader.num_st ) { // new index
					base_st[fmheader.num_st].s = triangle_st[i][j][0];
					base_st[fmheader.num_st].t = triangle_st[i][j][1];
					fmheader.num_st++;
				}

				triangles[i].index_st[j] = k;
			}

			if ( TransPixels || BaseTrueColor ) {
				translucent[i] = CheckTransRecursiveTri( triangle_st[i][0], triangle_st[i][1], triangle_st[i][2] );
			}
			else
			{
				translucent[i] = false;
			}
		}
	}

	if ( !BaseTrueColor ) {
		SetPixel = true;
		memset( BasePixels,0,BaseWidth * BaseHeight );
		for ( i = 0 ; i < fmheader.num_tris ; i++ )
		{
			CheckTransRecursiveTri( triangle_st[i][0], triangle_st[i][1], triangle_st[i][2] );
		}
		SetPixel = false;

		skin_pixels_used = 0;
		for ( i = 0; i < fmheader.skinheight; i++ )
		{
			for ( j = 0; j < fmheader.skinwidth; j++ )
			{
				skin_pixels_used += BasePixels[( i * BaseWidth ) + j];
			}
		}
		total_skin_pixels = fmheader.skinheight * fmheader.skinwidth;
	}
	else
	{
		SetPixel = true;
		memset( BasePixels,0,BaseWidth * BaseHeight * 4 );
		for ( i = 0 ; i < fmheader.num_tris ; i++ )
		{
			CheckTransRecursiveTri( triangle_st[i][0], triangle_st[i][1], triangle_st[i][2] );
		}
		SetPixel = false;

		skin_pixels_used = 0;
		for ( i = 0; i < fmheader.skinheight; i++ )
		{
			for ( j = 0; j < fmheader.skinwidth; j++ )
			{
				skin_pixels_used += BasePixels[( ( i * BaseWidth ) + j ) * 4];
			}
		}
		total_skin_pixels = fmheader.skinheight * fmheader.skinwidth;
	}

	// build triangle strips / fans
	BuildGlCmds();

	if ( TransPixels ) {
		free( TransPixels );
	}
	free( BasePixels );
	if ( BasePalette ) {
		free( BasePalette );
	}
	free( ptri );
}

void Cmd_FMNodeOrder( void ){
	mesh_node_t *newnodes, *pos;
	int i,j;

	if ( !pmnodes ) {
		Error( "Base has not been established yet" );
	}

	pos = newnodes = malloc( sizeof( mesh_node_t ) * fmheader.num_mesh_nodes );

	for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
	{
		GetScriptToken( false );

		for ( j = 0; j < fmheader.num_mesh_nodes; j++ )
		{
			if ( strcmpi( pmnodes[j].name, token ) == 0 ) {
				*pos = pmnodes[j];
				pos++;
				break;
			}
		}
		if ( j >= fmheader.num_mesh_nodes ) {
			Error( "Node '%s' not in base list!\n", token );
		}
	}

	free( pmnodes );
	pmnodes = newnodes;
}

//===============================================================

extern char *FindFrameFile( char *frame );


/*
   ===============
   GrabFrame
   ===============
 */
void GrabFrame( char *frame ){
	triangle_t      *ptri;
	int i, j;
	fmtrivert_t     *ptrivert;
	int num_tris;
	char file1[1024];
	fmframe_t       *fr;
	int index_xyz;
	char            *framefile;

	// the frame 'run1' will be looked for as either
	// run.1 or run1.tri, so the new alias sequence save
	// feature an be used
	framefile = FindFrameFile( frame );

	sprintf( file1, "%s/%s", cdarchive, framefile );
	ExpandPathAndArchive( file1 );

	sprintf( file1, "%s/%s",cddir, framefile );

	printf( "grabbing %s  ", file1 );

	if ( fmheader.num_frames >= MAX_FM_FRAMES ) {
		Error( "fmheader.num_frames >= MAX_FM_FRAMES" );
	}
	fr = &g_frames[fmheader.num_frames];
	fmheader.num_frames++;

	strcpy( fr->name, frame );

//
// load the frame
//
	if ( do3ds ) {
		Load3DSTriangleList( file1, &ptri, &num_tris, NULL, NULL );
	}
	else{
		LoadTriangleList( file1, &ptri, &num_tris, NULL, NULL );
	}

	if ( num_tris != fmheader.num_tris ) {
		Error( "%s: number of triangles (%d) doesn't match base frame (%d)\n", file1, num_tris, fmheader.num_tris );
	}

//
// allocate storage for the frame's vertices
//
	ptrivert = fr->v;

	for ( i = 0 ; i < fmheader.num_xyz ; i++ )
	{
		ptrivert[i].vnorm.numnormals = 0;
		VectorClear( ptrivert[i].vnorm.normalsum );
	}
	ClearBounds( fr->mins, fr->maxs );

//
// store the frame's vertices in the same order as the base. This assumes the
// triangles and vertices in this frame are in exactly the same order as in the
// base
//
	for ( i = 0 ; i < num_tris ; i++ )
	{
		vec3_t vtemp1, vtemp2, normal;
		float ftemp;

		VectorSubtract( ptri[i].verts[0], ptri[i].verts[1], vtemp1 );
		VectorSubtract( ptri[i].verts[2], ptri[i].verts[1], vtemp2 );
		CrossProduct( vtemp1, vtemp2, normal );

		VectorNormalize( normal, normal );

		// rotate the normal so the model faces down the positive x axis
		ftemp = normal[0];
		normal[0] = -normal[1];
		normal[1] = ftemp;

		for ( j = 0 ; j < 3 ; j++ )
		{
			index_xyz = triangles[i].index_xyz[j];

			// rotate the vertices so the model faces down the positive x axis
			// also adjust the vertices to the desired origin
			ptrivert[index_xyz].v[0] = ( ( -ptri[i].verts[j][1] ) * scale_up ) +
									   adjust[0];
			ptrivert[index_xyz].v[1] = ( ptri[i].verts[j][0] * scale_up ) +
									   adjust[1];
			ptrivert[index_xyz].v[2] = ( ptri[i].verts[j][2] * scale_up ) +
									   adjust[2];

			AddPointToBounds( ptrivert[index_xyz].v, fr->mins, fr->maxs );

			VectorAdd( ptrivert[index_xyz].vnorm.normalsum, normal, ptrivert[index_xyz].vnorm.normalsum );
			ptrivert[index_xyz].vnorm.numnormals++;
		}
	}

//
// calculate the vertex normals, match them to the template list, and store the
// index of the best match
//
	for ( i = 0 ; i < fmheader.num_xyz ; i++ )
	{
		int j;
		vec3_t v;
		float maxdot;
		int maxdotindex;
		int c;

		c = ptrivert[i].vnorm.numnormals;
		if ( !c ) {
			Error( "Vertex with no triangles attached" );
		}

		VectorScale( ptrivert[i].vnorm.normalsum, 1.0 / c, v );
		VectorNormalize( v, v );

		maxdot = -999999.0;
		maxdotindex = -1;

		for ( j = 0 ; j < NUMVERTEXNORMALS ; j++ )
		{
			float dot;

			dot = DotProduct( v, avertexnormals[j] );
			if ( dot > maxdot ) {
				maxdot = dot;
				maxdotindex = j;
			}
		}

		ptrivert[i].lightnormalindex = maxdotindex;
	}

	free( ptri );
}

/*
   ===============
   Cmd_Frame
   ===============
 */
void Cmd_FMFrame( void ){
	while ( ScriptTokenAvailable() )
	{
		GetScriptToken( false );
		if ( g_skipmodel ) {
			continue;
		}
		if ( g_release || g_archive ) {
			fmheader.num_frames = 1;    // don't skip the writeout
			continue;
		}

		H_printf( "#define FRAME_%-16s\t%i\n", token, fmheader.num_frames );

		if ( ( g_skelModel.type != SKEL_NULL ) || ( g_skelModel.references != REF_NULL ) ) {
			GrabModelTransform( token );
		}

		GrabFrame( token );

		if ( g_skelModel.type != SKEL_NULL ) {
			GrabSkeletalFrame( token );
		}

		if ( g_skelModel.references != REF_NULL ) {
			GrabReferencedFrame( token );
		}

		// need to add the up and dir points to the frame bounds here
		// using AddPointToBounds (ptrivert[index_xyz].v, fr->mins, fr->maxs);
		// then remove fudge in determining scale on frame write out
	}
}

/*
   ===============
   Cmd_Skin

   Skins aren't actually stored in the file, only a reference
   is saved out to the header file.
   ===============
 */
void Cmd_FMSkin( void ){
	byte        *palette;
	byte        *pixels;
	int width, height;
	byte        *cropped;
	int y;
	char name[1024], savename[1024], transname[1024], extension[256];
	miptex32_t  *qtex32;
	int size;
	FILE        *FH;
	qboolean TrueColor;

	GetScriptToken( false );

	if ( fmheader.num_skins == MAX_FM_SKINS ) {
		Error( "fmheader.num_skins == MAX_FM_SKINS" );
	}

	if ( g_skipmodel ) {
		return;
	}

	sprintf( name, "%s/%s", cdarchive, token );
	strcpy( name, ExpandPathAndArchive( name ) );
//	sprintf (name, "%s/%s.lbm", cddir, token);

	if ( ScriptTokenAvailable() ) {
		GetScriptToken( false );
		sprintf( g_skins[fmheader.num_skins], "!%s", token );
		sprintf( savename, "%s!%s", g_outputDir, token );
		sprintf( transname, "%s!%s_a.pcx", gamedir, token );
	}
	else
	{
		sprintf( g_skins[fmheader.num_skins], "%s/!%s", cdpartial, token );
		sprintf( savename, "%s/!%s", g_outputDir, token );
		sprintf( transname, "%s/!%s_a.pcx", cddir, token );
	}

	fmheader.num_skins++;

	if ( g_skipmodel || g_release || g_archive ) {
		return;
	}

	// load the image
	printf( "loading %s\n", name );
	ExtractFileExtension( name, extension );
	if ( extension[0] == 0 ) {
		strcat( name, ".pcx" );
	}


	TrueColor = LoadAnyImage( name, &pixels, &palette, &width, &height );
//	RemapZero (pixels, palette, width, height);

	// crop it to the proper size

	if ( !TrueColor ) {
		cropped = (byte *) SafeMalloc( fmheader.skinwidth * fmheader.skinheight, "Cmd_FMSkin" );
		for ( y = 0 ; y < fmheader.skinheight ; y++ )
		{
			memcpy( cropped + y * fmheader.skinwidth,
					pixels + y * width, fmheader.skinwidth );
		}

		TransPixels = NULL;
		FH = fopen( transname,"rb" );
		if ( FH ) {
			fclose( FH );

			strcat( g_skins[fmheader.num_skins - 1],".pcx" );
			strcat( savename,".pcx" );

			// save off the new image
			printf( "saving %s\n", savename );
			CreatePath( savename );
			WritePCXfile( savename, cropped, fmheader.skinwidth, fmheader.skinheight, palette );
		}
		else
		{
	#if 1
			miptex_t    *qtex;
			qtex = CreateMip( cropped, fmheader.skinwidth, fmheader.skinheight, palette, &size, true );

			strcat( g_skins[fmheader.num_skins - 1],".m8" );
			strcat( savename,".m8" );

			printf( "saving %s\n", savename );
			CreatePath( savename );
			SaveFile( savename, (byte *)qtex, size );
			free( qtex );
	#else
			strcat( g_skins[fmheader.num_skins - 1],".pcx" );
			strcat( savename,".pcx" );

			// save off the new image
			printf( "saving %s\n", savename );
			CreatePath( savename );
			WritePCXfile( savename, cropped, fmheader.skinwidth, fmheader.skinheight, palette );
	#endif
		}
	}
	else
	{
		cropped = (byte *) SafeMalloc( fmheader.skinwidth * fmheader.skinheight * 4, "Cmd_FMSkin" );
		for ( y = 0 ; y < fmheader.skinheight ; y++ )
		{
			memcpy( cropped + ( ( y * fmheader.skinwidth ) * 4 ), pixels + ( y * width * 4 ), fmheader.skinwidth * 4 );
		}

		qtex32 = CreateMip32( (unsigned *)cropped, fmheader.skinwidth, fmheader.skinheight, &size, true );

		StripExtension( g_skins[fmheader.num_skins - 1] );
		strcat( g_skins[fmheader.num_skins - 1],".m32" );
		StripExtension( savename );
		strcat( savename,".m32" );

		printf( "saving %s\n", savename );
		CreatePath( savename );
		SaveFile( savename, (byte *)qtex32, size );
	}

	free( pixels );
	if ( palette ) {
		free( palette );
	}
	free( cropped );
}


/*
   ===============
   Cmd_Cd
   ===============
 */
void Cmd_FMCd( void ){
	char temp[256];

	FinishModel();
	ClearModel();

	GetScriptToken( false );

	// this is a silly mess...
	sprintf( cdpartial, "models/%s", token );
	sprintf( cdarchive, "%smodels/%s", gamedir + strlen( qdir ), token );
	sprintf( cddir, "%s%s", gamedir, cdpartial );

	// Since we also changed directories on the output side (for mirror) make sure the outputdir is set properly too.
	sprintf( temp, "%s%s", g_outputDir, cdpartial );
	strcpy( g_outputDir, temp );

	// if -only was specified and this cd doesn't match,
	// skip the model (you only need to match leading chars,
	// so you could regrab all monsters with -only monsters)
	if ( !g_only[0] ) {
		return;
	}
	if ( strncmp( token, g_only, strlen( g_only ) ) ) {
		g_skipmodel = true;
		printf( "skipping %s\n", cdpartial );
	}
}


/*

   //=======================
   //		NEW GEN
   //=======================

   void NewGen (char *ModelFile, char *OutputName, int width, int height)
   {
    trigroup_t  *triangles;
    triangle_t	*ptri;
    triangle_t	*grouptris;
    mesh_node_t	*pmnodes;

    vec3_t		*vertices;
    vec3_t		*uvs;
    vec3_t		aveNorm, crossvect;
    vec3_t		diffvect1, diffvect2;
    vec3_t		v0, v1, v2;
    vec3_t		n, u, v;
    vec3_t		base, zaxis, yaxis;
    vec3_t		uvwMin, uvwMax;
    vec3_t		groupMin, groupMax;
    vec3_t		uvw;

    float		*uFinal, *vFinal;
    unsigned char	*newpic;

    int			finalstart = 0, finalcount = 0;
    int			xbase = 0, xwidth = 0, ywidth = 0;
    int			*todo, *done, finished;
    int			i, j, k, l; //counters
    int			groupnum, numtris, numverts, num;
    int			count;
    FILE		*grpfile;
    long		datasize;

    for ( i = 0; i<3; i++)
    {
        aveNorm[i] = 0;
        uvwMin[i] = 1e30f;
        uvwMax[i] = -1e30f;
    }

    pmnodes = NULL;
    ptri = NULL;
    triangles = NULL;

    zaxis[0] = 0;
    zaxis[1] = 0;
    zaxis[2] = 1;

    yaxis[0] = 0;
    yaxis[1] = 1;
    yaxis[2] = 0;

    LoadTriangleList (ModelFile, &ptri, &fmheader.num_tris, &pmnodes, &fmheader.num_mesh_nodes);

    todo = (int*)SafeMalloc(fmheader.num_tris*sizeof(int), "NewGen");
    done = (int*)SafeMalloc(fmheader.num_tris*sizeof(int), "NewGen");
    triangles = (trigroup_t*)SafeMalloc(fmheader.num_tris*sizeof(trigroup_t), "NewGen");

    for ( i=0; i < fmheader.num_tris; i++)
    {
        todo[i] = false;
        done[i] = false;
        triangles[i].triangle = ptri[i];
        triangles[i].group = 0;
    }

    groupnum = 0;

   //  transitive closure algorithm follows
   //  put all triangles who transitively share vertices into separate groups

    while (1)
    {
        for ( i = 0; i < fmheader.num_tris; i++)
        {
            if (!done[i])
            {
                break;
            }
        }
        if ( i == fmheader.num_tris)
        {
            break;
        }
        finished = false;
        todo[i] = true;
        while (!finished)
        {
            finished = true;
            for ( i = 0; i < fmheader.num_tris; i++)
            {
                if (todo[i])
                {
                    done[i] = true;
                    triangles[i].group = groupnum;
                    todo[i] = false;
                    for ( j = 0; j < fmheader.num_tris; j++)
                    {
                        if ((!done[j]) && (ShareVertex(triangles[i],triangles[j])))
                        {
                            todo[j] = true;
                            finished = false;
                        }
                    }
                }
            }
        }
        groupnum++;
    }
        uFinal = (float*)SafeMalloc(3*fmheader.num_tris*sizeof(float), "NewGen");
        vFinal = (float*)SafeMalloc(3*fmheader.num_tris*sizeof(float), "NewGen");

    grpfile = fopen("grpdebug.txt","w");


    for (i = 0; i < groupnum; i++)
    {

        fprintf(grpfile,"Group Number: %d\n", i);

        numtris = GetNumTris(triangles, i); // number of triangles in group i
        numverts = numtris * 3;

        fprintf(grpfile,"%d triangles.\n", numtris);

        vertices = (vec3_t*)SafeMalloc(numverts*sizeof(vec3_t), "NewGen");
        uvs = (vec3_t*)SafeMalloc(numverts*sizeof(vec3_t), "NewGen");
        grouptris = (triangle_t*)SafeMalloc(numtris*sizeof(triangle_t), "NewGen");

        for (count = 0; count < fmheader.num_tris; count++)
        {
            if (triangles[count].group == i)
            {
                fprintf(grpfile,"Triangle %d\n", count);
            }
        }
        fprintf(grpfile,"\n");




        GetOneGroup(triangles, i, grouptris);

        num = 0;
        for (j = 0; j < numtris; j++)
        {
            VectorCopy(grouptris[j].verts[0], v0);
            VectorCopy(grouptris[j].verts[1], v1);
            VectorCopy(grouptris[j].verts[2], v2);
            VectorSubtract(v1, v0, diffvect1);
            VectorSubtract(v2, v1, diffvect2);
            CrossProduct( diffvect1, diffvect2, crossvect);
            VectorAdd(aveNorm, crossvect, aveNorm);
            VectorCopy(v0,vertices[num]);
            num++;					//  FIXME
            VectorCopy(v1,vertices[num]);
            num++;					//  add routine to add only verts that
            VectorCopy(v2,vertices[num]);
            num++;					// have not already been added
        }

        assert (num >= 3);
   // figure out the best plane projections
        DOsvdPlane ((float*)vertices, num, (float *)&n, (float *)&base);

        if (DotProduct(aveNorm,n) < 0.0f)
        {
            VectorScale(n, -1.0f, n);
        }
        VectorNormalize(n,n);
        if (fabs(n[2]) < .57)
        {
            CrossProduct( zaxis, n, crossvect);
            VectorCopy(crossvect, u);
        }
        else
        {
            CrossProduct( yaxis, n, crossvect);
            VectorCopy(crossvect, u);
        }
        VectorNormalize(u,u);
        CrossProduct( n, u, crossvect);
        VectorCopy(crossvect, v);
        VectorNormalize(v,v);

        num = 0;

        for ( j = 0; j < 3; j++)
        {
            groupMin[j] = 1e30f;
            groupMax[j] = -1e30f;
        }

        for ( j = 0; j < numtris; j++)
        {
            for ( k = 0; k < 3; k++)
            {
                VectorCopy(grouptris[j].verts[k],v0);
                VectorSubtract(v0, base, v0);
                uvw[0] = DotProduct(v0, u);
                uvw[1] = DotProduct(v0, v);
                uvw[2] = DotProduct(v0, n);
                VectorCopy(uvw,uvs[num]);
                num++;
                for ( l = 0; l < 3; l++)
                {
                    if (uvw[l] < groupMin[l])
                    {
                        groupMin[l] = uvw[l];
                    }
                    if (uvw[l] > groupMax[l])
                    {
                        groupMax[l] = uvw[l];
                    }
                }
            }
        }

        xwidth = ceil(0 - groupMin[0]) + 2; // move right of origin and avoid overlap
        ywidth = ceil(0 - groupMin[1]) + 2; // move "above" origin

        for ( j=0; j < numverts; j++)
        {
            uFinal[finalcount] = uvs[j][0] + xwidth + xbase;
            vFinal[finalcount] = uvs[j][1] + ywidth;
            if (uFinal[finalcount] < uvwMin[0])
            {
                uvwMin[0] = uFinal[finalcount];
            }
            if (uFinal[finalcount] > uvwMax[0])
            {
                uvwMax[0] = uFinal[finalcount];
            }
            if (vFinal[finalcount] < uvwMin[1])
            {
                uvwMin[1] = vFinal[finalcount];
            }
            if (vFinal[finalcount] > uvwMax[1])
            {
                uvwMax[1] = vFinal[finalcount];
            }
            finalcount++;
        }

        fprintf(grpfile,"svdPlaned Group min: ( %f , %f )\n",groupMin[0] + xwidth + xbase, groupMin[1] + ywidth);
        fprintf(grpfile,"svdPlaned Group max: ( %f , %f )\n",groupMax[0] + xwidth + xbase, groupMax[1] + ywidth);

        finalcount = finalstart;

        for ( count = 0; count < numverts; count++)
        {
            fprintf(grpfile,"Vertex %d: ( %f , %f , %f )\n",count,vertices[count][0],vertices[count][1],vertices[count][2]);
            fprintf(grpfile,"svdPlaned: ( %f , %f )\n",uFinal[finalcount],vFinal[finalcount++]);
        }

        finalstart = finalcount;

        fprintf(grpfile,"\n");

        free(vertices);
        free(uvs);
        free(grouptris);

        xbase += ceil(groupMax[0] - groupMin[0]) + 2;

    }

    fprintf(grpfile,"Global Min ( %f , %f )\n",uvwMin[0],uvwMin[1]);
    fprintf(grpfile,"Global Max ( %f , %f )\n",uvwMax[0],uvwMax[1]);


    ScaleTris(uvwMin, uvwMax, width, height, uFinal, vFinal, finalcount);

    for (k = 0; k < finalcount; k++)
    {
        fprintf(grpfile, "scaled vertex %d: ( %f , %f )\n",k,uFinal[k],vFinal[k]);
    }

    //  i've got the array of vertices in uFinal and vFinal.  Now I need to write them and draw lines

    datasize = width * height*sizeof(unsigned char);
    newpic = (unsigned char*)SafeMalloc(datasize, "NewGen");
    memset(newpic,0,datasize);
    memset(pic_palette,0,sizeof(pic_palette));
    pic_palette[767] = pic_palette[766] = pic_palette[765] = 255;

    k = 0;
    while (k < finalcount)
    {
        NewDrawLine(uFinal[k], vFinal[k], uFinal[k+1], vFinal[k+1], newpic, width, height);
        k++;
        NewDrawLine(uFinal[k], vFinal[k], uFinal[k+1], vFinal[k+1], newpic, width, height);
        k++;
        NewDrawLine(uFinal[k], vFinal[k], uFinal[k-2], vFinal[k-2], newpic, width, height);
        k++;
        fprintf(grpfile, "output tri with verts %d, %d, %d", k-2, k-1, k);
    }

    WritePCXfile (OutputName, newpic, width, height, pic_palette);

    fclose(grpfile);

    free(todo);
    free(done);
    free(triangles);
    free(newpic);
    return;
   }
   void NewDrawLine(int x1, int y1, int x2, int y2, unsigned char* picture, int width, int height)
   {
    long dx, dy;
    long adx, ady;
    long count;
    float xfrac, yfrac, xstep, ystep;
    unsigned long sx, sy;
    float u, v;

    dx = x2 - x1;
    dy = y2 - y1;
    adx = abs(dx);
    ady = abs(dy);

    count = adx > ady ? adx : ady;
    count++;

    if(count > 300)
    {
        printf("Bad count\n");
        return; // don't ever hang up on bad data
    }

    xfrac = x1;
    yfrac = y1;

    xstep = (float)dx/count;
    ystep = (float)dy/count;

    switch(LineType)
    {
        case LINE_NORMAL:
            do
            {
                if(xfrac < width && yfrac < height)
                {
                    picture[(long)yfrac*width+(long)xfrac] = LineColor;
                }
                xfrac += xstep;
                yfrac += ystep;
                count--;
            } while (count > 0);
            break;
        case LINE_FAT:
            do
            {
                for (u=-0.1 ; u<=0.9 ; u+=0.999)
                {
                    for (v=-0.1 ; v<=0.9 ; v+=0.999)
                    {
                        sx = xfrac+u;
                        sy = yfrac+v;
                        if(sx < width && sy < height)
                        {
                            picture[sy*width+sx] = LineColor;
                        }
                    }
                }
                xfrac += xstep;
                yfrac += ystep;
                count--;
            } while (count > 0);
            break;
        case LINE_DOTTED:
            do
            {
                if(count&1 && xfrac < width &&
                    yfrac < height)
                {
                    picture[(long)yfrac*width+(long)xfrac] = LineColor;
                }
                xfrac += xstep;
                yfrac += ystep;
                count--;
            } while (count > 0);
            break;
        default:
            Error("Unknown <linetype> %d.\n", LineType);
    }
   }
 */
void ScaleTris( vec3_t min, vec3_t max, int Width, int Height, float* u, float* v, int verts ){

	int i;
	float hscale, vscale;
	float scale;

	hscale = max[0];
	vscale = max[1];

	hscale = ( Width - 2 ) / max[0];
	vscale = ( Height - 2 ) / max[1];

	scale = hscale;
	if ( scale > vscale ) {
		scale = vscale;
	}
	for ( i = 0; i < verts; i++ )
	{
		u[i] *= scale;
		v[i] *= scale;
	}
	return;
}


void GetOneGroup( trigroup_t *tris, int grp, triangle_t* triangles ){
	int i;
	int j;

	j = 0;
	for ( i = 0; i < fmheader.num_tris; i++ )
	{
		if ( tris[i].group == grp ) {
			triangles[j++] = tris[i].triangle;
		}
	}
	return;
}


int GetNumTris( trigroup_t *tris, int grp ){
	int i;
	int verts;

	verts = 0;
	for ( i = 0; i < fmheader.num_tris; i++ )
	{
		if ( tris[i].group == grp ) {
			verts++;
		}
	}
	return verts;
}


int ShareVertex( trigroup_t trione, trigroup_t tritwo ){
	int i;
	int j;

	i = 1;
	j = 1;
	for ( i = 0; i < 3; i++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			if ( DistBetween( trione.triangle.verts[i],tritwo.triangle.verts[j] ) < TRIVERT_DIST ) {
				return true;
			}
		}
	}
	return false;
}


float DistBetween( vec3_t point1, vec3_t point2 ){
	float dist;

	dist = ( point1[0] - point2[0] );
	dist *= dist;
	dist += ( point1[1] - point2[1] ) * ( point1[1] - point2[1] );
	dist += ( point1[2] - point2[2] ) * ( point1[2] - point2[2] );
	dist = sqrt( dist );
	return dist;
}


void GenSkin( char *ModelFile, char *OutputName, int Width, int Height ){
	triangle_t  *ptri;
	mesh_node_t *pmnodes;
	int i;

	pmnodes = NULL;
	ptri = NULL;

	LoadTriangleList( ModelFile, &ptri, &fmheader.num_tris, &pmnodes, &fmheader.num_mesh_nodes );
	if ( g_ignoreTriUV ) {
		for ( i = 0; i < fmheader.num_tris; i++ )
		{
			ptri[i].HasUV = 0;
		}
	}

	memset( pic,0,sizeof( pic ) );
	memset( pic_palette,0,sizeof( pic_palette ) );
	pic_palette[767] = pic_palette[766] = pic_palette[765] = 255;

	ScaleWidth = Width;
	ScaleHeight = Height;

	BuildST( ptri, fmheader.num_tris, true );

	WritePCXfile( OutputName, pic, SKINPAGE_WIDTH, SKINPAGE_HEIGHT, pic_palette );

	printf( "Gen Skin Stats:\n" );
	printf( "   Input Base: %s\n",ModelFile );
	printf( "   Input Dimensions: %d,%d\n",Width,Height );
	printf( "\n" );
	printf( "   Output File: %s\n",OutputName );
	printf( "   Output Dimensions: %d,%d\n",ScaleWidth,ScaleHeight );

	if ( fmheader.num_mesh_nodes ) {
		printf( "\nNodes:\n" );
		for ( i = 0; i < fmheader.num_mesh_nodes; i++ )
		{
			printf( "   %s\n",pmnodes[i].name );
		}
	}

	free( ptri );
	free( pmnodes );
}


void Cmd_FMBeginGroup( void ){
	GetScriptToken( false );

	g_no_opimizations = false;

	groups[num_groups].start_frame = fmheader.num_frames;
	groups[num_groups].num_frames = 0;

	groups[num_groups].degrees = atol( token );
	if ( groups[num_groups].degrees < 1 || groups[num_groups].degrees > 32 ) {
		Error( "Degrees of freedom out of range: %d",groups[num_groups].degrees );
	}
}

void Cmd_FMEndGroup( void ){
	groups[num_groups].num_frames = fmheader.num_frames - groups[num_groups].start_frame;

	if ( num_groups < MAX_GROUPS - 1 ) {
		num_groups++;
	}
	else
	{
		Error( "Number of compression groups exceded: %i\n", MAX_GROUPS );
	}
}
