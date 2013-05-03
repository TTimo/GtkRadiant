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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "gensurf.h"

double xmin,xmax,ymin,ymax,zmin,zmax;
double backface;
extern double dh, dv;
FILE  *fmap;
XYZ xyz[MAX_ROWS + 1][MAX_ROWS + 1];
int contents;
int surface[3];
LPVOID h_func_group;
LPVOID terrainkey;  // ^Fishman - Add terrain key to func_group.

//=============================================================
// Hydra : snap-to-grid begin
double CalculateSnapValue( double value ){
	long snapvalue;

	// simple uncomplicated snapping, rounding both UP and DOWN to the nearest
	// grid unit.
	if ( SnapToGrid > 0 ) {
		snapvalue = (int)value / SnapToGrid;
		if ( (long)value % SnapToGrid < ( SnapToGrid / 2 ) ) { // Snap Downwards if less than halfway between to grid units
			value = snapvalue * SnapToGrid;
		}
		else{ // Snap Upwards if more than halfway between to grid units
			value = ( snapvalue + 1 ) * SnapToGrid;
		}
	}
	return value;
}
// Hydra : snap-to-grid end

//=============================================================
bool ValidSurface(){
	if ( WaveType == WAVE_BITMAP && !gbmp.colors ) {
		return FALSE;
	}
	if ( NH  < 1 ) {
		return FALSE;
	}
	if ( NH  > MAX_ROWS ) {
		return FALSE;
	}
	if ( NV  < 1 ) {
		return FALSE;
	}
	if ( NV  > MAX_ROWS ) {
		return FALSE;
	}
	if ( Hll >= Hur ) {
		return FALSE;
	}
	if ( Vll >= Vur ) {
		return FALSE;
	}
	return TRUE;
}

//=============================================================
int MapPatches(){
	int NH_remain;
	int NV_remain;
	int NH_patch;
	int NV_patch;
	int BrushNum = 0;
	int i, j, k1, k2, k3;
	int i0, j0, ii;
	char szOops[128];
	patchMesh_t p;

	dh = ( Hur - Hll ) / NH;
	dv = ( Vur - Vll ) / NV;
	memset( &p,0,sizeof( patchMesh_t ) );

	// Generate control points in pp array to give desired values currently
	// in p array.
	switch ( Plane )
	{
	case PLANE_XY0:
	case PLANE_XY1:
		k1 = 0;
		k2 = 1;
		k3 = 2;
		break;
	case PLANE_XZ0:
	case PLANE_XZ1:
		k1 = 0;
		k2 = 2;
		k3 = 1;
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		k1 = 1;
		k2 = 2;
		k3 = 0;
		break;
	}
	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			xyz[i][j].pp[k1] = xyz[i][j].p[k1];
			xyz[i][j].pp[k2] = xyz[i][j].p[k2];
		}
	}
	for ( i = 0; i <= NH; i += 2 )
	{
		for ( j = 0; j <= NV; j += 2 )
			xyz[i][j].pp[k3] = xyz[i][j].p[k3];
	}
	for ( i = 1; i < NH; i += 2 )
	{
		for ( j = 0; j <= NV; j += 2 )
		{
			xyz[i][j].pp[k3] = ( 4 * xyz[i][j].p[k3] - xyz[i - 1][j].p[k3] - xyz[i + 1][j].p[k3] ) / 2;
		}
	}
	for ( j = 1; j < NV; j += 2 )
	{
		for ( i = 0; i <= NH; i += 2 )
		{
			xyz[i][j].pp[k3] = ( 4 * xyz[i][j].p[k3] - xyz[i][j - 1].p[k3] - xyz[i][j + 1].p[k3] ) / 2;
		}
	}
	for ( i = 1; i < NH; i += 2 )
	{
		for ( j = 1; j < NV; j += 2 )
		{
			xyz[i][j].pp[k3] = ( 16 * xyz[i][j].p[k3] - xyz[i - 1][j - 1].p[k3] - 2 * xyz[i][j - 1].p[k3]
								 - xyz[i + 1][j - 1].p[k3] - 2 * xyz[i - 1][j].p[k3] - 2 * xyz[i + 1][j].p[k3]
								 - xyz[i - 1][j + 1].p[k3] - 2 * xyz[i][j + 1].p[k3] - xyz[i + 1][j + 1].p[k3] ) / 4;
		}
	}

	NH_remain = NH + 1;
	i0 = 0;
	while ( NH_remain > 1 )
	{
		if ( ( ( NH_remain - 1 ) % 14 ) == 0 ) {
			NH_patch = 15;
		}
		else if ( ( ( NH_remain - 1 ) % 12 ) == 0 ) {
			NH_patch = 13;
		}
		else if ( ( ( NH_remain - 1 ) % 10 ) == 0 ) {
			NH_patch = 11;
		}
		else if ( ( ( NH_remain - 1 ) % 8 ) == 0 ) {
			NH_patch = 9;
		}
		else if ( ( ( NH_remain - 1 ) % 6 ) == 0 ) {
			NH_patch = 7;
		}
		else if ( ( ( NH_remain - 1 ) % 4 ) == 0 ) {
			NH_patch = 5;
		}
		else if ( ( ( NH_remain - 1 ) % 2 ) == 0 ) {
			NH_patch = 3;
		}
		else if ( NH_remain > 16 ) {
			NH_patch = 7;
		}
		else if ( NH_remain > 4 ) {
			NH_patch = 5;
		}
		else{
			NH_patch = 3;
		}
		while ( NH_patch > 3 && ( NH_patch - 1 ) * dh > 512 )
			NH_patch -= 2;
		NH_remain -= ( NH_patch - 1 );
		if ( NH_remain < 0 ) {
			sprintf( szOops,"Oops... screwed up with NH=%d",NH );
			g_FuncTable.m_pfnMessageBox( NULL,szOops,"Uh oh", 0, NULL );
		}
		NV_remain = NV + 1;
		j0 = 0;
		while ( NV_remain > 1 )
		{
			if ( ( ( NV_remain - 1 ) % 14 ) == 0 ) {
				NV_patch = 15;
			}
			else if ( ( ( NV_remain - 1 ) % 12 ) == 0 ) {
				NV_patch = 13;
			}
			else if ( ( ( NV_remain - 1 ) % 10 ) == 0 ) {
				NV_patch = 11;
			}
			else if ( ( ( NV_remain - 1 ) % 8 ) == 0 ) {
				NV_patch = 9;
			}
			else if ( ( ( NV_remain - 1 ) % 6 ) == 0 ) {
				NV_patch = 7;
			}
			else if ( ( ( NV_remain - 1 ) % 4 ) == 0 ) {
				NV_patch = 5;
			}
			else if ( ( ( NV_remain - 1 ) % 2 ) == 0 ) {
				NV_patch = 3;
			}
			else if ( NV_remain > 16 ) {
				NV_patch = 7;
			}
			else if ( NV_remain > 4 ) {
				NV_patch = 5;
			}
			else{
				NV_patch = 3;
			}
			while ( NV_patch > 3 && ( NV_patch - 1 ) * dh > 512 )
				NV_patch -= 2;
			NV_remain -= ( NV_patch - 1 );
			if ( NV_remain < 0 ) {
				sprintf( szOops,"Oops... screwed up with NV=%d",NV );
				g_FuncTable.m_pfnMessageBox( NULL,szOops,"Uh oh", 0, NULL );
			}

			p.width  = NH_patch;
			p.height = NV_patch;
			p.type   = PATCH_GENERIC;
			for ( i = 0; i < NH_patch; i++ )
			{
				switch ( Plane )
				{
				case PLANE_XY1:
				case PLANE_XZ0:
				case PLANE_YZ1:
					ii = i0 + NH_patch - 1 - i;
					break;
				default:
					ii = i0 + i;
				}
				for ( j = 0; j < NV_patch; j++ )
				{
					p.ctrl[i][j].xyz[0] = (float)xyz[ii][j0 + j].pp[0];
					p.ctrl[i][j].xyz[1] = (float)xyz[ii][j0 + j].pp[1];
					p.ctrl[i][j].xyz[2] = (float)xyz[ii][j0 + j].pp[2];
					p.ctrl[i][j].st[0]  = (float)i;
					p.ctrl[i][j].st[1]  = (float)j;
				}
			}
			MakePatch( &p );
			BrushNum++;
			j0 += NV_patch - 1;
		}
		i0 += NH_patch - 1;
	}
	return BrushNum;
}

