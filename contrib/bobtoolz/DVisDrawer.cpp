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

// BobView.cpp: implementation of the DVisDrawer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DPoint.h"
#include "DVisDrawer.h"
#include "misc.h"
#include "funchandlers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DVisDrawer::DVisDrawer(){
	refCount = 1;
	m_bHooked = FALSE;
	m_list = NULL;
}

DVisDrawer::~DVisDrawer(){
	if ( m_bHooked ) {
		UnRegister();
	}

	g_VisView = NULL;
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void DVisDrawer::Draw2D( VIEWTYPE vt ){
	if ( !m_list ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglDisable( GL_BLEND );
	g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );

	g_QglTable.m_pfn_qglPushMatrix();

	switch ( vt )
	{
	case XY:
		break;
	case XZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		break;
	case YZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		g_QglTable.m_pfn_qglRotatef( 270.0f, 0.0f, 0.0f, 1.0f );
		break;
	}

	g_QglTable.m_pfn_qglLineWidth( 1.0f );
	g_QglTable.m_pfn_qglColor4f( 1.0f, 0.0f, 0.0f, 0.5f );

	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglDisable( GL_POLYGON_SMOOTH );

	g_QglTable.m_pfn_qglDepthFunc( GL_ALWAYS );

	//bleh
	list<DWinding *>::const_iterator l = m_list->begin();

	for (; l != m_list->end(); l++ )
	{
		DWinding* w = *l;

		g_QglTable.m_pfn_qglColor4f( w->clr[0], w->clr[1], w->clr[2], 0.5f );

		g_QglTable.m_pfn_qglBegin( GL_POLYGON );
		for ( int i = 0; i < w->numpoints; i++ ) {
			g_QglTable.m_pfn_qglVertex3f( ( w->p[i] )[0], ( w->p[i] )[1], ( w->p[i] )[2] );
		}
		g_QglTable.m_pfn_qglEnd();
	}


	g_QglTable.m_pfn_qglPopMatrix();

	g_QglTable.m_pfn_qglPopAttrib();
}

void DVisDrawer::Draw3D(){
	if ( !m_list ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglColor4f( 1.0, 0.0, 0.0, 0.5f );

//	g_QglTable.m_pfn_qglHint(GL_FOG_HINT, GL_NICEST);

//	g_QglTable.m_pfn_qglDisable(GL_CULL_FACE);
	g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );

//	g_QglTable.m_pfn_qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//	g_QglTable.m_pfn_qglShadeModel(GL_SMOOTH);

	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglDisable( GL_POLYGON_SMOOTH );

	g_QglTable.m_pfn_qglDepthFunc( GL_ALWAYS );

	//bleh
	list<DWinding *>::const_iterator l = m_list->begin();

	for (; l != m_list->end(); l++ )
	{
		DWinding* w = *l;

		g_QglTable.m_pfn_qglColor4f( w->clr[0], w->clr[1], w->clr[2], 0.5f );

		g_QglTable.m_pfn_qglBegin( GL_POLYGON );
		for ( int i = 0; i < w->numpoints; i++ ) {
			g_QglTable.m_pfn_qglVertex3f( ( w->p[i] )[0], ( w->p[i] )[1], ( w->p[i] )[2] );
		}
		g_QglTable.m_pfn_qglEnd();
	}

	g_QglTable.m_pfn_qglPopAttrib();
}

void DVisDrawer::Register(){
	g_QglTable.m_pfnHookGL2DWindow( this );
	g_QglTable.m_pfnHookGL3DWindow( this );
	m_bHooked = TRUE;
}

void DVisDrawer::UnRegister(){
	g_QglTable.m_pfnUnHookGL2DWindow( this );
	g_QglTable.m_pfnUnHookGL3DWindow( this );
	m_bHooked = FALSE;
}

void DVisDrawer::SetList( list<DWinding*> *pointList ){
	if ( m_list ) {
		ClearPoints();
	}

	m_list = pointList;
}

void DVisDrawer::ClearPoints(){
	list<DWinding *>::const_iterator deadPoint = m_list->begin();
	for (; deadPoint != m_list->end(); deadPoint++ )
		delete *deadPoint;
	m_list->clear();
}
