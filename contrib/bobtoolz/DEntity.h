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

// DEntity.h: interface for the DEntity class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DENTITY_H__35B2C523_F0A7_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DENTITY_H__35B2C523_F0A7_11D4_ACF7_004095A18133__INCLUDED_

#include "DBrush.h"
#include "DEPair.h"
#include "DPatch.h"
#include "StdAfx.h" // Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DEntity
{
public:
void RemoveFromRadiant();
entity_t* QER_Entity;
int m_nID;

//	Constrcution/Destruction
DEntity( const char* classname = "worldspawn", int ID = -1 );     // sets classname
virtual ~DEntity();
//	---------------------------------------------

//	epair functions........
void LoadEPairList( epair_t* epl );
void AddEPair( const char* key, const char* value );
void ClearEPairs();
DEPair* FindEPairByKey( const char* keyname );
//	---------------------------------------------

//	random functions........
bool ResetTextures( const char* textureName, float fScale[2], float fShift[2], int rotation, const char* newTextureName, int bResetTextureName, int bResetScale[2], int bResetShift[2], int bResetRotation, bool rebuild );
void SaveToFile( FILE* pFile );
void SetClassname( const char* classname );
int GetIDMax();

void BuildInRadiant( bool allowDestruction );
void ResetChecks( list<Str>* exclusionList );
void RemoveNonCheckBrushes( list<Str>* exclusionList, bool useDetail );

DPlane* AddFaceToBrush( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* faceData, int ID );      // slow, try not to use much
int GetBrushCount( void );
DBrush* FindBrushByPointer( brush_t* brush );
//	---------------------------------------------


//	bool list functions
void SelectBrushes( bool* selectList );
bool* BuildDuplicateList();
bool* BuildIntersectList();
//	---------------------------------------------


//	brush operations
void ClearBrushes();        // clears brush list and frees memory for brushes

DBrush* GetBrushForID( int ID );
DBrush* NewBrush( int ID = -1 );
//	---------------------------------------------

//	patch operations
void ClearPatches();

DPatch* NewPatch();
//	---------------------------------------------

//	vars
list<DEPair*> epairList;
list<DBrush*> brushList;
// new patches, wahey!!!
list<DPatch*> patchList;
Str m_Classname;
//	---------------------------------------------


int FixBrushes( bool rebuild );

bool LoadFromEntity( int id, bool bLoadPatches = FALSE );
bool LoadFromEntity( entity_t* ent, bool bLoadPatches = FALSE );
void LoadSelectedBrushes();
void LoadSelectedPatches();

bool LoadFromPrt( char* filename );
//	---------------------------------------------
void SpawnString( const char* key, const char* defaultstring, const char** out );
void SpawnInt( const char* key, const char* defaultstring, int* out );
void SpawnFloat( const char* key, const char* defaultstring, float* out );
void SpawnVector( const char* key, const char* defaultstring, vec_t* out );
};

#endif // !defined(AFX_DENTITY_H__35B2C523_F0A7_11D4_ACF7_004095A18133__INCLUDED_)
