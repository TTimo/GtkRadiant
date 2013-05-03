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

// DEntity.cpp: implementation of the DEntity class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "DEntity.h"

#include "dialogs/dialogs-gtk.h"
#include "misc.h"
#include "CPortals.h"

const char* brushEntityList[] = {
	"worldspawn",
	"trigger_always",
	"trigger_hurt",
	"trigger_multiple",
	"trigger_push",
	"trigger_teleport",
	"func_bobbing",
	"func_button",
	"func_door",
	"func_group",
	"func_pendulum",
	"func_plat",
	"func_rotating",
	"func_static",
	"func_timer",
	"func_train",
	0
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEntity::DEntity( const char *classname, int ID ){
	SetClassname( classname );
	m_nID = ID;
	QER_Entity = NULL;
}

DEntity::~DEntity(){
	ClearPatches();
	ClearBrushes();
	ClearEPairs();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void DEntity::ClearBrushes(){
	for ( list<DBrush *>::const_iterator deadBrush = brushList.begin(); deadBrush != brushList.end(); deadBrush++ )
	{
		delete *deadBrush;
	}
	brushList.clear();
}

void DEntity::ClearPatches(){
	for ( list<DPatch *>::const_iterator deadPatch = patchList.begin(); deadPatch != patchList.end(); deadPatch++ )
	{
		delete *deadPatch;
	}
	patchList.clear();
}

DPatch* DEntity::NewPatch(){
	DPatch* newPatch = new DPatch;

	patchList.push_back( newPatch );

	return newPatch;
}

DBrush* DEntity::NewBrush( int ID ){
	DBrush* newBrush = new DBrush( ID );

	brushList.push_back( newBrush );

	return newBrush;
}

char* getNextBracket( char* s ){
	char* p = s;
	while ( *p )
	{
		p++;
		if ( *p == '(' ) {
			break;
		}
	}

	return p;
}

bool DEntity::LoadFromPrt( char *filename ){
	CPortals portals;
	strcpy( portals.fn, filename );
	portals.Load();

	if ( portals.node_count == 0 ) {
		return FALSE;
	}

	ClearBrushes();
	ClearEPairs();

	bool build = false;
	for ( unsigned int i = 0; i < portals.node_count; i++ )
	{
		build = false;
		DBrush* brush = NewBrush();

		for ( unsigned int j = 0; j < portals.node[i].portal_count; j++ )
		{
			for ( unsigned int k = 0; k < portals.node[i].portal[j].point_count - 2; k++ )
			{
				vec3_t v1, v2, normal, n;
				VectorSubtract( portals.node[i].portal[j].point[k + 2].p, portals.node[i].portal[j].point[k + 1].p, v1 );
				VectorSubtract( portals.node[i].portal[j].point[k].p, portals.node[i].portal[j].point[k + 1].p, v2 );
				CrossProduct( v1, v2, n );
				VectorNormalize( n, v2 );

				if ( k == 0 ) {
					VectorCopy( v2, normal );
				}
				else
				{
					VectorSubtract( v2, normal, v1 );
					if ( VectorLength( v1 ) > 0.01 ) {
						build = true;
						break;
					}
				}
			}

			if ( !build ) {
				brush->AddFace( portals.node[i].portal[j].point[2].p, portals.node[i].portal[j].point[1].p, portals.node[i].portal[j].point[0].p, "textures/common/caulk", FALSE );
			}
			else{
				brush->AddFace( portals.node[i].portal[j].point[0].p, portals.node[i].portal[j].point[1].p, portals.node[i].portal[j].point[2].p, "textures/common/caulk", FALSE );
			}
		}
		if ( build ) {
			brush->BuildInRadiant( FALSE, NULL );
		}
	}

	return TRUE;
}

DPlane* DEntity::AddFaceToBrush( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* faceData, int ID ){
	DBrush* buildBrush = GetBrushForID( ID );
	return buildBrush->AddFace( va, vb, vc, faceData );
	// slow, dont use much
}

DBrush* DEntity::GetBrushForID( int ID ){
	DBrush* buildBrush = NULL;

	for ( list<DBrush *>::const_iterator chkBrush = brushList.begin(); chkBrush != brushList.end(); chkBrush++ )
	{
		if ( ( *chkBrush )->m_nBrushID == ID ) {
			buildBrush = ( *chkBrush );
			break;
		}
	}

	if ( !buildBrush ) {
		buildBrush = NewBrush( ID );
	}

	return buildBrush;
}

void DEntity::LoadSelectedBrushes(){
	ClearBrushes();
	ClearEPairs();

	int count = g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	for ( int i = 0; i < count; i++ ) {
		brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( i );

		if ( brush->pPatch ) {
			continue;
		}

		DBrush* loadBrush = NewBrush( i );
		loadBrush->LoadFromBrush_t( brush, TRUE );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DEntity::LoadSelectedPatches(){
	ClearPatches();
	ClearEPairs();

	int count = g_FuncTable.m_pfnAllocateSelectedPatchHandles();

	for ( int i = 0; i < count; i++ )
	{
		//$ FIXME: m_pfnGetPatchHandle
		patchMesh_t *pmesh = (patchMesh_t*)g_FuncTable.m_pfnGetPatchData( i );

		DPatch* loadPatch = NewPatch();
		loadPatch->LoadFromBrush_t( pmesh->pSymbiot );
	}

	g_FuncTable.m_pfnReleasePatchHandles();
}

bool* DEntity::BuildIntersectList(){
	int max = GetIDMax();
	if ( max == 0 ) {
		return NULL;
	}

	bool* pbIntList = new bool[max];
	memset( pbIntList, 0, sizeof( bool ) * ( max ) );

	for ( list<DBrush *>::const_iterator pB1 = brushList.begin(); pB1 != brushList.end(); pB1++ )
	{
		list<DBrush *>::const_iterator pB2 = pB1;
		for ( pB2++; pB2 != brushList.end(); pB2++ )
		{
			if ( ( *pB1 )->IntersectsWith( ( *pB2 ) ) ) {
				pbIntList[( *pB1 )->m_nBrushID] = TRUE;
				pbIntList[( *pB2 )->m_nBrushID] = TRUE;
			}
		}
	}

	return pbIntList;
}

bool* DEntity::BuildDuplicateList(){
	int max = GetIDMax();
	if ( max == 0 ) {
		return NULL;
	}

	bool* pbDupList = new bool[max];
	memset( pbDupList, 0, sizeof( bool ) * ( max ) );

	for ( list<DBrush *>::const_iterator pB1 = brushList.begin(); pB1 != brushList.end(); pB1++ )
	{
		list<DBrush *>::const_iterator pB2 = pB1;
		for ( pB2++; pB2 != brushList.end(); pB2++ )
		{
			if ( **pB1 == *pB2 ) {
				pbDupList[( *pB1 )->m_nBrushID] = TRUE;
				pbDupList[( *pB2 )->m_nBrushID] = TRUE;
			}
		}
	}

	return pbDupList;
}

void DEntity::SelectBrushes( bool *selectList ){
	if ( selectList == NULL ) {
		return;
	}

	g_FuncTable.m_pfnDeselectAllBrushes();

	g_FuncTable.m_pfnAllocateActiveBrushHandles();

	for ( list<DBrush *>::const_iterator pBrush = brushList.begin(); pBrush != brushList.end(); pBrush++ )
	{
		if ( selectList[( *pBrush )->m_nBrushID] ) {
			g_FuncTable.m_pfnSelectBrush( ( *pBrush )->QER_brush );
		}
	}
	g_FuncTable.m_pfnReleaseActiveBrushHandles();
}

bool DEntity::LoadFromEntity( int id, bool bLoadPatches ) {
	return LoadFromEntity( (entity_t*)g_FuncTable.m_pfnGetEntityHandle( id ), bLoadPatches );
}

bool DEntity::LoadFromEntity( entity_t* ent, bool bLoadPatches ) {
	ClearPatches();
	ClearBrushes();
	ClearEPairs();

	QER_Entity = ent;

	epair_t* epl = *g_EntityTable.m_pfnGetEntityKeyValList( QER_Entity );
	LoadEPairList( epl );

	bool keep = FALSE;
	int i;
	for ( i = 0; brushEntityList[i]; i++ )
	{
		if ( !stricmp( brushEntityList[i], m_Classname ) ) {
			keep = TRUE;
			break;
		}
	}

	if ( !keep ) {
		return FALSE;
	}

	int count = g_FuncTable.m_pfnAllocateEntityBrushHandles( QER_Entity );

	for ( i = 0; i < count; i++ )
	{

		brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetEntityBrushHandle( i );

		if ( brush == NULL ) {
			DoMessageBox( "GTKRadiant returned a NULL pointer, NOT a good sign", "WARNING!!!", MB_OK );
			continue;
		}

		if ( brush->pPatch ) {
			if ( bLoadPatches ) {
				DPatch* loadPatch = NewPatch();
				loadPatch->LoadFromBrush_t( brush );
			}
		}
		else
		{
			DBrush* loadBrush = NewBrush( i );
			loadBrush->LoadFromBrush_t( brush, TRUE );
		}
	}

	g_FuncTable.m_pfnReleaseEntityBrushHandles();

	return TRUE;
}

void DEntity::RemoveNonCheckBrushes( list<Str>* exclusionList, bool useDetail ){
	list<DBrush *>::iterator chkBrush = brushList.begin();

	while ( chkBrush != brushList.end() )
	{
		if ( !useDetail ) {
			if ( ( *chkBrush )->IsDetail() ) {
				delete *chkBrush;
				chkBrush = brushList.erase( chkBrush );
				continue;
			}
		}

		list<Str>::iterator eTexture;

		for ( eTexture = exclusionList->begin(); eTexture != exclusionList->end(); eTexture++ )
		{
			if ( ( *chkBrush )->HasTexture( ( *eTexture ).GetBuffer() ) ) {
				delete *chkBrush;
				chkBrush = brushList.erase( chkBrush );
				break;
			}
		}

		if ( eTexture == exclusionList->end() ) {
			chkBrush++;
		}
	}
}

void DEntity::ResetChecks( list<Str>* exclusionList ){
	for ( list<DBrush *>::const_iterator resetBrush = brushList.begin(); resetBrush != brushList.end(); resetBrush++ )
	{
		( *resetBrush )->ResetChecks( exclusionList );
	}
}

int DEntity::FixBrushes( bool rebuild ){
	g_FuncTable.m_pfnAllocateActiveBrushHandles();

	int cnt = 0;

	for ( list<DBrush *>::const_iterator fixBrush = brushList.begin(); fixBrush != brushList.end(); fixBrush++ )
	{
		int count = ( *fixBrush )->RemoveRedundantPlanes();
		if ( count ) {
			cnt += count;
			if ( rebuild ) {
				g_FuncTable.m_pfnDeleteBrushHandle( ( *fixBrush )->QER_brush );

				( *fixBrush )->BuildInRadiant( FALSE, NULL );
			}
		}
	}

	g_FuncTable.m_pfnReleaseActiveBrushHandles();

	return cnt;
}

void DEntity::BuildInRadiant( bool allowDestruction ){
	bool makeEntity = strcmp( m_Classname, "worldspawn" ) ? true : false;

	if ( makeEntity ) {
		entity_t* pE = (entity_t*)g_FuncTable.m_pfnCreateEntityHandle();

		epair_t* pEpS = GetNextChainItem( NULL, "classname", m_Classname );

		epair_t* pEp = pEpS;

		for ( list<DEPair* >::const_iterator buildEPair = epairList.begin(); buildEPair != epairList.end(); buildEPair++ )
		{
			pEp = GetNextChainItem( pEp, ( *buildEPair )->key, ( *buildEPair )->value );
		}

		g_EntityTable.m_pfnSetEntityKeyValList( pE, pEpS );

		g_FuncTable.m_pfnCommitEntityHandleToMap( pE );

		for ( list<DBrush *>::const_iterator buildBrush = brushList.begin(); buildBrush != brushList.end(); buildBrush++ )
			( *buildBrush )->BuildInRadiant( allowDestruction, NULL, pE );

		for ( list<DPatch *>::const_iterator buildPatch = patchList.begin(); buildPatch != patchList.end(); buildPatch++ )
			( *buildPatch )->BuildInRadiant( pE );

		QER_Entity = pE;
	}
	else
	{
		for ( list<DBrush *>::const_iterator buildBrush = brushList.begin(); buildBrush != brushList.end(); buildBrush++ )
			( *buildBrush )->BuildInRadiant( allowDestruction, NULL );

		for ( list<DPatch *>::const_iterator buildPatch = patchList.begin(); buildPatch != patchList.end(); buildPatch++ )
			( *buildPatch )->BuildInRadiant();
	}
}



int DEntity::GetIDMax( void ) {
	int max = -1;
	for ( list<DBrush *>::const_iterator cntBrush = brushList.begin(); cntBrush != brushList.end(); cntBrush++ ) {
		if ( ( *cntBrush )->m_nBrushID > max ) {
			max = ( *cntBrush )->m_nBrushID;
		}
	}
	return max + 1;
}

void DEntity::SetClassname( const char *classname ) {
	m_Classname = classname;
}

void DEntity::SaveToFile( FILE *pFile ){
	fprintf( pFile, "{\n" );

	fprintf( pFile, "\"classname\" \"%s\"\n", (const char *)m_Classname );

	for ( list<DEPair *>::const_iterator ep = epairList.begin(); ep != epairList.end(); ep++ )
	{
		fprintf( pFile, "\"%s\" \"%s\"\n", (const char *)( *ep )->key, (const char *)( *ep )->value );
	}

	for ( list<DBrush *>::const_iterator bp = brushList.begin(); bp != brushList.end(); bp++ )
	{
		( *bp )->SaveToFile( pFile );
	}

	fprintf( pFile, "}\n" );
}

void DEntity::ClearEPairs(){
	for ( list<DEPair *>::const_iterator deadEPair = epairList.begin(); deadEPair != epairList.end(); deadEPair++ )
	{
		delete ( *deadEPair );
	}
	epairList.clear();
}

void DEntity::AddEPair( const char *key, const char *value ) {
	DEPair* newEPair;
	newEPair = FindEPairByKey( key );
	if ( !newEPair ) {
		newEPair = new DEPair;
		newEPair->Build( key, value );
		epairList.push_back( newEPair );
	}
	else {
		newEPair->Build( key, value );
	}
}

void DEntity::LoadEPairList( epair_t *epl ){
	epair_t* ep = epl;
	while ( ep )
	{
		if ( !strcmp( ep->key, "classname" ) ) {
			SetClassname( ep->value );
		}
		else{
			AddEPair( ep->key, ep->value );
		}

		ep = ep->next;
	}
}

bool DEntity::ResetTextures( const char* textureName, float fScale[2],     float fShift[2],    int rotation, const char* newTextureName,
							 int bResetTextureName,    int bResetScale[2], int bResetShift[2], int bResetRotation, bool rebuild ){
	g_FuncTable.m_pfnDeselectAllBrushes();

	g_FuncTable.m_pfnAllocateActiveBrushHandles();

	bool reset = FALSE;

	for ( list<DBrush *>::const_iterator resetBrush = brushList.begin(); resetBrush != brushList.end(); resetBrush++ )
	{
		bool tmp = ( *resetBrush )->ResetTextures( textureName,        fScale,       fShift,       rotation, newTextureName,
												   bResetTextureName,  bResetScale,  bResetShift,  bResetRotation );

		if ( tmp ) {
			reset = TRUE;

			if ( rebuild ) {
				entity_t *pE = ( *resetBrush )->QER_brush->owner;
				g_FuncTable.m_pfnDeleteBrushHandle( ( *resetBrush )->QER_brush );
				( *resetBrush )->BuildInRadiant( FALSE, NULL, pE->entityId == 0 ? NULL : pE );

				if ( pE->entityId == 0 ? NULL : pE ) {
				}
			}
		}
	}

	if ( bResetTextureName ) {
		for ( list<DPatch *>::const_iterator resetPatch = patchList.begin(); resetPatch != patchList.end(); resetPatch++ )
		{
			bool tmp = ( *resetPatch )->ResetTextures( textureName, newTextureName );

			if ( tmp ) {
				reset = TRUE;

				if ( rebuild ) {
					entity_t *pE = ( *resetPatch )->QER_brush->owner;
					g_FuncTable.m_pfnDeleteBrushHandle( ( *resetPatch )->QER_brush );
					( *resetPatch )->BuildInRadiant( pE->entityId == 0 ? NULL : pE );
				}
			}
		}
	}

	g_FuncTable.m_pfnReleaseActiveBrushHandles();

	return reset;
}

DEPair* DEntity::FindEPairByKey( const char* keyname ){
	for ( list<DEPair *>::const_iterator ep = epairList.begin(); ep != epairList.end(); ep++ )
	{
		char* c = ( *ep )->key;
		if ( !strcmp( c, keyname ) ) {
			return *ep;
		}
	}
	return NULL;
}

void DEntity::RemoveFromRadiant(){
	g_EntityTable.m_pfnEntity_Free( (entity_t*)QER_Entity );

	QER_Entity = NULL;
}

void DEntity::SpawnString( const char* key, const char* defaultstring, const char** out ){
	DEPair* pEP = FindEPairByKey( key );
	if ( pEP ) {
		*out = pEP->value;
	}
	else {
		*out = defaultstring;
	}
}

void DEntity::SpawnInt( const char* key, const char* defaultstring, int* out ){
	DEPair* pEP = FindEPairByKey( key );
	if ( pEP ) {
		*out = atoi( pEP->value );
	}
	else {
		*out = atoi( defaultstring );
	}
}

void DEntity::SpawnFloat( const char* key, const char* defaultstring, float* out ){
	DEPair* pEP = FindEPairByKey( key );
	if ( pEP ) {
		*out = static_cast< float >( atof( pEP->value ) );
	}
	else {
		*out = static_cast< float >( atof( defaultstring ) );
	}
}

void DEntity::SpawnVector( const char* key, const char* defaultstring, vec_t* out ){
	DEPair* pEP = FindEPairByKey( key );
	if ( pEP ) {
		sscanf( pEP->value, "%f %f %f", &out[0], &out[1], &out[2] );
	}
	else {
		sscanf( defaultstring, "%f %f %f", &out[0], &out[1], &out[2] );
	}
}

int DEntity::GetBrushCount( void ) {
	return brushList.size();
}

DBrush* DEntity::FindBrushByPointer( brush_t* brush ) {
	for ( list<DBrush *>::const_iterator listBrush = brushList.begin(); listBrush != brushList.end(); listBrush++ ) {
		DBrush* pBrush = ( *listBrush );
		if ( pBrush->QER_brush == brush ) {
			return pBrush;
		}
	}
	return NULL;
}
