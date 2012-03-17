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

// DShape.cpp: implementation of the DShape class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DShape.h"

//#include "dialogs-gtk.h"

#include "misc.h"
#include "shapes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool bFacesAll[6] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};

DShape::DShape(){
	m_nNextBrush = 0;
}

DShape::~DShape(){

}

void DShape::BuildRegularPrism( vec3_t min, vec3_t max, int nSides, bool bAlignTop ){
	vec3_t vc[MAX_POLYGON_FACES + 2], vd[MAX_POLYGON_FACES + 2];

	vec3_t radius;
	vec3_t origin;

	VectorSubtract( max, min, radius );
	VectorScale( radius, 0.5f, radius );
	// calc 3d radius and origin
	VectorAdd( max, min, origin );
	VectorScale( origin, 0.5f, origin );

	float phase = 0.0f;

	if ( bAlignTop ) {
		phase = -( Q_PI / nSides );
		VectorScale( radius, static_cast< float >( 1 / cos( phase ) ), radius );
	}

	//----- Build Polygon Vertices -----

	int i;
	for ( i = 0; i < nSides; i++ )
	{
		VectorCopy( origin, vc[i] );
		VectorCopy( origin, vd[i] );

		vc[i][2] = min[2];
		vd[i][2] = max[2];

		vc[i][0] += radius[0] * sinf( ( 2 * Q_PI * i / nSides ) + phase );
		vc[i][1] += radius[1] * cosf( ( 2 * Q_PI * i / nSides ) + phase );

		vd[i][0] = vc[i][0];
		vd[i][1] = vc[i][1];
	}

	VectorCopy( vc[0], vc[nSides] );
	VectorCopy( vd[0], vd[nSides] );
	VectorCopy( vc[1], vc[nSides + 1] );
	VectorCopy( vd[1], vd[nSides + 1] );

	//----------------------------------

	DBrush* pB = m_Container.GetWorldSpawn()->NewBrush( m_nNextBrush++ );

	for ( i = 1; i <= nSides; i++ )
		pB->AddFace( vc[i - 1], vc[i], vd[i], GetCurrentTexture(), FALSE );

	pB->AddFace( vc[2], vc[1], vc[0], "textures/common/caulk", FALSE );
	pB->AddFace( vd[0], vd[1], vd[2], "textures/common/caulk", FALSE );
}

void DShape::Commit(){
	m_Container.GetWorldSpawn()->FixBrushes( FALSE );
	m_Container.BuildInRadiant( TRUE );
}

void DShape::BuildInversePrism( vec3_t min, vec3_t max, int nSides, bool bAlignTop ){
	vec3_t va[MAX_POLYGON_FACES + 1], vb[MAX_POLYGON_FACES + 1];
	vec3_t radius;
	vec3_t origin;

	VectorSubtract( max, min, radius );
	VectorScale( radius, 0.5f, radius );
	// calc 3d radius and origin
	VectorAdd( max, min, origin );
	VectorScale( origin, 0.5f, origin );

	float phase = 0.0f;

	if ( bAlignTop ) {
		phase = -( Q_PI / nSides );
		VectorScale( radius, static_cast< float >( 1 / cos( phase ) ), radius );
	}

	//----- Build Polygon Vertices -----

	int i;
	for ( i = 0; i < nSides; i++ )
	{
		VectorCopy( origin, va[i] );
		VectorCopy( origin, vb[i] );

		va[i][2] = min[2];
		vb[i][2] = max[2];

		va[i][0] += radius[0] * sinf( ( 2 * Q_PI * i / nSides ) + phase );
		va[i][1] += radius[1] * cosf( ( 2 * Q_PI * i / nSides ) + phase );

		vb[i][0] = va[i][0];
		vb[i][1] = va[i][1];
	}

	VectorCopy( va[0], va[nSides] );
	VectorCopy( vb[0], vb[nSides] );

	//----------------------------------

	for ( i = 1; i <= nSides; i++ )
	{
		DBrush* pB = GetBoundingCube( min, max, "textures/common/caulk" );

		vec3_t top, bottom;
		VectorCopy( va[i - 1], top );
		VectorCopy( va[i], bottom );

		if ( va[i - 1][1] > va[i][1] ) {
			top[0] += 5;
			bottom[0] += 5;
		}
		else    // flip direction of plane on crossover
		{
			top[0] -= 5;
			bottom[0] -= 5;
		}

		if ( top[1] != bottom[1] ) { // internal line is flat already if true
			pB->AddFace( va[i - 1], top, vb[i - 1], "textures/common/caulk", FALSE );
			pB->AddFace( va[i], vb[i], bottom, "textures/common/caulk", FALSE );
		}   // add cut-off planes

		pB->AddFace( va[i - 1], vb[i - 1], vb[i], GetCurrentTexture(), FALSE );
		// add internal polygon plane
	}
}

