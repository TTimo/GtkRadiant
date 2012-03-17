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

#include "StdAfx.h"

void CControlPointsManagerBFace::Init( int iPts, CtrlPts_t *Pts, C2DView *p2DView, int TexSize[2],
									   _QERFaceData* pFaceData, _QERQglTable *pQglTable ){
	ManagerState = Idle;
	m_NumPoints = iPts;
	m_pPts = Pts;
	// store the initial config
	memcpy( &m_RefPts, Pts, sizeof( CtrlPts_t ) );
	// init TM
	memset( m_TM, 0, sizeof( float[2][3] ) );
	m_TM[0][0] = 1.0f; m_TM[1][1] = 1.0f;
	m_bGotAnchor = false;
	m_TransOffset[0] = 0.0f; m_TransOffset[1] = 0.0f;
	m_TexSize[0] = TexSize[0];
	m_TexSize[1] = TexSize[1];
	m_pFaceData = pFaceData;

	CControlPointsManager::Init( p2DView, pQglTable );
}

bool CControlPointsManagerBFace::OnLButtonDown( int xPos, int yPos ){
	if ( ManagerState == Idle ) {
		int i;

		// scan the point list to see if we selected something
		for ( i = 0; i < m_NumPoints; i++ )
			if ( m_p2DView->DoesSelect( xPos, yPos, m_pPts->data[i] ) ) {
				m_iDragPoint = i;
				ManagerState = Drag;
				if ( m_bGotAnchor && i == m_iAnchorPoint ) {
					// this means we selected the Anchor, so we'll translate
					m_bGotAnchor = false;
				}
				// perhaps we won't use translation, but we can compute it anyway
				ComputeTransOffset( i );
				if ( m_bGotAnchor ) {
					// we have an Anchor and selected another point
					m_Anchor[0] = m_pPts->data[m_iAnchorPoint][0];
					m_Anchor[1] = m_pPts->data[m_iAnchorPoint][1];
				}
			}
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}
	return false;
}

bool CControlPointsManagerBFace::OnMouseMove( int xPos, int yPos ){
	if ( ManagerState == Drag ) {
		if ( m_bGotAnchor ) {
			// there's an anchor, we are rotating the shape
			// we need to work in XY space for orthonormality
			float Pt[2];
			vec3_t V1,V2;
			vec3_t cross;
			float c,s;
			// used in XY space
			float XYTM[2][3];
			float XYRefAnchor[2];
			float XYAnchor[2];
			m_p2DView->GridForWindow( Pt, xPos, yPos );
			V2[0] = Pt[0] - m_Anchor[0];
			V2[1] = Pt[1] - m_Anchor[1];
			V2[2] = 0.0f;
			V1[0] = m_RefPts.data[m_iDragPoint][0] - m_RefPts.data[m_iAnchorPoint][0];
			V1[1] = m_RefPts.data[m_iDragPoint][1] - m_RefPts.data[m_iAnchorPoint][1];
			V1[2] = 0.0f;
			// compute transformation from V1 to V2
			// we need to work in XY orthonormal space
			XYSpaceForSTSpace( V1, V1 );
			XYSpaceForSTSpace( V2, V2 );
			VectorNormalize( V2, V2 );
			VectorNormalize( V1, V1 );
			c = DotProduct( V1, V2 );
			CrossProduct( V1, V2, cross );
			s = VectorLength( cross );
			// we compute the transformation matrix in XY space
			// reference position of the Anchor in XY space
			XYSpaceForSTSpace( XYRefAnchor, m_RefPts.data[m_iAnchorPoint] );
			// current position of the Anchor in XY space
			XYSpaceForSTSpace( XYAnchor, m_Anchor );
			// compute transformation matrix
			XYTM[0][0] = c; XYTM[1][1] = c;
			if ( cross[2] > 0 ) {
				s *= -1.0f;
			}
			XYTM[0][1] = s; XYTM[1][0] = -s;
			XYTM[0][2] = -c * XYRefAnchor[0] - s * XYRefAnchor[1] + XYAnchor[0];
			XYTM[1][2] = s * XYRefAnchor[0] - c * XYRefAnchor[1] + XYAnchor[1];
			// express this transformation matrix in ST space
			m_TM[0][0] = XYTM[0][0];
			m_TM[1][0] = XYTM[1][0] * (float)m_TexSize[0] / (float)m_TexSize[1];
			m_TM[0][1] = XYTM[0][1] * (float)m_TexSize[1] / (float)m_TexSize[0];
			m_TM[1][1] = XYTM[1][1];
			m_TM[0][2] = XYTM[0][2] / (float)m_TexSize[0];
			m_TM[1][2] = XYTM[1][2] / (float)m_TexSize[1];
			// update all points
			UpdateCtrlPts();
		}
		else
		{
			// no Anchor point is defined, we translate all points
			m_p2DView->GridForWindow( m_pPts->data[m_iDragPoint], xPos, yPos );
			m_TM[0][2] = m_pPts->data[m_iDragPoint][0] + m_TransOffset[0];
			m_TM[1][2] = m_pPts->data[m_iDragPoint][1] + m_TransOffset[1];
			// update all points
			UpdateCtrlPts();
		}
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}

	return false;
}