//=============================================================
void MapBrushes(){
	char hint[128];
	char skip[128];
	char sidetext[64];
	char surftext[64];
	char surftext2[64];
	char surft[64];
	float Steep;
	vec3_t PlaneNormal,SurfNormal;
	vec3_t t[2];
	int i, j, k;
	int surf;
	bool CheckAngle;
	BRUSH brush;
	XYZ v[8];

	strcpy( surftext,Texture[Game][0] );
	strcpy( sidetext,( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
	strcpy( surftext2,( strlen( Texture[Game][2] ) ? Texture[Game][2] : Texture[Game][0] ) );

	// if surftext2 is identical to surftext, there's no need to
	// check surface angle
	if ( !g_ascii_strcasecmp( surftext,surftext2 ) ) {
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

	OpenFuncGroup();

	for ( i = 0; i < NH; i++ )
	{
		for ( j = 0; j < NV; j++ )
		{
			if ( ( i + j ) % 2 ) {
				VectorCopy( xyz[i  ][j  ].p, v[0].p );
				switch ( Plane )
				{
				case PLANE_XY1:
				case PLANE_XZ1:
				case PLANE_YZ1:
					VectorCopy( xyz[i + 1][j  ].p, v[1].p );
					VectorCopy( xyz[i + 1][j + 1].p, v[2].p );
					break;
				default:
					VectorCopy( xyz[i + 1][j + 1].p, v[1].p );
					VectorCopy( xyz[i + 1][j  ].p, v[2].p );
				}
			}
			else
			{
				VectorCopy( xyz[i  ][j  ].p, v[0].p );
				switch ( Plane )
				{
				case PLANE_XY1:
				case PLANE_XZ1:
				case PLANE_YZ1:
					VectorCopy( xyz[i + 1][j  ].p, v[1].p );
					VectorCopy( xyz[i  ][j + 1].p, v[2].p );
					break;
				default:
					VectorCopy( xyz[i  ][j + 1].p, v[1].p );
					VectorCopy( xyz[i + 1][j  ].p, v[2].p );
				}
			}
			VectorCopy( v[0].p,v[3].p );
			VectorCopy( v[1].p,v[4].p );
			VectorCopy( v[2].p,v[5].p );
			switch ( Plane )
			{
			case PLANE_XZ0:
			case PLANE_XZ1:
				v[0].p[1] = backface;
				v[1].p[1] = backface;
				v[2].p[1] = backface;
				break;
			case PLANE_YZ0:
			case PLANE_YZ1:
				v[3].p[0] = backface;
				v[4].p[0] = backface;
				v[5].p[0] = backface;
				break;
			default:
				v[3].p[2] = backface;
				v[4].p[2] = backface;
				v[5].p[2] = backface;
			}

			brush.Number   = i * NV * 2 + j * 2;
			brush.NumFaces = 5;
			XYZtoV( &v[0],&brush.face[0].v[0] );
			XYZtoV( &v[3],&brush.face[0].v[1] );
			XYZtoV( &v[4],&brush.face[0].v[2] );
			strcpy( brush.face[0].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[0].Shift[0] = (float)TexOffset[0];
			brush.face[0].Shift[1] = (float)TexOffset[1];
			brush.face[0].Rotate   = 0.;
			brush.face[0].Scale[0] = (float)TexScale[0];
			brush.face[0].Scale[1] = (float)TexScale[1];
			brush.face[0].Contents = contents;
			brush.face[0].Surface  = surface[1];
			brush.face[0].Value    = 0;

			XYZtoV( &v[1],&brush.face[1].v[0] );
			XYZtoV( &v[4],&brush.face[1].v[1] );
			XYZtoV( &v[5],&brush.face[1].v[2] );
			strcpy( brush.face[1].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[1].Shift[0] = (float)TexOffset[0];
			brush.face[1].Shift[1] = (float)TexOffset[1];
			brush.face[1].Rotate   = 0.;
			brush.face[1].Scale[0] = (float)TexScale[0];
			brush.face[1].Scale[1] = (float)TexScale[1];
			brush.face[1].Contents = contents;
			brush.face[1].Surface  = surface[1];
			brush.face[1].Value    = 0;

			XYZtoV( &v[2],&brush.face[2].v[0] );
			XYZtoV( &v[5],&brush.face[2].v[1] );
			XYZtoV( &v[3],&brush.face[2].v[2] );
			strcpy( brush.face[2].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[2].Shift[0] = (float)TexOffset[0];
			brush.face[2].Shift[1] = (float)TexOffset[1];
			brush.face[2].Rotate   = 0.;
			brush.face[2].Scale[0] = (float)TexScale[0];
			brush.face[2].Scale[1] = (float)TexScale[1];
			brush.face[2].Contents = contents;
			brush.face[2].Surface  = surface[1];
			brush.face[2].Value    = 0;

			if ( CheckAngle && ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ) ) {
				XYZVectorSubtract( v[4].p,v[3].p,t[0] );
				XYZVectorSubtract( v[5].p,v[4].p,t[1] );
				CrossProduct( t[0],t[1],SurfNormal );
				VectorNormalize( SurfNormal,SurfNormal );
				if ( DotProduct( SurfNormal,PlaneNormal ) < Steep ) {
					strcpy( surft,surftext2 );
					surf = surface[2];
				}
				else
				{
					strcpy( surft,surftext );
					surf = surface[0];
				}
			}
			else
			{
				strcpy( surft,surftext );
				surf = surface[0];
			}

			XYZtoV( &v[3],&brush.face[3].v[0] );
			XYZtoV( &v[5],&brush.face[3].v[1] );
			XYZtoV( &v[4],&brush.face[3].v[2] );
			strcpy( brush.face[3].texture,
					( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surft : sidetext ) );
			brush.face[3].Shift[0] = (float)TexOffset[0];
			brush.face[3].Shift[1] = (float)TexOffset[1];
			brush.face[3].Rotate   = 0.;
			brush.face[3].Scale[0] = (float)TexScale[0];
			brush.face[3].Scale[1] = (float)TexScale[1];
			brush.face[3].Contents = contents;
			brush.face[3].Surface  = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surf : surface[1] );
			brush.face[3].Value    = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? ArghRad2 : 0 );

			if ( CheckAngle && Plane != PLANE_XZ0 && Plane != PLANE_XZ1 ) {
				XYZVectorSubtract( v[2].p,v[0].p,t[0] );
				XYZVectorSubtract( v[1].p,v[2].p,t[1] );
				CrossProduct( t[0],t[1],SurfNormal );
				VectorNormalize( SurfNormal,SurfNormal );
				if ( DotProduct( SurfNormal,PlaneNormal ) < Steep ) {
					strcpy( surft,surftext2 );
					surf = surface[2];
				}
				else
				{
					strcpy( surft,surftext );
					surf = surface[0];
				}
			}
			else
			{
				strcpy( surft,surftext );
				surf = surface[0];
			}

			XYZtoV( &v[0],&brush.face[4].v[0] );
			XYZtoV( &v[1],&brush.face[4].v[1] );
			XYZtoV( &v[2],&brush.face[4].v[2] );
			strcpy( brush.face[4].texture,
					( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? sidetext : surft ) );
			brush.face[4].Shift[0] = (float)TexOffset[0];
			brush.face[4].Shift[1] = (float)TexOffset[1];
			brush.face[4].Rotate   = 0.;
			brush.face[4].Scale[0] = (float)TexScale[0];
			brush.face[4].Scale[1] = (float)TexScale[1];
			brush.face[4].Contents = contents;
			brush.face[4].Surface  = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surface[1] : surf );
			brush.face[4].Value    = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? 0 : ArghRad2 );

			MakeBrush( &brush );
			if ( ( i + j ) % 2 ) {
				VectorCopy( xyz[i  ][j + 1].p,v[0].p );
				switch ( Plane )
				{
				case PLANE_XY1:
				case PLANE_XZ1:
				case PLANE_YZ1:
					VectorCopy( xyz[i  ][j  ].p,v[1].p );
					VectorCopy( xyz[i + 1][j + 1].p,v[2].p );
					break;
				default:
					VectorCopy( xyz[i + 1][j + 1].p,v[1].p );
					VectorCopy( xyz[i  ][j  ].p,v[2].p );
				}
			}
			else
			{
				VectorCopy( xyz[i  ][j + 1].p,v[0].p );
				switch ( Plane )
				{
				case PLANE_XY1:
				case PLANE_XZ1:
				case PLANE_YZ1:
					VectorCopy( xyz[i + 1][j  ].p,v[1].p );
					VectorCopy( xyz[i + 1][j + 1].p,v[2].p );
					break;
				default:
					VectorCopy( xyz[i + 1][j + 1].p,v[1].p );
					VectorCopy( xyz[i + 1][j  ].p,v[2].p );
				}
			}
			VectorCopy( v[0].p,v[3].p );
			VectorCopy( v[1].p,v[4].p );
			VectorCopy( v[2].p,v[5].p );
			switch ( Plane )
			{
			case PLANE_XZ0:
			case PLANE_XZ1:
				v[0].p[1] = backface;
				v[1].p[1] = backface;
				v[2].p[1] = backface;
				break;
			case PLANE_YZ0:
			case PLANE_YZ1:
				v[3].p[0] = backface;
				v[4].p[0] = backface;
				v[5].p[0] = backface;
				break;
			default:
				v[3].p[2] = backface;
				v[4].p[2] = backface;
				v[5].p[2] = backface;
			}
			brush.Number   = i * NV * 2 + j * 2 + 1;
			brush.NumFaces = 5;
			XYZtoV( &v[0],&brush.face[0].v[0] );
			XYZtoV( &v[3],&brush.face[0].v[1] );
			XYZtoV( &v[4],&brush.face[0].v[2] );
			strcpy( brush.face[0].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[0].Shift[0] = (float)TexOffset[0];
			brush.face[0].Shift[1] = (float)TexOffset[1];
			brush.face[0].Rotate   = 0.;
			brush.face[0].Scale[0] = (float)TexScale[0];
			brush.face[0].Scale[1] = (float)TexScale[1];
			brush.face[0].Contents = contents;
			brush.face[0].Surface  = surface[1];
			brush.face[0].Value    = 0;

			XYZtoV( &v[1],&brush.face[1].v[0] );
			XYZtoV( &v[4],&brush.face[1].v[1] );
			XYZtoV( &v[5],&brush.face[1].v[2] );
			strcpy( brush.face[1].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[1].Shift[0] = (float)TexOffset[0];
			brush.face[1].Shift[1] = (float)TexOffset[1];
			brush.face[1].Rotate   = 0.;
			brush.face[1].Scale[0] = (float)TexScale[0];
			brush.face[1].Scale[1] = (float)TexScale[1];
			brush.face[1].Contents = contents;
			brush.face[1].Surface  = surface[1];
			brush.face[1].Value    = 0;

			XYZtoV( &v[2],&brush.face[2].v[0] );
			XYZtoV( &v[5],&brush.face[2].v[1] );
			XYZtoV( &v[3],&brush.face[2].v[2] );
			strcpy( brush.face[2].texture,
					( strlen( Texture[Game][1] ) ? Texture[Game][1] : Texture[Game][0] ) );
			brush.face[2].Shift[0] = (float)TexOffset[0];
			brush.face[2].Shift[1] = (float)TexOffset[1];
			brush.face[2].Rotate   = 0.;
			brush.face[2].Scale[0] = (float)TexScale[0];
			brush.face[2].Scale[1] = (float)TexScale[1];
			brush.face[2].Contents = contents;
			brush.face[2].Surface  = surface[1];
			brush.face[2].Value    = 0;

			if ( CheckAngle && ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ) ) {
				XYZVectorSubtract( v[4].p,v[3].p,t[0] );
				XYZVectorSubtract( v[5].p,v[4].p,t[1] );
				CrossProduct( t[0],t[1],SurfNormal );
				VectorNormalize( SurfNormal,SurfNormal );
				if ( DotProduct( SurfNormal,PlaneNormal ) < Steep ) {
					strcpy( surft,surftext2 );
					surf = surface[2];
				}
				else
				{
					strcpy( surft,surftext );
					surf = surface[0];
				}
			}
			else
			{
				strcpy( surft,surftext );
				surf = surface[0];
			}
			XYZtoV( &v[3],&brush.face[3].v[0] );
			XYZtoV( &v[5],&brush.face[3].v[1] );
			XYZtoV( &v[4],&brush.face[3].v[2] );
			strcpy( brush.face[3].texture,
					( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surft : sidetext ) );
			brush.face[3].Shift[0] = (float)TexOffset[0];
			brush.face[3].Shift[1] = (float)TexOffset[1];
			brush.face[3].Rotate   = 0.;
			brush.face[3].Scale[0] = (float)TexScale[0];
			brush.face[3].Scale[1] = (float)TexScale[1];
			brush.face[3].Contents = contents;
			brush.face[3].Surface  = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surf : surface[1] );
			brush.face[3].Value    = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? ArghRad2 : 0 );

			if ( CheckAngle && Plane != PLANE_XZ0 && Plane != PLANE_XZ1 ) {
				XYZVectorSubtract( v[2].p,v[0].p,t[0] );
				XYZVectorSubtract( v[1].p,v[2].p,t[1] );
				CrossProduct( t[0],t[1],SurfNormal );
				VectorNormalize( SurfNormal,SurfNormal );
				if ( DotProduct( SurfNormal,PlaneNormal ) < Steep ) {
					strcpy( surft,surftext2 );
					surf = surface[2];
				}
				else
				{
					strcpy( surft,surftext );
					surf = surface[0];
				}
			}
			else
			{
				strcpy( surft,surftext );
				surf = surface[0];
			}
			XYZtoV( &v[0],&brush.face[4].v[0] );
			XYZtoV( &v[1],&brush.face[4].v[1] );
			XYZtoV( &v[2],&brush.face[4].v[2] );
			strcpy( brush.face[4].texture,
					( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? sidetext : surft ) );
			brush.face[4].Shift[0] = (float)TexOffset[0];
			brush.face[4].Shift[1] = (float)TexOffset[1];
			brush.face[4].Rotate   = 0.;
			brush.face[4].Scale[0] = (float)TexScale[0];
			brush.face[4].Scale[1] = (float)TexScale[1];
			brush.face[4].Contents = contents;
			brush.face[4].Surface  = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? surface[1] : surf );
			brush.face[4].Value    = ( Plane == PLANE_XZ0 || Plane == PLANE_XZ1 ? 0 : ArghRad2 );

			MakeBrush( &brush );
		}
	}
	CloseFuncGroup();

	if ( AddHints || GimpHints ) {
		int detail, i1, j1, N;
		double front;

		switch ( Game )
		{
		case HALFLIFE:
			strcpy( hint,"HINT" );
			strcpy( skip,"HINT" );
			break;
		case SIN:
			strcpy( hint,"generic/misc/hint" );
			strcpy( skip,"generic/misc/skip" );
			break;
		case HERETIC2:
			strcpy( hint,"general/hint" );
			strcpy( skip,"general/hint" ); // Heretic2 doesn't have a skip texture
			break;
		case KINGPIN:
			strcpy( hint,"common/0_hint" );
			strcpy( skip,"common/0_skip" );
			break;
		case GENESIS3D:
			strcpy( hint,"hint" );
			strcpy( skip,"hint" );
			break;
		case QUAKE3:
			strcpy( hint,"textures/common/hint" );
			strcpy( skip,"textures/common/skip" );
			break;
		default:
			strcpy( hint,"e1u1/hint" );
			strcpy( skip,"e1u1/skip" );
		}

		OpenFuncGroup();

		if ( AddHints == 1 ) {
			detail = CONTENTS_DETAIL;
			N = 0;
			for ( i = 0; i < NH; i++ )
			{
				i1 = i + 1;

				for ( j = 0; j < NV; j++ )
				{

					// For detail hint brushes, no need to use a hint brush over
					// EVERY grid square... it would be redundant. Instead use
					// a checkerboard pattern
					if ( ( i + j ) % 2 ) {
						continue;
					}

					j1 = j + 1;

					VectorCopy( xyz[i  ][j  ].p, v[0].p );
					switch ( Plane )
					{
					case PLANE_XY1:
					case PLANE_XZ1:
					case PLANE_YZ1:
						VectorCopy( xyz[i1][j ].p, v[1].p );
						VectorCopy( xyz[i1][j1].p, v[2].p );
						VectorCopy( xyz[i ][j1].p, v[3].p );
						break;
					default:
						VectorCopy( xyz[i ][j1].p, v[1].p );
						VectorCopy( xyz[i1][j1].p, v[2].p );
						VectorCopy( xyz[i1][j ].p, v[3].p );
					}

					VectorCopy( v[0].p,v[4].p );
					VectorCopy( v[1].p,v[5].p );
					VectorCopy( v[2].p,v[6].p );
					VectorCopy( v[3].p,v[7].p );

					switch ( Plane )
					{
					case PLANE_XY1:
						front  = LessThan( zmin,32. );
						v[4].p[2] = backface;
						v[5].p[2] = backface;
						v[6].p[2] = backface;
						v[7].p[2] = backface;
						break;
					case PLANE_XZ0:
						front  = MoreThan( ymax,32. );
						v[0].p[1] = backface;
						v[1].p[1] = backface;
						v[2].p[1] = backface;
						v[3].p[1] = backface;
						break;
					case PLANE_XZ1:
						front  = LessThan( ymin,32. );
						v[0].p[1] = backface;
						v[1].p[1] = backface;
						v[2].p[1] = backface;
						v[3].p[1] = backface;
						break;
					case PLANE_YZ0:
						front  = MoreThan( xmax,32. );
						v[4].p[0] = backface;
						v[5].p[0] = backface;
						v[6].p[0] = backface;
						v[7].p[0] = backface;
						break;
					case PLANE_YZ1:
						front  = LessThan( xmin,32. );
						v[4].p[0] = backface;
						v[5].p[0] = backface;
						v[6].p[0] = backface;
						v[7].p[0] = backface;
						break;
					default:
						front  = MoreThan( zmax,32. );
						v[4].p[2] = backface;
						v[5].p[2] = backface;
						v[6].p[2] = backface;
						v[7].p[2] = backface;
					}

					switch ( Plane )
					{
					case PLANE_XZ0:
					case PLANE_XZ1:
						v[4].p[1] = front;
						v[5].p[1] = v[4].p[1];
						v[6].p[1] = v[4].p[1];
						v[7].p[1] = v[4].p[1];
						break;
					case PLANE_YZ0:
					case PLANE_YZ1:
						v[0].p[0] = front;
						v[1].p[0] = v[0].p[0];
						v[2].p[0] = v[0].p[0];
						v[3].p[0] = v[0].p[0];
						break;
					default:
						v[0].p[2] = front;
						v[1].p[2] = v[0].p[2];
						v[2].p[2] = v[0].p[2];
						v[3].p[2] = v[0].p[2];
					}

					brush.NumFaces = 6;
					brush.Number   = N;
					XYZtoV( &v[0],&brush.face[0].v[0] );
					XYZtoV( &v[1],&brush.face[0].v[1] );
					XYZtoV( &v[2],&brush.face[0].v[2] );
					strcpy( brush.face[0].texture,skip );
					brush.face[0].Shift[0] = 0.;
					brush.face[0].Shift[1] = 0.;
					brush.face[0].Rotate   = 0.;
					brush.face[0].Scale[0] = 1.;
					brush.face[0].Scale[1] = 1.;
					brush.face[0].Contents = detail;
					brush.face[0].Surface  = SURF_SKIP;
					brush.face[0].Value    = 0;

					XYZtoV( &v[4],&brush.face[1].v[0] );
					XYZtoV( &v[7],&brush.face[1].v[1] );
					XYZtoV( &v[6],&brush.face[1].v[2] );
					strcpy( brush.face[1].texture,skip );
					brush.face[1].Shift[0] = 0.;
					brush.face[1].Shift[1] = 0.;
					brush.face[1].Rotate   = 0.;
					brush.face[1].Scale[0] = 1.;
					brush.face[1].Scale[1] = 1.;
					brush.face[1].Contents = detail;
					brush.face[1].Surface  = SURF_SKIP;
					brush.face[1].Value    = 0;

					XYZtoV( &v[0],&brush.face[2].v[0] );
					XYZtoV( &v[4],&brush.face[2].v[1] );
					XYZtoV( &v[5],&brush.face[2].v[2] );
					strcpy( brush.face[2].texture,hint );
					brush.face[2].Shift[0] = 0.;
					brush.face[2].Shift[1] = 0.;
					brush.face[2].Rotate   = 0.;
					brush.face[2].Scale[0] = 1.;
					brush.face[2].Scale[1] = 1.;
					brush.face[2].Contents = detail;
					brush.face[2].Surface  = SURF_HINT;
					brush.face[2].Value    = 0;

					XYZtoV( &v[1],&brush.face[3].v[0] );
					XYZtoV( &v[5],&brush.face[3].v[1] );
					XYZtoV( &v[6],&brush.face[3].v[2] );
					strcpy( brush.face[3].texture,hint );
					brush.face[3].Shift[0] = 0.;
					brush.face[3].Shift[1] = 0.;
					brush.face[3].Rotate   = 0.;
					brush.face[3].Scale[0] = 1.;
					brush.face[3].Scale[1] = 1.;
					brush.face[3].Contents = detail;
					brush.face[3].Surface  = SURF_HINT;
					brush.face[3].Value    = 0;

					XYZtoV( &v[2],&brush.face[4].v[0] );
					XYZtoV( &v[6],&brush.face[4].v[1] );
					XYZtoV( &v[7],&brush.face[4].v[2] );
					strcpy( brush.face[4].texture,hint );
					brush.face[4].Shift[0] = 0.;
					brush.face[4].Shift[1] = 0.;
					brush.face[4].Rotate   = 0.;
					brush.face[4].Scale[0] = 1.;
					brush.face[4].Scale[1] = 1.;
					brush.face[4].Contents = detail;
					brush.face[4].Surface  = SURF_HINT;
					brush.face[4].Value    = 0;

					XYZtoV( &v[3],&brush.face[5].v[0] );
					XYZtoV( &v[7],&brush.face[5].v[1] );
					XYZtoV( &v[4],&brush.face[5].v[2] );
					strcpy( brush.face[5].texture,hint );
					brush.face[5].Shift[0] = 0.;
					brush.face[5].Shift[1] = 0.;
					brush.face[5].Rotate   = 0.;
					brush.face[5].Scale[0] = 1.;
					brush.face[5].Scale[1] = 1.;
					brush.face[5].Contents = detail;
					brush.face[5].Surface  = SURF_HINT;
					brush.face[5].Value    = 0;

					MakeBrush( &brush );
					N++;
				}
			}
		}
		if ( GimpHints ) {
			N = 0;
			// these brush parameters never change
			brush.NumFaces = 5;
			for ( i = 0; i < 6; i++ )
			{
				strcpy( brush.face[i].texture,hint );
				brush.face[i].Shift[0] = 0.;
				brush.face[i].Shift[1] = 0.;
				brush.face[i].Rotate   = 0.;
				brush.face[i].Scale[0] = 1.;
				brush.face[i].Scale[1] = 1.;
				brush.face[i].Contents = 0;
				brush.face[i].Surface  = SURF_HINT;
				brush.face[i].Value    = 0;
			}
			for ( i = 0; i < NH; i++ )
			{
				for ( j = 0; j < NV; j++ )
				{
					for ( k = 0; k < 2; k++ )
					{
						if ( k == 0 ) {
							if ( ( i + j ) % 2 ) {
								VectorCopy( xyz[i  ][j  ].p, v[0].p );
								switch ( Plane )
								{
								case PLANE_XY1:
								case PLANE_XZ1:
								case PLANE_YZ1:
									VectorCopy( xyz[i + 1][j  ].p, v[1].p );
									VectorCopy( xyz[i + 1][j + 1].p, v[2].p );
									break;
								default:
									VectorCopy( xyz[i + 1][j + 1].p, v[1].p );
									VectorCopy( xyz[i + 1][j  ].p, v[2].p );
								}
							}
							else
							{
								VectorCopy( xyz[i  ][j  ].p, v[0].p );
								switch ( Plane )
								{
								case PLANE_XY1:
								case PLANE_XZ1:
								case PLANE_YZ1:
									VectorCopy( xyz[i + 1][j  ].p, v[1].p );
									VectorCopy( xyz[i  ][j + 1].p, v[2].p );
									break;
								default:
									VectorCopy( xyz[i  ][j + 1].p, v[1].p );
									VectorCopy( xyz[i + 1][j  ].p, v[2].p );
								}
							}
						}
						else
						{
							if ( ( i + j ) % 2 ) {
								VectorCopy( xyz[i  ][j + 1].p,v[0].p );
								switch ( Plane )
								{
								case PLANE_XY1:
								case PLANE_XZ1:
								case PLANE_YZ1:
									VectorCopy( xyz[i  ][j  ].p,v[1].p );
									VectorCopy( xyz[i + 1][j + 1].p,v[2].p );
									break;
								default:
									VectorCopy( xyz[i + 1][j + 1].p,v[1].p );
									VectorCopy( xyz[i  ][j  ].p,v[2].p );
								}
							}
							else
							{
								VectorCopy( xyz[i  ][j + 1].p,v[0].p );
								switch ( Plane )
								{
								case PLANE_XY1:
								case PLANE_XZ1:
								case PLANE_YZ1:
									VectorCopy( xyz[i + 1][j  ].p,v[1].p );
									VectorCopy( xyz[i + 1][j + 1].p,v[2].p );
									break;
								default:
									VectorCopy( xyz[i + 1][j + 1].p,v[1].p );
									VectorCopy( xyz[i + 1][j  ].p,v[2].p );
								}
							}
						}
						VectorCopy( v[0].p,v[3].p );
						VectorCopy( v[1].p,v[4].p );
						VectorCopy( v[2].p,v[5].p );
						switch ( Plane )
						{
						case PLANE_XY0:
							v[0].p[2] += HINT_OFFSET;
							v[1].p[2] += HINT_OFFSET;
							v[2].p[2] += HINT_OFFSET;
//              v[3].p[2] = backface;
//              v[4].p[2] = backface;
//              v[5].p[2] = backface;
							break;
						case PLANE_XY1:
							v[0].p[2] -= HINT_OFFSET;
							v[1].p[2] -= HINT_OFFSET;
							v[2].p[2] -= HINT_OFFSET;
//              v[3].p[2] = backface;
//              v[4].p[2] = backface;
//              v[5].p[2] = backface;
							break;
						case PLANE_XZ0:
//              v[0].p[1] = backface;
//              v[1].p[1] = backface;
//              v[2].p[1] = backface;
							v[3].p[1] += HINT_OFFSET;
							v[4].p[1] += HINT_OFFSET;
							v[5].p[1] += HINT_OFFSET;
							break;
						case PLANE_XZ1:
//              v[0].p[1] = backface;
//              v[1].p[1] = backface;
//              v[2].p[1] = backface;
							v[3].p[1] -= HINT_OFFSET;
							v[4].p[1] -= HINT_OFFSET;
							v[5].p[1] -= HINT_OFFSET;
							break;
						case PLANE_YZ0:
							v[0].p[0] += HINT_OFFSET;
							v[1].p[0] += HINT_OFFSET;
							v[2].p[0] += HINT_OFFSET;
//              v[3].p[0] = backface;
//              v[4].p[0] = backface;
//              v[5].p[0] = backface;
							break;
						case PLANE_YZ1:
							v[0].p[0] -= HINT_OFFSET;
							v[1].p[0] -= HINT_OFFSET;
							v[2].p[0] -= HINT_OFFSET;
//              v[3].p[0] = backface;
//              v[4].p[0] = backface;
//              v[5].p[0] = backface;
							break;
						}
						brush.Number   = N;
						XYZtoV( &v[0],&brush.face[0].v[0] );
						XYZtoV( &v[3],&brush.face[0].v[1] );
						XYZtoV( &v[4],&brush.face[0].v[2] );

						XYZtoV( &v[1],&brush.face[1].v[0] );
						XYZtoV( &v[4],&brush.face[1].v[1] );
						XYZtoV( &v[5],&brush.face[1].v[2] );

						XYZtoV( &v[2],&brush.face[2].v[0] );
						XYZtoV( &v[5],&brush.face[2].v[1] );
						XYZtoV( &v[3],&brush.face[2].v[2] );

						XYZtoV( &v[3],&brush.face[3].v[0] );
						XYZtoV( &v[5],&brush.face[3].v[1] );
						XYZtoV( &v[4],&brush.face[3].v[2] );

						XYZtoV( &v[0],&brush.face[4].v[0] );
						XYZtoV( &v[1],&brush.face[4].v[1] );
						XYZtoV( &v[2],&brush.face[4].v[2] );

						MakeBrush( &brush );
						N++;
					}
				}
			}
		} // endif AddHints==1
		CloseFuncGroup();
	}

} // end MapBrushes