void DShape::BuildBorderedPrism( vec3_t min, vec3_t max, int nSides, int nBorder, bool bAlignTop ){
	vec3_t va[MAX_POLYGON_FACES + 2], vb[MAX_POLYGON_FACES + 2];
	vec3_t vc[MAX_POLYGON_FACES + 2], vd[MAX_POLYGON_FACES + 2];

	vec3_t radius;
	vec3_t origin;

	VectorSubtract( max, min, radius );
	VectorScale( radius, 0.5f, radius );
	// calc 3d radius and origin
	VectorAdd( max, min, origin );
	VectorScale( origin, 0.5f, origin );

	if ( nBorder >= Min( radius[0], radius[1] ) ) {
//		DoMessageBox("Border is too large", "Error", MB_OK);
		return;
	}

	float phase = 0.0f;

	if ( bAlignTop ) {
		phase = -( Q_PI / nSides );
		VectorScale( radius, static_cast< float >( 1 / cos( phase ) ), radius );
	}

	//----- Build Polygon Vertices -----

	int i;
	for ( i = 0; i < nSides; i++ )
	{
		VectorCopy( origin, va[i] );
		VectorCopy( origin, vb[i] );
		VectorCopy( origin, vc[i] );
		VectorCopy( origin, vd[i] );

		va[i][2] = min[2];
		vb[i][2] = max[2];

		va[i][0] += ( radius[0] - nBorder ) * sinf( ( 2 * Q_PI * i / nSides ) + phase );
		va[i][1] += ( radius[1] - nBorder ) * cosf( ( 2 * Q_PI * i / nSides ) + phase );

		vb[i][0] = va[i][0];
		vb[i][1] = va[i][1];



		vc[i][2] = min[2];
		vd[i][2] = max[2];

		vc[i][0] += radius[0] * sinf( ( 2 * Q_PI * i / nSides ) + phase );
		vc[i][1] += radius[1] * cosf( ( 2 * Q_PI * i / nSides ) + phase );

		vd[i][0] = vc[i][0];
		vd[i][1] = vc[i][1];
	}

	VectorCopy( va[0], va[nSides] );
	VectorCopy( vb[0], vb[nSides] );
	VectorCopy( va[1], va[nSides + 1] );
	VectorCopy( vb[1], vb[nSides + 1] );

	VectorCopy( vc[0], vc[nSides] );
	VectorCopy( vd[0], vd[nSides] );
	VectorCopy( vc[1], vc[nSides + 1] );
	VectorCopy( vd[1], vd[nSides + 1] );

	//----------------------------------

	for ( i = 1; i <= nSides; i++ )
	{
		DBrush* pB = GetBoundingCube( min, max, "textures/common/caulk" );

		pB->AddFace( origin, vc[i - 1], vd[i - 1], "textures/common/caulk", FALSE );
		pB->AddFace( origin, vd[i], vc[i], "textures/common/caulk", FALSE );

		pB->AddFace( vc[i - 1], vc[i], vd[i], GetCurrentTexture(), FALSE );
		pB->AddFace( vb[i], va[i], va[i - 1], GetCurrentTexture(), FALSE );
	}
}

DBrush* DShape::GetBoundingCube_Ext( vec3_t min, vec3_t max, const char *textureName, bool* bUseFaces, bool detail ){
	DBrush* pB = new DBrush;
	//----- Build Outer Bounds ---------

	vec3_t v1, v2, v3, v5, v6, v7;
	VectorCopy( min, v1 );
	VectorCopy( min, v2 );
	VectorCopy( min, v3 );
	VectorCopy( max, v5 );
	VectorCopy( max, v6 );
	VectorCopy( max, v7 );

	v2[0] = max[0];
	v3[1] = max[1];

	v6[0] = min[0];
	v7[1] = min[1];

	//----------------------------------

	//----- Add Six Cube Faces ---------

	if ( bUseFaces[0] ) {
		pB->AddFace( v1, v2, v3, textureName, detail );
	}
	if ( bUseFaces[1] ) {
		pB->AddFace( v1, v3, v6, textureName, detail );
	}
	if ( bUseFaces[2] ) {
		pB->AddFace( v1, v7, v2, textureName, detail );
	}

	if ( bUseFaces[3] ) {
		pB->AddFace( v5, v6, v3, textureName, detail );
	}
	if ( bUseFaces[4] ) {
		pB->AddFace( v5, v2, v7, textureName, detail );
	}
	if ( bUseFaces[5] ) {
		pB->AddFace( v5, v7, v6, textureName, detail );
	}

	//----------------------------------

	return pB;
}