bool CControlPointsManagerBFace::OnLButtonUp( int x, int y ){
	if ( ManagerState == Drag ) {
		// this button is gonna become our Anchor
		m_bGotAnchor = true;
		m_iAnchorPoint = m_iDragPoint;
		// let's get out of Drag mode
		ManagerState = Idle;
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}
	return false;
}

void CControlPointsManagerBFace::Render(){
	int i;

	m_pQglTable->m_pfn_qglColor3f( 0, 1, 0 );
	m_pQglTable->m_pfn_qglPointSize( 6 );
	m_pQglTable->m_pfn_qglBegin( GL_POINTS );
	for ( i = 0; i < m_NumPoints; i++ )
	{
		if ( ManagerState == Drag && i == m_iDragPoint ) {
			m_pQglTable->m_pfn_qglColor3f( 1, 0, 0 );
		}
		else if ( m_bGotAnchor && i == m_iAnchorPoint ) {
			m_pQglTable->m_pfn_qglColor3f( 0, 0, 1 );
		}
		m_pQglTable->m_pfn_qglVertex2f( m_pPts->data[i][0], m_pPts->data[i][1] );
		m_pQglTable->m_pfn_qglColor3f( 0, 1, 0 );
	}
	m_pQglTable->m_pfn_qglEnd();
}

void CControlPointsManagerBFace::UpdateCtrlPts(){
	int i;

	// update all points
	for ( i = 0; i < m_NumPoints; i++ )
	{
		m_pPts->data[i][0] = m_RefPts.data[i][0] * m_TM[0][0] + m_RefPts.data[i][1] * m_TM[0][1] + m_TM[0][2];
		m_pPts->data[i][1] = m_RefPts.data[i][0] * m_TM[1][0] + m_RefPts.data[i][1] * m_TM[1][1] + m_TM[1][2];
	}

	if ( g_bPrefsUpdateCameraView ) {
		Commit();
		// tell Radiant to update
		// NOTE: little speed optimisation, disable window updates, and only update camera view
		g_FuncTable.m_pfnSetScreenUpdate( false );
		g_SelectedFaceTable.m_pfnSetFaceInfo( 0, m_pFaceData );
		g_FuncTable.m_pfnSetScreenUpdate( true );
		g_FuncTable.m_pfnSysUpdateWindows( W_CAMERA );
	}
}

//++timo FIXME: we are using a global for the reference data, use a m_pCancelFaceData instead
void CControlPointsManagerBFace::Commit(){
	brushprimit_texdef_t aux;
	aux.coords[0][0] = m_TM[0][0] * g_CancelFaceData.brushprimit_texdef.coords[0][0] + m_TM[0][1] * g_CancelFaceData.brushprimit_texdef.coords[1][0];
	aux.coords[0][1] = m_TM[0][0] * g_CancelFaceData.brushprimit_texdef.coords[0][1] + m_TM[0][1] * g_CancelFaceData.brushprimit_texdef.coords[1][1];
	aux.coords[0][2] = m_TM[0][0] * g_CancelFaceData.brushprimit_texdef.coords[0][2] + m_TM[0][1] * g_CancelFaceData.brushprimit_texdef.coords[1][2] + m_TM[0][2];
	aux.coords[1][0] = m_TM[1][0] * g_CancelFaceData.brushprimit_texdef.coords[0][0] + m_TM[1][1] * g_CancelFaceData.brushprimit_texdef.coords[1][0];
	aux.coords[1][1] = m_TM[1][0] * g_CancelFaceData.brushprimit_texdef.coords[0][1] + m_TM[1][1] * g_CancelFaceData.brushprimit_texdef.coords[1][1];
	aux.coords[1][2] = m_TM[1][0] * g_CancelFaceData.brushprimit_texdef.coords[0][2] + m_TM[1][1] * g_CancelFaceData.brushprimit_texdef.coords[1][2] + m_TM[1][2];
	memcpy( &m_pFaceData->brushprimit_texdef, &aux, sizeof( brushprimit_texdef_t ) );
}

