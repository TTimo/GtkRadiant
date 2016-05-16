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

// DBrush.cpp: implementation of the DBrush class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "DBrush.h"
#include "DWinding.h"
#include "dialogs/dialogs-gtk.h"

#include "misc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBrush::DBrush( int ID ){
	m_nBrushID = ID;
	bBoundsBuilt = FALSE;
	QER_brush = NULL;
}

DBrush::~DBrush(){
	ClearFaces();
	ClearPoints();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

DPlane* DBrush::AddFace( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* texData ){
#ifdef _DEBUG
//	Sys_Printf("(%f %f %f) (%f %f %f) (%f %f %f)\n", va[0], va[1], va[2], vb[0], vb[1], vb[2], vc[0], vc[1], vc[2]);
#endif
	bBoundsBuilt = FALSE;
	DPlane* newFace = new DPlane( va, vb, vc, texData );
	faceList.push_back( newFace );

	return newFace;
}

int DBrush::BuildPoints(){
	ClearPoints();

	if ( faceList.size() <= 3 ) {  // if less than 3 faces, there can be no points
		return 0;                   // with only 3 faces u can't have a bounded soild

	}
	for ( list<DPlane *>::const_iterator p1 = faceList.begin(); p1 != faceList.end(); p1++ )
	{
		list<DPlane *>::const_iterator p2 = p1;
		for ( p2++; p2 != faceList.end(); p2++ )
		{
			list<DPlane *>::const_iterator p3 = p2;
			for ( p3++; p3 != faceList.end(); p3++ )
			{
				vec3_t pnt;
				if ( ( *p1 )->PlaneIntersection( *p2, *p3, pnt ) ) {
					int pos = PointPosition( pnt );

					if ( pos == POINT_IN_BRUSH ) { // ???? shouldn't happen here
						Sys_FPrintf( SYS_ERR, "ERROR:: Build Brush Points: Point IN brush!!!\n" );
					}
					else if ( pos == POINT_ON_BRUSH ) { // normal point
						if ( !HasPoint( pnt ) ) {
							AddPoint( pnt );
						}
/*						else
                            Sys_Printf("Duplicate Point Found, pyramids ahoy!!!!!\n");*/
						// point lies on more that 3 planes
					}

					// otherwise point is removed due to another plane..

					// Sys_Printf("(%f, %f, %f)\n", pnt[0], pnt[1], pnt[2]);
				}
			}
		}
	}

#ifdef _DEBUG
//	Sys_Printf("%i points on brush\n", pointList.size());
#endif

	return pointList.size();
}

void DBrush::LoadFromBrush_t( brush_t* brush, bool textured ){
	ClearFaces();
	ClearPoints();

	for ( int i = g_FuncTable.m_pfnGetFaceCount( brush ) - 1; i >= 0 ; i-- )
	{   // running backwards so i dont have to use the count function each time (OPT)
		_QERFaceData* faceData = g_FuncTable.m_pfnGetFaceData( brush, i );

		if ( faceData == NULL ) {
			DoMessageBox( "Null pointer returned", "WARNING!", MB_OK );
		}

		if ( textured ) {
			AddFace( faceData->m_v1, faceData->m_v2, faceData->m_v3, faceData );
		}
		else{
			AddFace( faceData->m_v1, faceData->m_v2, faceData->m_v3, NULL );
		}
	}

	QER_brush = brush;
}

int DBrush::PointPosition( vec3_t pnt ){
	int state = POINT_IN_BRUSH; // if nothing happens point is inside brush

	for ( list<DPlane *>::const_iterator chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
	{
		float dist = ( *chkPlane )->DistanceToPoint( pnt );

		if ( dist > MAX_ROUND_ERROR ) {
			return POINT_OUT_BRUSH;     // if point is in front of plane, it CANT be in the brush
		}
		else if ( fabs( dist ) < MAX_ROUND_ERROR ) {
			state = POINT_ON_BRUSH;     // if point is ON plane point is either ON the brush
		}
		// or outside it, it can no longer be in it
	}

	return state;
}

void DBrush::ClearPoints(){
	for ( list<DPoint *>::const_iterator deadPoint = pointList.begin(); deadPoint != pointList.end(); deadPoint++ ) {
		delete *deadPoint;
	}
	pointList.clear();
}

void DBrush::ClearFaces(){
	bBoundsBuilt = FALSE;
	for ( list<DPlane *>::const_iterator deadPlane = faceList.begin(); deadPlane != faceList.end(); deadPlane++ )
	{
		delete *deadPlane;
	}
	faceList.clear();
}

void DBrush::AddPoint( vec3_t pnt ){
	DPoint* newPoint = new DPoint;
	VectorCopy( pnt, newPoint->_pnt );
	pointList.push_back( newPoint );
}

bool DBrush::HasPoint( vec3_t pnt ){
	for ( list<DPoint *>::const_iterator chkPoint = pointList.begin(); chkPoint != pointList.end(); chkPoint++ )
	{
		if ( **chkPoint == pnt ) {
			return TRUE;
		}
	}

	return FALSE;
}

int DBrush::RemoveRedundantPlanes(){
	int cnt = 0;
	list<DPlane *>::iterator chkPlane;

	// find duplicate planes
	list<DPlane *>::iterator p1 = faceList.begin();

	while ( p1 != faceList.end() )
	{
		list<DPlane *>::iterator p2 = p1;

		for ( p2++; p2 != faceList.end(); p2++ )
		{
			if ( **p1 == **p2 ) {
				if ( !strcmp( ( *p1 )->texInfo.m_TextureName, "textures/common/caulk" ) ) {
					delete *p1;
					p1 = faceList.erase( p1 );    // duplicate plane
				}
				else
				{
					delete *p2;
					p2 = faceList.erase( p2 );    // duplicate plane
				}

				cnt++;
				break;
			}
		}

		if ( p2 == faceList.end() ) {
			p1++;
		}
	}

	//+djbob kill planes with bad normal, they are more of a nuisance than losing a brush
	chkPlane = faceList.begin();
	while ( chkPlane != faceList.end() )
	{
		if ( VectorLength( ( *chkPlane )->normal ) == 0 ) { // plane has bad normal
			delete *chkPlane;
			chkPlane = faceList.erase( chkPlane );
			cnt++;
		}
		else {
			chkPlane++;
		}
	}
	//-djbob

	if ( pointList.size() == 0 ) { // if points may not have been built, build them
/*		if(BuildPoints() == 0)	// just let the planes die if they are all bad
            return cnt;*/
		BuildPoints();
	}

	chkPlane = faceList.begin();
	while ( chkPlane != faceList.end() )
	{
		if ( ( *chkPlane )->IsRedundant( pointList ) ) { // checks that plane "0wnz" :), 3 or more points
			delete *chkPlane;
			chkPlane = faceList.erase( chkPlane );
			cnt++;
		}
		else{
			chkPlane++;
		}
	}

	return cnt;
}

bool DBrush::GetBounds( vec3_t min, vec3_t max ){
	BuildBounds();

	if ( !bBoundsBuilt ) {
		return FALSE;
	}

	VectorCopy( bbox_min, min );
	VectorCopy( bbox_max, max );

	return TRUE;
}

bool DBrush::BBoxCollision( DBrush* chkBrush ){
	vec3_t min1, min2;
	vec3_t max1, max2;

	GetBounds( min1, max1 );
	chkBrush->GetBounds( min2, max2 );

	if ( min1[0] >= max2[0] ) {
		return FALSE;
	}
	if ( min1[1] >= max2[1] ) {
		return FALSE;
	}
	if ( min1[2] >= max2[2] ) {
		return FALSE;
	}

	if ( max1[0] <= min2[0] ) {
		return FALSE;
	}
	if ( max1[1] <= min2[1] ) {
		return FALSE;
	}
	if ( max1[2] <= min2[2] ) {
		return FALSE;
	}

	return TRUE;
}

DPlane* DBrush::HasPlane( DPlane* chkPlane ){
	for ( list<DPlane *>::const_iterator brushPlane = faceList.begin(); brushPlane != faceList.end(); brushPlane++ )
	{
		if ( **brushPlane == *chkPlane ) {
			return *brushPlane;
		}
	}
	return NULL;
}

bool DBrush::IsCutByPlane( DPlane *cuttingPlane ){
	bool isInFront;

	if ( pointList.size() == 0 ) {
		if ( BuildPoints() == 0 ) {
			return FALSE;
		}
	}

	list<DPoint *>::const_iterator chkPnt = pointList.begin();

	if ( chkPnt == pointList.end() ) {
		return FALSE;
	}

	float dist = cuttingPlane->DistanceToPoint( ( *chkPnt )->_pnt );

	if ( dist > MAX_ROUND_ERROR ) {
		isInFront = FALSE;
	}
	else if ( dist < MAX_ROUND_ERROR ) {
		isInFront = TRUE;
	}
	else{
		return TRUE;
	}

	for ( chkPnt++ = pointList.begin(); chkPnt != pointList.end(); chkPnt++ )
	{
		dist = cuttingPlane->DistanceToPoint( ( *chkPnt )->_pnt );

		if ( dist > MAX_ROUND_ERROR ) {
			if ( isInFront ) {
				return TRUE;
			}
		}
		else if ( dist < MAX_ROUND_ERROR ) {
			if ( !isInFront ) {
				return TRUE;
			}
		}
		else{
			return TRUE;
		}
	}

	return FALSE;
}

brush_t* DBrush::BuildInRadiant( bool allowDestruction, int* changeCnt, entity_t* entity ){
	if ( allowDestruction ) {
		bool kill = TRUE;

		for ( list<DPlane *>::const_iterator chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
		{
			if ( ( *chkPlane )->m_bChkOk ) {
				kill = FALSE;
				break;
			}
		}
		if ( kill ) {
			return NULL;
		}
	}

	//+djbob: fixed bug when brush had no faces "phantom brush" in radiant.
	if ( faceList.size() < 4 ) {
		Sys_Printf( "Possible Phantom Brush Found, will not rebuild\n" );
		return NULL;
	}
	//-djbob

	QER_brush = (brush_t*)g_FuncTable.m_pfnCreateBrushHandle();

	for ( list<DPlane *>::const_iterator buildPlane = faceList.begin(); buildPlane != faceList.end(); buildPlane++ ) {
		if ( ( *buildPlane )->AddToBrush_t( QER_brush ) && changeCnt ) {
			( *changeCnt )++;
		}
	}

	if ( entity ) {
		g_FuncTable.m_pfnCommitBrushHandleToEntity( QER_brush, entity );
		g_BrushTable.m_pfnBrush_Build( QER_brush, false, false, false, false );
		g_BrushTable.m_pfnBrush_AddToList( QER_brush, g_AppDataTable.m_pfnSelectedBrushes() );
	}
	else {
		g_FuncTable.m_pfnCommitBrushHandle( QER_brush );
	}

	return QER_brush;
}

void DBrush::CutByPlane( DPlane *cutPlane, DBrush **newBrush1, DBrush **newBrush2 ){
	if ( !IsCutByPlane( cutPlane ) ) {
		*newBrush1 = NULL;
		*newBrush2 = NULL;
		return;
	}

	DBrush* b1 = new DBrush;
	DBrush* b2 = new DBrush;

	for ( list<DPlane *>::const_iterator parsePlane = faceList.begin(); parsePlane != faceList.end(); parsePlane++ )
	{
		b1->AddFace( ( *parsePlane )->points[0], ( *parsePlane )->points[1], ( *parsePlane )->points[2], NULL );
		b2->AddFace( ( *parsePlane )->points[0], ( *parsePlane )->points[1], ( *parsePlane )->points[2], NULL );
	}

	b1->AddFace( cutPlane->points[0], cutPlane->points[1], cutPlane->points[2], NULL );
	b2->AddFace( cutPlane->points[2], cutPlane->points[1], cutPlane->points[0], NULL );

	b1->RemoveRedundantPlanes();
	b2->RemoveRedundantPlanes();

	*newBrush1 = b1;
	*newBrush2 = b2;
}

bool DBrush::IntersectsWith( DBrush *chkBrush ){
	if ( pointList.size() == 0 ) {
		if ( BuildPoints() == 0 ) {
			return FALSE;   // invalid brush!!!!

		}
	}
	if ( chkBrush->pointList.size() == 0 ) {
		if ( chkBrush->BuildPoints() == 0 ) {
			return FALSE;   // invalid brush!!!!

		}
	}
	if ( !BBoxCollision( chkBrush ) ) {
		return FALSE;
	}

	list<DPlane *>::const_iterator iplPlane;

	for ( iplPlane = faceList.begin(); iplPlane != faceList.end(); iplPlane++ )
	{

		bool allInFront = TRUE;
		for ( list<DPoint *>::const_iterator iPoint = chkBrush->pointList.begin(); iPoint != chkBrush->pointList.end(); iPoint++ )
		{
			if ( ( *iplPlane )->DistanceToPoint( ( *iPoint )->_pnt ) < -MAX_ROUND_ERROR ) {
				allInFront = FALSE;
				break;
			}
		}
		if ( allInFront ) {
			return FALSE;
		}
	}

	for ( iplPlane = chkBrush->faceList.begin(); iplPlane != chkBrush->faceList.end(); iplPlane++ )
	{
		bool allInFront = TRUE;
		for ( list<DPoint *>::const_iterator iPoint = pointList.begin(); iPoint != pointList.end(); iPoint++ )
		{
			if ( ( *iplPlane )->DistanceToPoint( ( *iPoint )->_pnt ) < -MAX_ROUND_ERROR ) {
				allInFront = FALSE;
				break;
			}
		}
		if ( allInFront ) {
			return FALSE;
		}
	}

	return TRUE;
}

bool DBrush::IntersectsWith( DPlane* p1, DPlane* p2, vec3_t v ) {
	vec3_t vDown = { 0, 0, -1 };

	list<DPlane *>::const_iterator iplPlane;
	for ( iplPlane = faceList.begin(); iplPlane != faceList.end(); iplPlane++ ) {
		DPlane* p = ( *iplPlane );

		vec_t d = DotProduct( p->normal, vDown );
		if ( d >= 0 ) {
			continue;
		}
		if ( p->PlaneIntersection( p1, p2, v ) ) {
			if ( PointPosition( v ) != POINT_OUT_BRUSH ) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

void DBrush::BuildBounds(){
	if ( !bBoundsBuilt ) {
		if ( pointList.size() == 0 ) { // if points may not have been built, build them
			if ( BuildPoints() == 0 ) {
				return;
			}
		}

		list<DPoint *>::const_iterator first = pointList.begin();
		VectorCopy( ( *first )->_pnt, bbox_min );
		VectorCopy( ( *first )->_pnt, bbox_max );

		list<DPoint *>::const_iterator point = pointList.begin();
		for ( point++; point != pointList.end(); point++ )
		{
			if ( ( *point )->_pnt[0] > bbox_max[0] ) {
				bbox_max[0] = ( *point )->_pnt[0];
			}
			if ( ( *point )->_pnt[1] > bbox_max[1] ) {
				bbox_max[1] = ( *point )->_pnt[1];
			}
			if ( ( *point )->_pnt[2] > bbox_max[2] ) {
				bbox_max[2] = ( *point )->_pnt[2];
			}

			if ( ( *point )->_pnt[0] < bbox_min[0] ) {
				bbox_min[0] = ( *point )->_pnt[0];
			}
			if ( ( *point )->_pnt[1] < bbox_min[1] ) {
				bbox_min[1] = ( *point )->_pnt[1];
			}
			if ( ( *point )->_pnt[2] < bbox_min[2] ) {
				bbox_min[2] = ( *point )->_pnt[2];
			}
		}

		bBoundsBuilt = TRUE;
	}
}

bool DBrush::BBoxTouch( DBrush *chkBrush ){
	vec3_t min1, min2;
	vec3_t max1, max2;

	GetBounds( min1, max1 );
	chkBrush->GetBounds( min2, max2 );

	if ( ( min1[0] - max2[0] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}
	if ( ( min1[1] - max2[1] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}
	if ( ( min1[2] - max2[2] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}

	if ( ( min2[0] - max1[0] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}
	if ( ( min2[1] - max1[1] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}
	if ( ( min2[2] - max1[2] ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}

	int cnt = 0;

	if ( ( min2[0] - max1[0] ) == 0 ) {
		cnt++;
	}

	if ( ( min2[1] - max1[1] ) == 0 ) {
		cnt++;
	}

	if ( ( min2[2] - max1[2] ) == 0 ) {
		cnt++;
	}

	if ( ( min1[0] - max2[0] ) == 0 ) {
		cnt++;
	}

	if ( ( min1[1] - max2[1] ) == 0 ) {
		cnt++;
	}

	if ( ( min1[2] - max2[2] ) == 0 ) {
		cnt++;
	}

	if ( cnt > 1 ) {
		return FALSE;
	}

	return TRUE;
}

void DBrush::ResetChecks( list<Str>* exclusionList ){
	for ( list<DPlane *>::const_iterator resetPlane = faceList.begin(); resetPlane != faceList.end(); resetPlane++ )
	{
		bool set = FALSE;

		if ( exclusionList ) {
			for ( list<Str>::iterator eTexture = exclusionList->begin(); eTexture != exclusionList->end(); eTexture++ )
			{
				if ( strstr( ( *resetPlane )->texInfo.m_TextureName, eTexture->GetBuffer() ) ) {
					set = TRUE;
					break;
				}
			}
		}

		( *resetPlane )->m_bChkOk = set;
	}
}

DPlane* DBrush::HasPlaneInverted( DPlane *chkPlane ){
	for ( list<DPlane *>::const_iterator brushPlane = faceList.begin(); brushPlane != faceList.end(); brushPlane++ )
	{
		if ( **brushPlane != *chkPlane ) {
			if ( fabs( ( *brushPlane )->_d + chkPlane->_d ) < 0.1 ) {
				return ( *brushPlane );
			}
		}
	}
	return NULL;
}

bool DBrush::HasTexture( const char *textureName ){
	for ( list<DPlane *>::const_iterator chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
	{
		if ( strstr( ( *chkPlane )->texInfo.m_TextureName, textureName ) ) {
			return TRUE;
		}

	}
	return FALSE;
}

bool DBrush::IsDetail(){
	for ( list<DPlane *>::const_iterator chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
	{
		if ( ( *chkPlane )->texInfo.m_nContents & FACE_DETAIL ) {
			return TRUE;
		}

	}
	return FALSE;
}

void DBrush::BuildFromWinding( DWinding *w ){
	if ( w->numpoints < 3 ) {
		Sys_ERROR( "Winding has invalid number of points" );
		return;
	}

	DPlane* wPlane = w->WindingPlane();

	DWinding* w2;
	w2 = w->CopyWinding();
	int i;
	for ( i = 0; i < w2->numpoints; i++ )
		VectorAdd( w2->p[i], wPlane->normal, w2->p[i] );

	AddFace( w2->p[0], w2->p[1], w2->p[2], NULL );
	AddFace( w->p[2], w->p[1], w->p[0], NULL );

	for ( i = 0; i < w->numpoints - 1; i++ )
		AddFace( w2->p[i], w->p[i], w->p[i + 1], NULL );
	AddFace( w2->p[w->numpoints - 1], w->p[w->numpoints - 1], w->p[0], NULL );

	delete wPlane;
	delete w2;
}

void DBrush::SaveToFile( FILE *pFile ){
	fprintf( pFile, "{\n" );

	for ( list<DPlane *>::const_iterator pp = faceList.begin(); pp != faceList.end(); pp++ )
	{
		char buffer[512];

		sprintf( buffer, "( %.0f %.0f %.0f ) ( %.0f %.0f %.0f ) ( %.0f %.0f %.0f ) %s %.0f %.0f %f %f %.0f 0 0 0\n",
				 ( *pp )->points[0][0], ( *pp )->points[0][1], ( *pp )->points[0][2],
				 ( *pp )->points[1][0], ( *pp )->points[1][1], ( *pp )->points[1][2],
				 ( *pp )->points[2][0], ( *pp )->points[2][1], ( *pp )->points[2][2],
				 ( *pp )->texInfo.m_TextureName,
				 ( *pp )->texInfo.m_fShift[0], ( *pp )->texInfo.m_fShift[1],
				 ( *pp )->texInfo.m_fScale[0], ( *pp )->texInfo.m_fScale[0],
				 ( *pp )->texInfo.m_fRotate );

		fprintf( pFile, "%s", buffer );
	}

	fprintf( pFile, "}\n" );
}

void DBrush::Rotate( vec3_t vOrigin, vec3_t vRotation ){
	for ( list<DPlane *>::const_iterator rotPlane = faceList.begin(); rotPlane != faceList.end(); rotPlane++ )
	{
		for ( int i = 0; i < 3; i++ )
			VectorRotate( ( *rotPlane )->points[i], vRotation, vOrigin );

		( *rotPlane )->Rebuild();
	}
}

void DBrush::RotateAboutCentre( vec3_t vRotation ){
	vec3_t min, max, centre;
	GetBounds( min, max );
	VectorAdd( min, max, centre );
	VectorScale( centre, 0.5f, centre );

	Rotate( centre, vRotation );
}

bool DBrush::ResetTextures( const char* textureName, float fScale[2],    float fShift[2],    int rotation, const char* newTextureName,
							int bResetTextureName,   int bResetScale[2], int bResetShift[2], int bResetRotation ){
	if ( textureName ) {
		bool changed = FALSE;
		for ( list<DPlane *>::const_iterator resetPlane = faceList.begin(); resetPlane != faceList.end(); resetPlane++ )
		{
			if ( !strcmp( ( *resetPlane )->texInfo.m_TextureName, textureName ) ) {
				if ( bResetTextureName ) {
					strcpy( ( *resetPlane )->texInfo.m_TextureName, newTextureName );
				}

				if ( bResetScale[0] ) {
					( *resetPlane )->texInfo.m_fScale[0] = fScale[0];
				}
				if ( bResetScale[1] ) {
					( *resetPlane )->texInfo.m_fScale[1] = fScale[1];
				}

				if ( bResetShift[0] ) {
					( *resetPlane )->texInfo.m_fShift[0] = fShift[0];
				}
				if ( bResetShift[1] ) {
					( *resetPlane )->texInfo.m_fShift[1] = fShift[1];
				}

				if ( bResetRotation ) {
					( *resetPlane )->texInfo.m_fRotate = (float)rotation;
				}

				changed = TRUE;
			}
		}
		return changed; // no point rebuilding unless we need to, only slows things down
	}
	else
	{
		for ( list<DPlane *>::const_iterator resetPlane = faceList.begin(); resetPlane != faceList.end(); resetPlane++ )
		{
			if ( bResetTextureName ) {
				strcpy( ( *resetPlane )->texInfo.m_TextureName, newTextureName );
			}

			if ( bResetScale[0] ) {
				( *resetPlane )->texInfo.m_fScale[0] = fScale[0];
			}
			if ( bResetScale[1] ) {
				( *resetPlane )->texInfo.m_fScale[1] = fScale[1];
			}

			if ( bResetShift[0] ) {
				( *resetPlane )->texInfo.m_fShift[0] = fShift[0];
			}
			if ( bResetShift[1] ) {
				( *resetPlane )->texInfo.m_fShift[1] = fShift[1];
			}

			if ( bResetRotation ) {
				( *resetPlane )->texInfo.m_fRotate = (float)rotation;
			}
		}
		return TRUE;
	}
}

bool DBrush::operator ==( DBrush* other ){
	list<DPlane *>::const_iterator chkPlane;

	for ( chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
	{
		if ( !other->HasPlane( ( *chkPlane ) ) ) {
			return FALSE;
		}
	}

	for ( chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ )
	{
		if ( !HasPlane( ( *chkPlane ) ) ) {
			return FALSE;
		}
	}

	return TRUE;
}

DPlane* DBrush::AddFace( vec3_t va, vec3_t vb, vec3_t vc, const char *textureName, bool bDetail ){
	bBoundsBuilt = FALSE;
	DPlane* newFace = new DPlane( va, vb, vc, textureName, bDetail );
	faceList.push_back( newFace );

	return newFace;
}

DPlane* DBrush::FindPlaneWithClosestNormal( vec_t* normal ) {
	vec_t bestDot = -2;
	DPlane* bestDotPlane = NULL;
	list<DPlane *>::const_iterator chkPlane;
	for ( chkPlane = faceList.begin(); chkPlane != faceList.end(); chkPlane++ ) {
		DPlane* pPlane = ( *chkPlane );

		vec_t dot = DotProduct( pPlane->normal, normal );
		if ( dot > bestDot ) {
			bestDot = dot;
			bestDotPlane = pPlane;
		}
	}

	return bestDotPlane;
}

int DBrush::FindPointsForPlane( DPlane* plane, DPoint** pnts, int maxpnts ) {
	int numpnts = 0;

	if ( !maxpnts ) {
		return 0;
	}

	BuildPoints();

	for ( list<DPoint *>::const_iterator points = pointList.begin(); points != pointList.end(); points++ ) {
		DPoint* point = ( *points );

		if ( fabs( plane->DistanceToPoint( point->_pnt ) ) < MAX_ROUND_ERROR ) {
			pnts[numpnts] = point;
			numpnts++;

			if ( numpnts >= maxpnts ) {
				return numpnts;
			}

		}
	}

	return numpnts;
}

void DBrush::RemovePlane( DPlane* plane ) {
	bBoundsBuilt = FALSE;
	for ( list<DPlane *>::const_iterator deadPlane = faceList.begin(); deadPlane != faceList.end(); deadPlane++ ) {
		if ( *deadPlane == plane ) {
			delete *deadPlane;
			faceList.remove( plane );
		}
	}
}

void DBrush::RemoveFromRadiant( void ) {
	if ( QER_brush ) {
		g_FuncTable.m_pfnDeleteBrushHandle( QER_brush );
	}
}