DBrush* DShape::GetBoundingCube( vec3_t min, vec3_t max, const char *textureName, DEntity* ent, bool* bUseFaces ){
	DBrush* pB;
	if ( ent == NULL ) {
		pB = m_Container.GetWorldSpawn()->NewBrush( m_nNextBrush++ );
	}
	else{
		pB = ent->NewBrush( m_nNextBrush++ );
	}

	//----- Build Outer Bounds ---------

	vec3_t v1, v2, v3, v5, v6, v7;
	VectorCopy( min, v1 );
	VectorCopy( min, v2 );
	VectorCopy( min, v3 );
	VectorCopy( max, v5 );
	VectorCopy( max, v6 );
	VectorCopy( max, v7 );

	v2[0] = max[0];
	v3[1] = max[1];

	v6[0] = min[0];
	v7[1] = min[1];

	//----------------------------------

	//----- Add Six Cube Faces ---------

	if ( bUseFaces[0] ) {
		pB->AddFace( v1, v2, v3, textureName, FALSE );
	}
	if ( bUseFaces[1] ) {
		pB->AddFace( v1, v3, v6, textureName, FALSE );
	}
	if ( bUseFaces[2] ) {
		pB->AddFace( v1, v7, v2, textureName, FALSE );
	}

	if ( bUseFaces[3] ) {
		pB->AddFace( v5, v6, v3, textureName, FALSE );
	}
	if ( bUseFaces[4] ) {
		pB->AddFace( v5, v2, v7, textureName, FALSE );
	}
	if ( bUseFaces[5] ) {
		pB->AddFace( v5, v7, v6, textureName, FALSE );
	}

	//----------------------------------

	return pB;
}

bool DShape::BuildPit( vec3_t min, vec3_t max ){
	if ( ( max[2] - min[2] ) < 196 ) {
		return FALSE;
	}

	srand( time( NULL ) );

	vec3_t centre;
	VectorAdd( min, max, centre );
	VectorScale( centre, 0.5f, centre );

	char buffer[256];

	int team = ( rand() % 10000 ) + 5000;

// ************* SPEAKER ***************
	sprintf( buffer, "t%i_1", team );

// trigger for speaker
	vec3_t triggerVoiceBtm;
	VectorCopy( min, triggerVoiceBtm );
	triggerVoiceBtm[2] = max[2] - 16;

	DEntity* triggerVoice = m_Container.AddEntity( "trigger_multiple" );
	GetBoundingCube( triggerVoiceBtm, max, "textures/common/trigger", triggerVoice );
	triggerVoice->AddEPair( "target", buffer );
//--------------------

// target for speaker
	vec3_t voiceOrigin;
	VectorCopy( centre, voiceOrigin );
	voiceOrigin[2] = max[2] + 16;


	DEntity* targetVoice = m_Container.AddEntity( "target_speaker" );
	targetVoice->AddEPair( "targetname", buffer );

	sprintf( buffer, "%f %f %f", voiceOrigin[0], voiceOrigin[1], voiceOrigin[2] );
	targetVoice->AddEPair( "origin", buffer );
	targetVoice->AddEPair( "spawnflags", "8" );
	targetVoice->AddEPair( "noise", "*falling1.wav" );
//--------------------

// *********** END SPEAKER *************

// ********* POWERUP REMOVAL ***********
	sprintf( buffer, "t%i_2", team );

// trigger for powerup removal
	vec3_t triggerPwrRmvTop, triggerPwrRmvBtm;
	VectorCopy( min, triggerPwrRmvBtm );
	VectorCopy( max, triggerPwrRmvTop );

	triggerPwrRmvTop[2] = triggerVoiceBtm[2] - 64;
	triggerPwrRmvBtm[2] = triggerPwrRmvTop[2] - 16;

	DEntity* triggerPwrRmv = m_Container.AddEntity( "trigger_multiple" );
	GetBoundingCube( triggerPwrRmvBtm, triggerPwrRmvTop, "textures/common/trigger", triggerPwrRmv );
	triggerPwrRmv->AddEPair( "target", buffer );
//--------------------

// target for powerup removal
	vec3_t pwrRmvOrigin;
	VectorCopy( centre, pwrRmvOrigin );
	pwrRmvOrigin[2] = triggerPwrRmvTop[2] + 16;

	DEntity* targetPwrRmv = m_Container.AddEntity( "target_remove_powerups" );
	targetPwrRmv->AddEPair( "targetname", buffer );

	sprintf( buffer, "%f %f %f", pwrRmvOrigin[0], pwrRmvOrigin[1], pwrRmvOrigin[2] );
	targetPwrRmv->AddEPair( "origin", buffer );
//--------------------

// ****** END POWERUP REMOVAL ********

// ********* DAMAGE ***********

// trigger for damage
	vec3_t triggerDmgTop, triggerDmgBtm;
	VectorCopy( min, triggerDmgBtm );
	VectorCopy( max, triggerDmgTop );

	triggerDmgBtm[2] = min[2] + 64;
	triggerDmgTop[2] = triggerDmgBtm[2] + 16;

	DEntity* triggerDmg = m_Container.AddEntity( "trigger_hurt" );
	GetBoundingCube( triggerDmgBtm, triggerDmgTop, "textures/common/trigger", triggerDmg );
	triggerDmg->AddEPair( "dmg", "9999" );
	triggerDmg->AddEPair( "spawnflags", "12" );
//--------------------

// ****** END DAMAGE ********

// ********* NODROP ***********

	vec3_t nodropTop;
	VectorCopy( max, nodropTop );

	nodropTop[2] = min[2] + 64;

	GetBoundingCube( min, nodropTop, "textures/common/nodrop" );

// ****** END NODROP ********

	return TRUE;
}