//=============================================================
void GenerateMap(){
	extern void MapOut( int,int,NODE *,TRI * );
	extern bool SingleBrushSelected;
	int ntri;

	if ( !ValidSurface() ) {
		return;
	}
	/*
	   ghCursorCurrent = LoadCursor(NULL,IDC_WAIT);
	   SetCursor(ghCursorCurrent);
	 */
	if ( SingleBrushSelected ) {
		g_FuncTable.m_pfnDeleteSelection();
	}

	GenerateXYZ();
	ntri = NH * NV * 2;

	if ( Game == QUAKE3 && UsePatches != 0 ) {
		MapPatches();
	}

	if ( Decimate > 0 && ( Game != QUAKE3 || UsePatches == 0 ) ) {
		MapOut( gNumNodes,gNumTris,gNode,gTri );
		/*
		   ghCursorCurrent = ghCursorDefault;
		   SetCursor(ghCursorCurrent);
		 */
		return;
	}

	contents = 0;
	// HL doesn't have detail property
	if ( ( Game != HALFLIFE ) && UseDetail ) {
		contents += CONTENTS_DETAIL;
	}
	// HL and Q3 don't have ladder property
	if ( ( Game != HALFLIFE && Game != QUAKE3 ) && UseLadder ) {
		contents += CONTENTS_LADDER;
	}
	// Genesis requires solid property to be set explicitly
	if ( Game == GENESIS3D ) {
		contents |= CONTENTS_SOLID;
	}
	// Heretic 2 uses different sounds (in surface props) for different texture types
	if ( Game == HERETIC2 ) {
		surface[0] = GetDefSurfaceProps( Texture[Game][0] );
		surface[1] = GetDefSurfaceProps( Texture[Game][1] );
		surface[2] = GetDefSurfaceProps( Texture[Game][2] );
	}
	else
	{
		surface[0] = 0;
		surface[1] = 0;
		surface[2] = 0;
	}
	if ( Game != QUAKE3 || UsePatches == 0 ) {
		MapBrushes();
	}

	/*
	   ghCursorCurrent = ghCursorDefault;
	   SetCursor(ghCursorCurrent);
	 */
}

