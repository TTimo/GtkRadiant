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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// a class to handle control points in a 2D view
// TODO: this one can be placed under an interface, and provided to the editor as service
//
// NOTE: the C2DView *m_p2DView is the orthogonal mapping between window and ST space
// in Drag mode (for rotation) we need an orthonormal XY space
// we do ST <-> XY transformations using the texture size
// ( for translation-only moves, orthogonal is enough )
// FIXME: is there a better way to deal between Window space <-> ST space <-> XY space ?
//
// NOTE: ControlPointsManagers are a bit different between brush faces and patches
// so there's a base virtual class, and we have two versions

#ifndef _CONTROLPOINTSMANAGER_H_
#define _CONTROLPOINTSMANAGER_H_

class CControlPointsManager
{
protected:
// used by Render
_QERQglTable *m_pQglTable;
C2DView       *m_p2DView;
public:
CControlPointsManager() { m_pQglTable = NULL; m_p2DView = NULL; }
virtual ~CControlPointsManager() { }
void Init( C2DView *p2DView, _QERQglTable *pQglTable ) { m_pQglTable = pQglTable; m_p2DView = p2DView; }

virtual bool OnLButtonDown( int x, int y ) = 0;
virtual bool OnMouseMove( int x, int y ) = 0;
virtual bool OnLButtonUp( int x, int y ) = 0;

virtual void Render() = 0;
virtual void Commit() = 0;
};

// brush face manager
class CControlPointsManagerBFace : public CControlPointsManager
{
enum      EManagerState { Idle, Drag } ManagerState;
int m_NumPoints;
// initial geometry
CtrlPts_t m_RefPts;
// current geometry
CtrlPts_t *m_pPts;
// transform matrix ( 2DView is Window <-> ST )
float m_TM[2][3];
// texture size for ST <-> XY
int m_TexSize[2];
// used when translating
float m_TransOffset[2];
// dragged point index
int m_iDragPoint;
// do we have an anchor ?
bool m_bGotAnchor;
// anchor point index
int m_iAnchorPoint;
// coordinates of Anchor
float m_Anchor[2];
// used for commit
_QERFaceData  *m_pFaceData;

public:
// construction / init -------------------------------------------------
CControlPointsManagerBFace() { ManagerState = Idle; }
virtual ~CControlPointsManagerBFace() { }
// NOTE: pQglTable is sent to CControlPointsManager::Init
void Init( int iPts, CtrlPts_t * Pts, C2DView * p2DView, int TexSize[2], _QERFaceData * pFaceData, _QERQglTable * pQglTable );
// CControlPointsManager interface -------------------------------------

virtual bool OnLButtonDown( int x, int y );
virtual bool OnMouseMove( int x, int y );
virtual bool OnLButtonUp( int x, int y );

virtual void Render();
virtual void Commit();

private:
// internal members
void UpdateCtrlPts();
void ComputeTransOffset( int i );
void XYSpaceForSTSpace( float xy[2], const float st[2] );
};

// patch manager
class CControlPointsManagerPatch : public CControlPointsManager
{
enum      EManagerState { Idle, Drag } ManagerState;
// reference data, used for commits
patchMesh_t* m_pPatch;
// work patch, holds current data
patchMesh_t* m_pWorkPatch;
int m_iDragPoint[2];

public:
// construction / init -------------------------------------------------
CControlPointsManagerPatch() { ManagerState = Idle; }
virtual ~CControlPointsManagerPatch() { }
// NOTE: pQglTable is sent to CControlPointsManager::Init
void Init( patchMesh_t* pWorkPatch, C2DView *p2DView, _QERQglTable *pQglTable, patchMesh_t* pPatch );
// CControlPointsManager interface -------------------------------------

virtual bool OnLButtonDown( int x, int y );
virtual bool OnMouseMove( int x, int y );
virtual bool OnLButtonUp( int x, int y );

virtual void Render();
virtual void Commit();
};

#endif
