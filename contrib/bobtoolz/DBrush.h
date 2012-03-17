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

// DBrush.h: interface for the DBrush class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DBRUSH_H__35B2C522_F0A7_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DBRUSH_H__35B2C522_F0A7_11D4_ACF7_004095A18133__INCLUDED_

#include "DPlane.h"

#define POINT_IN_BRUSH  0
#define POINT_ON_BRUSH  1
#define POINT_OUT_BRUSH 2

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DBrush
{
public:
DPlane* AddFace( vec3_t va, vec3_t vb, vec3_t vc, const char* textureName, bool bDetail );
void SaveToFile( FILE* pFile );

void Rotate( vec3_t vOrigin, vec3_t vRotation );
void RotateAboutCentre( vec3_t vRotation );

DPlane* HasPlaneInverted( DPlane* chkPlane );
DPlane* HasPlane( DPlane* chkPlane );
DPlane* AddFace( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* texData );

bool ResetTextures( const char* textureName, float fScale[2], float fShift[2], int rotation, const char* newTextureName, int bResetTextureName, int bResetScale[2], int bResetShift[2], int bResetRotation );
bool IsDetail();
bool HasTexture( const char* textureName );
bool IntersectsWith( DBrush *chkBrush );
bool IntersectsWith( DPlane* p1, DPlane* p2, vec3_t v );
bool IsCutByPlane( DPlane* cuttingPlane );
bool GetBounds( vec3_t min, vec3_t max );
bool HasPoint( vec3_t pnt );
bool BBoxCollision( DBrush* chkBrush );
bool BBoxTouch( DBrush* chkBrush );

int BuildPoints();
void BuildBounds();
void BuildFromWinding( DWinding* w );
brush_t* BuildInRadiant( bool allowDestruction, int* changeCnt, entity_t* entity = NULL );

void ResetChecks( list<Str>* exclusionList );

void ClearFaces();
void ClearPoints();

int RemoveRedundantPlanes( void );
void RemovePlane( DPlane* plane );
int PointPosition( vec3_t pnt );
void RemoveFromRadiant( void );


void CutByPlane( DPlane* cutPlane, DBrush** newBrush1, DBrush** newBrush2 );

void LoadFromBrush_t( brush_t* brush, bool textured );
void AddPoint( vec3_t pnt );

DPlane* FindPlaneWithClosestNormal( vec_t* normal );
int FindPointsForPlane( DPlane* plane, DPoint** pnts, int maxpnts );

DBrush( int ID = -1 );
virtual ~DBrush();

bool operator==( DBrush* other );

//	members
brush_t* QER_brush;
list<DPlane*> faceList;
list<DPoint*> pointList;
int m_nBrushID;
vec3_t bbox_min, bbox_max;
bool bBoundsBuilt;
};

//typedef CList<DBrush*, DBrush*> DBrushList;

#endif // !defined(AFX_DBRUSH_H__35B2C522_F0A7_11D4_ACF7_004095A18133__INCLUDED_)