//=============================================================
void GenerateXYZ(){
	extern void MakeDecimatedMap( int *, int *, NODE * *, TRI * * );
	double zl, zu;
	double wh, wv;
	int NHalfcycles;
	double a,v,h,ha,va;
	double delta, dr, rate;
	double range, maxrange;
	double r;
	int i, j, k, N;
	int i0, i1, j0, j1;
	int ii, jj;

//  FILE *f;
//  char CSV[64];

	if ( !ValidSurface() ) {
		return;
	}

	srand( 1 );
	srand( RandomSeed );

	dh = ( Hur - Hll ) / NH;
	dv = ( Vur - Vll ) / NV;

	// H & V
	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			switch ( Plane )
			{
			case PLANE_XZ0:
			case PLANE_XZ1:
				xyz[i][j].p[0] = Hll + i * dh;
				xyz[i][j].p[2] = Vll + j * dv;
				break;
			case PLANE_YZ0:
			case PLANE_YZ1:
				xyz[i][j].p[1] = Hll + i * dh;
				xyz[i][j].p[2] = Vll + j * dv;
				break;
			default:
				xyz[i][j].p[0] = Hll + i * dh;
				xyz[i][j].p[1] = Vll + j * dv;
			}
		}
	}

	if ( WaveType == WAVE_BITMAP ) {
		GenerateBitmapMapping();
	}
	/*
	   else if(WaveType == WAVE_FORMULA)
	   DoFormula();
	 */
	else
	{
		// Initialize Z values using bilinear interpolation
		for ( i = 0; i <= NH; i++ )
		{
			zl = Z00 + i * ( Z10 - Z00 ) / NH;
			zu = Z01 + i * ( Z11 - Z01 ) / NH;
			switch ( Plane )
			{
			case PLANE_XZ0:
			case PLANE_XZ1:
				for ( j = 0; j <= NV; j++ )
					xyz[i][j].p[1] = zl + j * ( zu - zl ) / NV;
				break;
			case PLANE_YZ0:
			case PLANE_YZ1:
				for ( j = 0; j <= NV; j++ )
					xyz[i][j].p[0] = zl + j * ( zu - zl ) / NV;
				break;
			default:
				for ( j = 0; j <= NV; j++ )
					xyz[i][j].p[2] = zl + j * ( zu - zl ) / NV;
			}
		}
	}

	switch ( WaveType )
	{
	case WAVE_COS_SIN:
		if ( FixBorders ) {
			NHalfcycles = (int)( ( Hur - Hll ) / ( WaveLength / 2. ) );
			NHalfcycles = max( NHalfcycles,1 );
			wh = 2. * ( Hur - Hll ) / NHalfcycles;
			NHalfcycles = (int)( ( Vur - Vll ) / ( WaveLength / 2. ) );
			wv = 2. * ( Vur - Vll ) / NHalfcycles;
			NHalfcycles = max( NHalfcycles,1 );
			i0 = 1;
			i1 = NH - 1;
			j0 = 1;
			j1 = NV - 1;
		}
		else
		{
			wh = WaveLength;
			wv = WaveLength;
			i0 = 0;
			i1 = NH;
			j0 = 0;
			j1 = NV;
		}

		for ( i = i0; i <= i1; i++ )
		{
			h  = Hll + i * dh;
			ha = ( ( h - Hll ) / wh ) * 2. * PI - PI / 2.;
			for ( j = j0; j <= j1; j++ )
			{
				v  = Vll + j * dv;
				va = ( ( v - Vll ) / wv ) * 2. * PI;
				a = Amplitude * cos( ha ) * sin( va );
				switch ( Plane )
				{
				case PLANE_XY1:
					xyz[i][j].p[2] -= a;
					break;
				case PLANE_XZ0:
					xyz[i][j].p[1] += a;
					break;
				case PLANE_XZ1:
					xyz[i][j].p[1] -= a;
					break;
				case PLANE_YZ0:
					xyz[i][j].p[0] += a;
					break;
				case PLANE_YZ1:
					xyz[i][j].p[0] -= a;
					break;
				default:
					xyz[i][j].p[2] += a;
				}
			}
		}
		break;
	case WAVE_HCYLINDER:
		for ( i = 0; i <= NH; i++ )
		{
			h  = Hll + i * dh;
			ha = ( ( h - Hll ) / WaveLength ) * 2. * PI - PI / 2.;
			for ( j = 0; j <= NV; j++ )
			{
				a = Amplitude * cos( ha );
				switch ( Plane )
				{
				case PLANE_XY1:
					xyz[i][j].p[2] -= a;
					break;
				case PLANE_XZ0:
					xyz[i][j].p[1] += a;
					break;
				case PLANE_XZ1:
					xyz[i][j].p[1] -= a;
					break;
				case PLANE_YZ0:
					xyz[i][j].p[0] += a;
					break;
				case PLANE_YZ1:
					xyz[i][j].p[0] -= a;
					break;
				default:
					xyz[i][j].p[2] += a;
				}
			}
		}
		break;
	case WAVE_VCYLINDER:
		for ( i = 0; i <= NH; i++ )
		{
			h  = Hll + i * dh;
			for ( j = 0; j <= NV; j++ )
			{
				v  = Vll + j * dv;
				va = ( ( v - Vll ) / WaveLength ) * 2. * PI;
				a = Amplitude * sin( va );
				switch ( Plane )
				{
				case PLANE_XY1:
					xyz[i][j].p[2] -= a;
					break;
				case PLANE_XZ0:
					xyz[i][j].p[1] += a;
					break;
				case PLANE_XZ1:
					xyz[i][j].p[1] -= a;
					break;
				case PLANE_YZ0:
					xyz[i][j].p[0] += a;
					break;
				case PLANE_YZ1:
					xyz[i][j].p[0] -= a;
					break;
				default:
					xyz[i][j].p[2] += a;
				}
			}
		}
		break;
	case WAVE_ROUGH_ONLY:
		PlasmaCloud();
		break;
	}

	if ( WaveType != WAVE_ROUGH_ONLY ) {
		// Fixed values
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( xyz[i][j].fixed ) {
					switch ( Plane )
					{
					case PLANE_XZ0:
					case PLANE_XZ1:
						xyz[i][j].p[1] = xyz[i][j].fixed_value;
						break;
					case PLANE_YZ0:
					case PLANE_YZ1:
						xyz[i][j].p[0] = xyz[i][j].fixed_value;
						break;
					default:
						xyz[i][j].p[2] = xyz[i][j].fixed_value;
					}

					if ( xyz[i][j].range > 0 ) {
						maxrange = pow( xyz[i][j].range,2 ); // so we don't have to do sqrt's
						i0 = i - (int)( floor( xyz[i][j].range / dh - 0.5 ) + 1 );
						i1 = i + i - i0;
						j0 = j - (int)( floor( xyz[i][j].range / dv - 0.5 ) + 1 );
						j1 = j + j - j0;
						if ( FixBorders ) {
							i0 = max( i0,1 );
							i1 = min( i1,NH - 1 );
							j0 = max( j0,1 );
							j1 = min( j1,NV - 1 );
						}
						else
						{
							i0 = max( i0,0 );
							i1 = min( i1,NH );
							j0 = max( j0,0 );
							j1 = min( j1,NV );
						}
						for ( ii = i0; ii <= i1; ii++ )
						{
							for ( jj = j0; jj <= j1; jj++ )
							{
								if ( ii == i && jj == j ) {
									continue;
								}
								range = pow( dh * ( i - ii ), 2 ) + pow( dv * ( j - jj ), 2 );
								if ( range > maxrange ) {
									continue;
								}
								dr = sqrt( range / maxrange );
								rate = max( -30.,min( xyz[i][j].rate,30. ) );
								if ( rate < -1. ) {
									delta = pow( ( 1. - dr ),-rate + 1. );
								}
								else if ( rate < 0. ) {
									delta = ( 1 + rate ) * 0.5 * ( cos( dr * PI ) + 1.0 ) -
											rate*pow( ( 1. - dr ),2 );
								}
								else if ( rate == 0. ) {
									delta = 0.5 * ( cos( dr * PI ) + 1.0 );
								}
								else if ( rate <= 1. ) {
									delta = ( 1. - rate ) * 0.5 * ( cos( dr * PI ) + 1.0 ) +
											rate * ( 1. - pow( dr,2 ) );
								}
								else
								{
									delta = 1. - pow( dr,rate + 1 );
								}
								switch ( Plane )
								{
								case PLANE_XZ0:
								case PLANE_XZ1:
									xyz[ii][jj].p[1] += ( xyz[i][j].p[1] - xyz[ii][jj].p[1] ) * delta;
									break;
								case PLANE_YZ0:
								case PLANE_YZ1:
									xyz[ii][jj].p[0] += ( xyz[i][j].p[0] - xyz[ii][jj].p[0] ) * delta;
									break;
								default:
									xyz[ii][jj].p[2] += ( xyz[i][j].p[2] - xyz[ii][jj].p[2] ) * delta;
								}
							}
						}
					}
				}
			}
		}
	}

	if ( ( Roughness > 0. ) && ( WaveType != WAVE_ROUGH_ONLY ) ) {
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( CanEdit( i,j ) && !xyz[i][j].fixed ) {
					switch ( Plane )
					{
					case PLANE_XZ0:
					case PLANE_XZ1:
						xyz[i][j].p[1] += -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX );
						break;
					case PLANE_YZ0:
					case PLANE_YZ1:
						xyz[i][j].p[0] += -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX );
						break;
					default:
						xyz[i][j].p[2] += -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX );
					}
				}
				else{
					r = rand(); // We still get a random number, so that fixing points
				}
				// doesn't change the sequence.

			}
		}
	}

	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			for ( k = 0; k < 3; k++ )
			{
				xyz[i][j].p[k] = Nearest( xyz[i][j].p[k],2.0 );
			}
		}
	}

	// Find minima and maxima
	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		xmin = Hll;
		xmax = Hur;
		zmin = Vll;
		zmax = Vur;
		ymin = xyz[0][0].p[1];
		ymax = ymin;
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				ymin = min( ymin,xyz[i][j].p[1] );
				ymax = max( ymax,xyz[i][j].p[1] );
			}
		}
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		ymin = Hll;
		ymax = Hur;
		zmin = Vll;
		zmax = Vur;
		xmin = xyz[0][0].p[0];
		xmax = ymin;
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				xmin = min( xmin,xyz[i][j].p[0] );
				xmax = max( xmax,xyz[i][j].p[0] );
			}
		}
		break;
		break;
	default:
		xmin = Hll;
		xmax = Hur;
		ymin = Vll;
		ymax = Vur;
		zmin = xyz[0][0].p[2];
		zmax = zmin;
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				zmin = min( zmin,xyz[i][j].p[2] );
				zmax = max( zmax,xyz[i][j].p[2] );
			}
		}
	}

	xmin = Nearest( xmin,2. );
	xmax = Nearest( xmax,2. );
	ymin = Nearest( ymin,2. );
	ymax = Nearest( ymax,2. );
	zmin = Nearest( zmin,2. );
	zmax = Nearest( zmax,2. );

	switch ( Plane )
	{
	case PLANE_XY1:
		backface = AtLeast( zmax + 32.,32. );
		break;
	case PLANE_XZ0:
		backface = NoMoreThan( ymin - 32.,32. );
		break;
	case PLANE_XZ1:
		backface = AtLeast( ymax + 32.,32. );
		break;
	case PLANE_YZ0:
		backface = NoMoreThan( xmin - 32.,32. );
		break;
	case PLANE_YZ1:
		backface = AtLeast( xmax + 32.,32. );
		break;
	default:
		backface = NoMoreThan( zmin - 32.,32. );
	}

	if ( gNode ) {
		free( gNode );
		free( gTri );
		gNode = (NODE *)NULL;
		gTri  = (TRI *)NULL;
	}
	if ( Decimate > 0 && ( Game != QUAKE3 || UsePatches == 0 ) ) {
		MakeDecimatedMap( &gNumNodes,&gNumTris,&gNode,&gTri );
	}
	else
	{
		gNumNodes = ( NH + 1 ) * ( NV + 1 );
		gNumTris  = NH * NV * 2;
		gNode = (NODE *) malloc( gNumNodes * sizeof( NODE ) );
		gTri = (TRI *) malloc( gNumTris * sizeof( TRI ) );

		for ( i = 0,N = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++, N++ )
			{
				gNode[N].used = 1;
				gNode[N].p[0] = (float)xyz[i][j].p[0];
				gNode[N].p[1] = (float)xyz[i][j].p[1];
				gNode[N].p[2] = (float)xyz[i][j].p[2];
			}
		}

		for ( i = 0; i < NH; i++ )
		{
			for ( j = 0; j < NV; j++ )
			{
				k = i * NV * 2 + j * 2;
				if ( ( i + j ) % 2 ) {
					switch ( Plane )
					{
					case PLANE_XY1:
					case PLANE_XZ1:
					case PLANE_YZ1:
						gTri[k  ].v[0] = i * ( NV + 1 ) + j;
						gTri[k  ].v[1] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						gTri[k  ].v[2] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k + 1].v[0] = i * ( NV + 1 ) + j;
						gTri[k + 1].v[1] = i * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[2] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						break;
					default:
						gTri[k  ].v[0] = i * ( NV + 1 ) + j;
						gTri[k  ].v[1] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k  ].v[2] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[0] = i * ( NV + 1 ) + j;
						gTri[k + 1].v[1] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[2] = i * ( NV + 1 ) + j + 1;
					}
				}
				else
				{
					switch ( Plane )
					{
					case PLANE_XY1:
					case PLANE_XZ1:
					case PLANE_YZ1:
						gTri[k  ].v[0] = i * ( NV + 1 ) + j;
						gTri[k  ].v[1] = i * ( NV + 1 ) + j + 1;
						gTri[k  ].v[2] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k + 1].v[0] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k + 1].v[1] = i * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[2] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						break;
					default:
						gTri[k  ].v[0] = i * ( NV + 1 ) + j;
						gTri[k  ].v[1] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k  ].v[2] = i * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[0] = ( i + 1 ) * ( NV + 1 ) + j;
						gTri[k + 1].v[1] = ( i + 1 ) * ( NV + 1 ) + j + 1;
						gTri[k + 1].v[2] = i * ( NV + 1 ) + j + 1;
					}
				}
			}
		}
	}
