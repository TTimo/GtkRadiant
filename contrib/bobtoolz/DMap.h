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

// DMap.h: interface for the DMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DMAP_H__ACAE597A_D26D_49AD_AA69_EDE743DB54FA__INCLUDED_ )
#define AFX_DMAP_H__ACAE597A_D26D_49AD_AA69_EDE743DB54FA__INCLUDED_

#include "DEntity.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DMap
{
public:
static void RebuildEntity( DEntity* ent );

void ResetTextures( const char* textureName, float fScale[2],      float fShift[2],      int rotation, const char* newTextureName, int bResetTextureName,  int bResetScale[2],  int bResetShift[2],  int bResetRotation );
void LoadAll( bool bLoadPatches = FALSE );
void BuildInRadiant( bool bAllowDestruction );
int m_nNextEntity;
DEntity* GetWorldSpawn();
void ClearEntities();

DEntity* GetEntityForID( int ID );
DEntity* AddEntity( const char* classname = "worldspawn", int ID = -1 );

list<DEntity*> entityList;

DMap();
virtual ~DMap();

int FixBrushes( bool rebuild );
};

#endif // !defined(AFX_DMAP_H__ACAE597A_D26D_49AD_AA69_EDE743DB54FA__INCLUDED_)
