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

// DMap.cpp: implementation of the DMap class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DMap.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DMap::DMap(){
	m_nNextEntity = 1;
	AddEntity( "worldspawn", 0 );
}

DMap::~DMap(){
	ClearEntities();
}

DEntity* DMap::AddEntity( const char *classname, int ID ){
	DEntity* newEntity;
	if ( ID == -1 ) {
		newEntity = new DEntity( classname, m_nNextEntity++ );
	}
	else{
		newEntity = new DEntity( classname, ID );
	}

	entityList.push_back( newEntity );

	return newEntity;
}

void DMap::ClearEntities(){
	m_nNextEntity = 1;

	for ( list<DEntity *>::const_iterator deadEntity = entityList.begin(); deadEntity != entityList.end(); deadEntity++ )
		delete *deadEntity;

	entityList.clear();
}

DEntity* DMap::GetEntityForID( int ID ){
	DEntity* findEntity = NULL;

	for ( list<DEntity *>::const_iterator chkEntity = entityList.begin(); chkEntity != entityList.end(); chkEntity++ )
	{
		if ( ( *chkEntity )->m_nID == ID ) {
			findEntity = ( *chkEntity );
			break;
		}
	}

	if ( !findEntity ) {
		findEntity = AddEntity( "worldspawn", ID );
	}

	return findEntity;
}


DEntity* DMap::GetWorldSpawn(){
	return GetEntityForID( 0 );
}

void DMap::BuildInRadiant( bool bAllowDestruction ){
	for ( list<DEntity *>::const_iterator buildEntity = entityList.begin(); buildEntity != entityList.end(); buildEntity++ )
		( *buildEntity )->BuildInRadiant( bAllowDestruction );
}

void DMap::LoadAll( bool bLoadPatches ){
	ClearEntities();

	g_FuncTable.m_pfnDeselectAllBrushes();

	int count = g_FuncTable.m_pfnGetEntityCount();

	for ( int i = 0; i < count; i++ )
	{
		DEntity* loadEntity;

		if ( i == 0 ) {
			loadEntity = GetWorldSpawn();
		}
		else{
			loadEntity = AddEntity( "", m_nNextEntity++ );
		}

		if ( !loadEntity->LoadFromEntity( i, bLoadPatches ) ) {
			delete loadEntity;
			entityList.pop_back();
		}
	}
}

int DMap::FixBrushes( bool rebuild ){
	int count = 0;
	for ( list<DEntity *>::const_iterator fixEntity = entityList.begin(); fixEntity != entityList.end(); fixEntity++ )
	{
		int cnt;

		if ( !stricmp( "worldspawn", ( *fixEntity )->m_Classname ) ) {
			cnt = ( *fixEntity )->FixBrushes( rebuild );
		}
		else
		{
			cnt = ( *fixEntity )->FixBrushes( FALSE );

			if ( cnt && rebuild ) {
				RebuildEntity( *fixEntity );
			}
		}

		count += cnt;
	}

	return count;
}

void DMap::ResetTextures( const char* textureName, float fScale[2],      float fShift[2],      int rotation, const char* newTextureName,
						  int bResetTextureName,  int bResetScale[2],  int bResetShift[2],  int bResetRotation ){
	for ( list<DEntity *>::const_iterator texEntity = entityList.begin(); texEntity != entityList.end(); texEntity++ )
	{
		if ( !stricmp( "worldspawn", ( *texEntity )->m_Classname ) ) {
			( *texEntity )->ResetTextures( textureName,        fScale,       fShift,       rotation, newTextureName,
										   bResetTextureName,  bResetScale,  bResetShift,  bResetRotation, TRUE );
		}
		else
		{
			if ( ( *texEntity )->ResetTextures( textureName,        fScale,       fShift,       rotation, newTextureName,
												bResetTextureName,  bResetScale,  bResetShift,  bResetRotation, FALSE ) ) {
				RebuildEntity( *texEntity );
			}
		}
	}
}

void DMap::RebuildEntity( DEntity *ent ){
	ent->RemoveFromRadiant();
	ent->BuildInRadiant( FALSE );
}