/*
   sprintf(CSV,"csv%03d.csv",Decimate);
   f = fopen(CSV,"w");
   for(i=0; i<gNumNodes; i++)
   {
    if(gNode[i].used)
      fprintf(f,"%g,%g,%g\n",gNode[i].p[0],gNode[i].p[1],gNode[i].p[2]);
   }
   fclose(f);
 */
	for ( i = 0; i < gNumTris; i++ )
		PlaneFromPoints( gNode[gTri[i].v[0]].p,
						 gNode[gTri[i].v[1]].p,
						 gNode[gTri[i].v[2]].p,
						 &gTri[i].plane );

	// Hydra: snap-to-grid begin
	if ( SnapToGrid > 0 ) {
		for ( i = 0; i < NH; i++ )
		{
			for ( j = 0; j < NV; j++ )
			{
				switch ( Plane )
				{
				case PLANE_XZ0:
				case PLANE_XZ1:
					xyz[i][j].p[1] = CalculateSnapValue( xyz[i][j].p[1] );
					break;
				case PLANE_YZ0:
				case PLANE_YZ1:
					xyz[i][j].p[0] = CalculateSnapValue( xyz[i][j].p[0] );
					break;
				default:
					xyz[i][j].p[2] = CalculateSnapValue( xyz[i][j].p[2] );
				}
			}
		}
	}
	// Hydra: snap-to-grid end
}
//=============================================================
double Nearest( double x, double dx ){
	double xx;

	xx = (double)( floor( x / dx - 0.5 ) + 1. ) * dx;
	if ( fabs( xx ) < dx / 2 ) {
		xx = 0.;
	}
	return xx;
}
//=============================================================
double NoMoreThan( double x, double dx ){
	double xx;

	xx = (double)( floor( x / dx - 0.5 ) + 1. ) * dx;
	if ( xx > x ) {
		xx -= dx;
	}
	return xx;
}
//=============================================================
double AtLeast( double x, double dx ){
	double xx;

	xx = (double)( floor( x / dx - 0.5 ) + 1. ) * dx;
	if ( xx < x ) {
		xx += dx;
	}
	return xx;
}
//=============================================================
double LessThan( double x,double dx ){
	double xx;

	xx = (double)( floor( x / dx - 0.5 ) + 1. ) * dx;
	if ( xx >= x ) {
		xx -= dx;
	}
	return xx;
}
//=============================================================
double MoreThan( double x,double dx ){
	double xx;

	xx = (double)( floor( x / dx - 0.5 ) + 1. ) * dx;
	while ( xx <= x )
		xx += dx;
	return xx;
}
//=============================================================
void SubdividePlasma( int i0,int j0,int i1,int j1 ){
	int i, j;
	double z1, z2;
	double r;    // NOTE: This is used to keep the random number sequence the same
	             //       when we fix a point. If we did NOT do this, then simply
	             //       fixing a point at its current value would change the entire
	             //       surface.

	i = ( i0 + i1 ) / 2;
	j = ( j0 + j1 ) / 2;
	if ( i1 > i0 + 1 ) {
		if ( !xyz[i][j0].done ) {
			xyz[i][j0].pp[2] = xyz[i0][j0].pp[2] +
							   ( xyz[i1][j0].pp[2] - xyz[i0][j0].pp[2] ) * (double)( i - i0 ) / (double)( i1 - i0 ) +
							   ( (double)( i - i0 ) ) * ( -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX ) );
			xyz[i][j0].done = 1;
		}
		else{
			r = rand();
		}
		if ( ( j1 > j0 ) && ( !xyz[i][j1].done ) ) {
			xyz[i][j1].pp[2] = xyz[i0][j1].pp[2] +
							   ( xyz[i1][j1].pp[2] - xyz[i0][j1].pp[2] ) * (double)( i - i0 ) / (double)( i1 - i0 ) +
							   ( (double)( i - i0 ) ) * ( -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX ) );
			xyz[i][j1].done = 1;
		}
		else{
			r = rand();
		}
	}
	if ( j1 > j0 + 1 ) {
		if ( !xyz[i0][j].done ) {
			xyz[i0][j].pp[2] = xyz[i0][j0].pp[2] +
							   ( xyz[i0][j1].pp[2] - xyz[i0][j0].pp[2] ) * (double)( j - j0 ) / (double)( j1 - j0 ) +
							   ( (double)( j - j0 ) ) * ( -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX ) );
			xyz[i0][j].done = 1;
		}
		else{
			r = rand();
		}
		if ( ( i1 > i0 ) && ( !xyz[i1][j].done ) ) {
			xyz[i1][j].pp[2] = xyz[i1][j0].pp[2] +
							   ( xyz[i1][j1].pp[2] - xyz[i1][j0].pp[2] ) * (double)( j - j0 ) / (double)( j1 - j0 ) +
							   ( (double)( j - j0 ) ) * ( -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX ) );
			xyz[i1][j].done = 1;
		}
		else{
			r = rand();
		}
	}
	if ( ( i1 > i0 + 1 ) && ( j1 > j0 + 1 ) ) {
		if ( !xyz[i][j].done ) {
			z1 = xyz[i0][j].pp[2] +
				 ( xyz[i1][j].pp[2] - xyz[i0][j].pp[2] ) * (double)( i - i0 ) / (double)( i1 - i0 );
			z2 = xyz[i][j0].pp[2] +
				 ( xyz[i][j1].pp[2] - xyz[i][j0].pp[2] ) * (double)( j - j0 ) / (double)( j1 - j0 );
			xyz[i][j].pp[2] = ( z1 + z2 ) / 2. +
							  ( (double)( i - i0 ) ) * ( -Roughness / 2. + Roughness * ( (double)rand() / (double)RAND_MAX ) );
			xyz[i][j].done = 1;
		}
		else{
			r = rand();
		}
	}
	if ( i > i0 + 1 || j > j0 + 1 ) {
		SubdividePlasma( i0,j0,i,j );
	}
	if ( i1 > i + 1 || j > j0 + 1 ) {
		SubdividePlasma( i,j0,i1,j );
	}
	if ( i > i0 + 1 || j1 > j0 + 1 ) {
		SubdividePlasma( i0,j,i,j1 );
	}
	if ( i1 > i + 1 || j1 > j0 + 1 ) {
		SubdividePlasma( i,j,i1,j1 );
	}
}
//==================================================================================
void PlasmaCloud(){
	int i, j;
	/* use pp[2] values until done to avoid messing with a bunch of
	   switch statements */

	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			if ( FixedPoint( i,j ) ) {
				xyz[i][j].done = 1;
			}
			else{
				xyz[i][j].done = 0;
			}
		}
	}

	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( xyz[i][j].fixed ) {
					xyz[i][j].pp[2] = xyz[i][j].fixed_value;
				}
				else{
					xyz[i][j].pp[2] = xyz[i][j].p[1];
				}
			}
		}
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( xyz[i][j].fixed ) {
					xyz[i][j].pp[2] = xyz[i][j].fixed_value;
				}
				else{
					xyz[i][j].pp[2] = xyz[i][j].p[0];
				}
			}
		}
		break;
	default:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( xyz[i][j].fixed ) {
					xyz[i][j].pp[2] = xyz[i][j].fixed_value;
				}
				else{
					xyz[i][j].pp[2] = xyz[i][j].p[2];
				}
			}
		}
		break;
	}
	SubdividePlasma( 0,0,NH,NV );
	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				xyz[i][j].p[1] = xyz[i][j].pp[2];
			}
		}
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				xyz[i][j].p[0] = xyz[i][j].pp[2];
			}
		}
		break;
	default:
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				xyz[i][j].p[2] = xyz[i][j].pp[2];
			}
		}
		break;
	}
}
//===========================================================================
bool FixedPoint( int i, int j ){
	if ( xyz[i][j].fixed ) {
		return TRUE;
	}
	return !CanEdit( i,j );
}
//===========================================================================
bool CanEdit( int i, int j ){
	if ( FixBorders && ( ( WaveType == WAVE_COS_SIN ) || ( WaveType == WAVE_ROUGH_ONLY ) ) ) {
		if ( i == 0 ) {
			return FALSE;
		}
		if ( i == NH ) {
			return FALSE;
		}
		if ( j == 0 ) {
			return FALSE;
		}
		if ( j == NV ) {
			return FALSE;
		}
	}
	if ( i == 0 && j == 0 ) {
		return FALSE;
	}
	if ( i == NH && j == 0 ) {
		return FALSE;
	}
	if ( i == 0 && j == NV ) {
		return FALSE;
	}
	if ( i == NH && j == NV ) {
		return FALSE;
	}
	return TRUE;
}
/*============================================================================
   TriangleFromPoint
   Determines which triangle in the gTri array bounds the input point. Doesn't
   do anything special with border points.
 */