void CControlPointsManagerBFace::ComputeTransOffset( int i ){
	// compute the translation offset used to counteract rotation
	m_TransOffset[0] = -m_TM[0][0] * m_RefPts.data[i][0] - m_TM[0][1] * m_RefPts.data[i][1];
	m_TransOffset[1] = -m_TM[1][0] * m_RefPts.data[i][0] - m_TM[1][1] * m_RefPts.data[i][1];
}

void CControlPointsManagerBFace::XYSpaceForSTSpace( float xy[2], const float st[2] ){
	xy[0] = st[0] * (float)m_TexSize[0];
	xy[1] = st[1] * (float)m_TexSize[1];
}

/*
   ======================================================================
   patch manager
   ======================================================================
 */

void CControlPointsManagerPatch::Init( patchMesh_t* pWorkPatch, C2DView *p2DView, _QERQglTable *pQglTable, patchMesh_t* pPatch ){
	CControlPointsManager::Init( p2DView, pQglTable );
	m_pPatch = pPatch;
	m_pWorkPatch = pWorkPatch;
}

bool CControlPointsManagerPatch::OnLButtonDown( int xPos, int yPos ){
	if ( ManagerState == Idle ) {
		int i,j;

		// scan the point list to see if we selected something
		for ( i = 0; i < m_pPatch->width; i++ )
			for ( j = 0; j < m_pPatch->height; j++ )
				if ( m_p2DView->DoesSelect( xPos, yPos, m_pWorkPatch->ctrl[i][j].st ) ) {
					m_iDragPoint[0] = i;
					m_iDragPoint[1] = j;
					ManagerState = Drag;
				}
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}
	return false;
}

bool CControlPointsManagerPatch::OnMouseMove( int xPos, int yPos ){
	if ( ManagerState == Drag ) {
		m_p2DView->GridForWindow( m_pWorkPatch->ctrl[ m_iDragPoint[0] ][ m_iDragPoint[1] ].st, xPos, yPos );
		if ( g_bPrefsUpdateCameraView ) {
			Commit();
			// ask to rebuild the patch display data
			m_pPatch->bDirty = true;
			// send a repaint to the camera window as well
			g_FuncTable.m_pfnSysUpdateWindows( W_CAMERA );
		}
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}
	return false;
}

bool CControlPointsManagerPatch::OnLButtonUp( int x, int y ){
	if ( ManagerState == Drag ) {
		ManagerState = Idle;
		// send a repaint message
		g_pToolWnd->Redraw();
	}
	return false;
}

void CControlPointsManagerPatch::Render(){
	int i,j;

	m_pQglTable->m_pfn_qglColor3f( 0, 1, 0 );
	m_pQglTable->m_pfn_qglPointSize( 6 );
	m_pQglTable->m_pfn_qglBegin( GL_POINTS );
	for ( i = 0; i < m_pPatch->width; i++ )
		for ( j = 0; j < m_pPatch->height; j++ )
		{
			if ( ManagerState == Drag && i == m_iDragPoint[0] && j == m_iDragPoint[1] ) {
				m_pQglTable->m_pfn_qglColor3f( 1, 0, 0 );
			}
			m_pQglTable->m_pfn_qglVertex2f( m_pWorkPatch->ctrl[i][j].st[0], m_pWorkPatch->ctrl[i][j].st[1] );
			m_pQglTable->m_pfn_qglColor3f( 0, 1, 0 );
		}
	m_pQglTable->m_pfn_qglEnd();
}

void CControlPointsManagerPatch::Commit(){
	int i,j;
	for ( i = 0; i < m_pPatch->width; i++ )
		for ( j = 0; j < m_pPatch->height; j++ )
		{
			m_pPatch->ctrl[i][j].st[0] = m_pWorkPatch->ctrl[i][j].st[0];
			m_pPatch->ctrl[i][j].st[1] = m_pWorkPatch->ctrl[i][j].st[1];
		}
}
