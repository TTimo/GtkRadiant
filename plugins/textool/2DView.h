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
// a class to provide basic services for 2D view of a world
// window <-> local 2D space transforms
// snap to grid
// TODO: this one could be placed under an interface, and provided to the editor as a service

#ifndef _2DVIEW_H_
#define _2DVIEW_H_

class C2DView
{
enum      E2DViewState { View_Idle, View_Move } ViewState;
int m_xPosMove, m_yPosMove;
float m_MinsMove[2], m_MaxsMove[2];
qboolean m_bDoGrid;
float m_GridStep[2];
qboolean m_bPopup;
public:
RECT m_rect;
float m_Mins[2],m_Maxs[2],m_Center[2];
C2DView(){
	ViewState = View_Idle;
	m_bDoGrid = false;
	m_bPopup = false;
}
~C2DView() { }
void SetGrid( float xGridStep, float yGridStep )
{   m_bDoGrid = true; m_GridStep[0] = xGridStep; m_GridStep[1] = yGridStep; }

// get window coordinates for space coordinates
void WindowForSpace( int &x, int &y, const float c[2] );
void SpaceForWindow( float c[2], int x, int y );
void GridForWindow( float c[2], int x, int y );
qboolean DoesSelect( int x, int y, float c[2] );
void PreparePaint();

bool OnRButtonDown( int x, int y );
bool OnMouseMove( int x, int y );
bool OnRButtonUp( int x, int y );
bool OnKeyDown( char *s );

void ZoomIn();
void ZoomOut();
};

#endif