int TriangleFromPoint( double x, double y ){
	int j, tri;

	if ( !gTri ) {
		return -1;
	}

	for ( j = 0, tri = -1; j < gNumTris && tri == -1; j++ )
	{
		if ( side( x,y,
				   gNode[gTri[j].v[0]].p[0],gNode[gTri[j].v[0]].p[1],
				   gNode[gTri[j].v[1]].p[0],gNode[gTri[j].v[1]].p[1] ) < 0. ) {
			continue;
		}
		if ( side( x,y,
				   gNode[gTri[j].v[1]].p[0],gNode[gTri[j].v[1]].p[1],
				   gNode[gTri[j].v[2]].p[0],gNode[gTri[j].v[2]].p[1] ) < 0. ) {
			continue;
		}
		if ( side( x,y,
				   gNode[gTri[j].v[2]].p[0],gNode[gTri[j].v[2]].p[1],
				   gNode[gTri[j].v[0]].p[0],gNode[gTri[j].v[0]].p[1] ) < 0. ) {
			continue;
		}
		tri = j;
	}

	return tri;
}
/*============================================================================
   PlayerStartZ
   Determines minimum height to place the player start such that he doesn't
   intersect any surface brushes.
 */
int PlayerStartZ( double x, double y ){
	int k,t[5];
	double z, zt;

	if ( !gTri ) {
		return (int)zmax;
	}

	t[0] = TriangleFromPoint( x,y );
	t[1] = TriangleFromPoint( x + PlayerBox[Game].x[0],y + PlayerBox[Game].y[0] );
	t[2] = TriangleFromPoint( x + PlayerBox[Game].x[0],y + PlayerBox[Game].y[1] );
	t[3] = TriangleFromPoint( x + PlayerBox[Game].x[1],y + PlayerBox[Game].y[0] );
	t[4] = TriangleFromPoint( x + PlayerBox[Game].x[1],y + PlayerBox[Game].y[1] );
	z = zmin;
	for ( k = 0; k < 5; k++ )
	{
		zt = ( gTri[t[k]].plane.dist -
			   gTri[t[k]].plane.normal[0] * x -
			   gTri[t[k]].plane.normal[1] * y   ) /
			 gTri[t[k]].plane.normal[2];
		z = max( z,zt );
	}
	return (int)( AtLeast( z,2. ) - PlayerBox[Game].z[0] );
}
//=============================================================
void XYZtoV( XYZ *xyz, vec3 *v ){
	v[0][0] = (vec)Nearest( xyz->p[0],2. );
	v[0][1] = (vec)Nearest( xyz->p[1],2. );
	v[0][2] = (vec)Nearest( xyz->p[2],2. );
}

//=============================================================
void MakePatch( patchMesh_t *p ){
	int ret;
	char shadername[64 + 9];

	ret = g_FuncTable.m_pfnCreatePatchHandle();
	// strcpy(shadername, "textures/");
	// strcpy(shadername+9, Texture[Game][0]);
	strcpy( shadername, Texture[Game][0] );
	g_FuncTable.m_pfnCommitPatchHandleToMap( ret,p,shadername );
	g_FuncTable.m_pfnReleasePatchHandles();
}

//=============================================================
void MakeBrush( BRUSH *brush ){
	LPVOID vp;
	int i;
	_QERFaceData QERFaceData;

	if ( g_FuncTable.m_pfnCreateBrushHandle == NULL ) {
		g_FuncTable.m_pfnMessageBox( g_pRadiantWnd,"m_pfnCreateBrushHandle==NULL","Aw damn",0, NULL );
		return;
	}
	vp = ( g_FuncTable.m_pfnCreateBrushHandle )();
	if ( !vp ) {
		return;
	}
	for ( i = 0; i < brush->NumFaces; i++ )
	{
		if ( !strncmp( brush->face[i].texture, "textures/", 9 ) ) {
			strcpy( QERFaceData.m_TextureName,brush->face[i].texture );
		}
		else
		{
			strcpy( QERFaceData.m_TextureName,"textures/" );
			strcpy( QERFaceData.m_TextureName + 9,brush->face[i].texture );
		}
		QERFaceData.m_nContents = brush->face[i].Contents;
		QERFaceData.m_nFlags    = brush->face[i].Surface;
		QERFaceData.m_nValue    = brush->face[i].Value;
		QERFaceData.m_fShift[0] = brush->face[i].Shift[0];
		QERFaceData.m_fShift[1] = brush->face[i].Shift[1];
		QERFaceData.m_fRotate   = brush->face[i].Rotate;
		QERFaceData.m_fScale[0] = brush->face[i].Scale[0];
		QERFaceData.m_fScale[1] = brush->face[i].Scale[1];
		QERFaceData.m_v1[0]     = brush->face[i].v[0][0];
		QERFaceData.m_v1[1]     = brush->face[i].v[0][1];
		QERFaceData.m_v1[2]     = brush->face[i].v[0][2];
		QERFaceData.m_v2[0]     = brush->face[i].v[1][0];
		QERFaceData.m_v2[1]     = brush->face[i].v[1][1];
		QERFaceData.m_v2[2]     = brush->face[i].v[1][2];
		QERFaceData.m_v3[0]     = brush->face[i].v[2][0];
		QERFaceData.m_v3[1]     = brush->face[i].v[2][1];
		QERFaceData.m_v3[2]     = brush->face[i].v[2][2];
		QERFaceData.m_bBPrimit  = false;
		( g_FuncTable.m_pfnAddFaceData )( vp,&QERFaceData );
	}
	if ( g_FuncTable.m_pfnCommitBrushHandle != NULL ) {
		if ( h_func_group ) {
			g_FuncTable.m_pfnCommitBrushHandleToEntity( vp,h_func_group );
		}
		else{
			g_FuncTable.m_pfnCommitBrushHandle( vp );
		}
	}
}
//=============================================================
void OpenFuncGroup(){
	if ( g_FuncTable.m_pfnAllocateEpair != NULL ) {
		epair_t *ep;

		h_func_group = g_FuncTable.m_pfnCreateEntityHandle();
		ep = g_EntityTable.m_pfnAllocateEpair( "classname","func_group" );
		g_EntityTable.m_pfnSetEntityKeyValList( (entity_t *)h_func_group,ep );

		if ( AddTerrainKey ) { // ^Fishman - Add terrain key to func_group.
			epair_t *ep2;
			terrainkey = g_FuncTable.m_pfnCreateEntityHandle();
			ep2 = g_EntityTable.m_pfnAllocateEpair( "terrain","1" );
			ep->next = ep2;
			g_EntityTable.m_pfnSetEntityKeyValList( (entity_t *)h_func_group,ep );
		}
	}
	else{
		h_func_group = NULL;
	}
}
//=============================================================
void CloseFuncGroup(){
	if ( h_func_group ) {
		g_FuncTable.m_pfnCommitEntityHandleToMap( h_func_group );
	}
	if ( g_FuncTable.m_pfnSysUpdateWindows != NULL ) {
		g_FuncTable.m_pfnSysUpdateWindows( W_ALL );
	}
}
