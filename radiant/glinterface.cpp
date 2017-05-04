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
//
// DESCRIPTION:
// Quick interface hack for selected face interface
// this one really needs more work, but I'm in a hurry with TexTool

#include "stdafx.h"
//#include "qe3.h"

// stores objects that want to be hooked into drawing in the XY window or Camera view
//++timo TODO: add support for Z view ... (texture view?)
CPtrArray l_GL2DWindows;
CPtrArray l_GL3DWindows;

void WINAPI QERApp_HookGL2DWindow( IGL2DWindow* pGLW ){
	l_GL2DWindows.Add( pGLW );
	pGLW->IncRef();
}

void WINAPI QERApp_UnHookGL2DWindow( IGL2DWindow* pGLW ){
	for ( int i = 0; i < l_GL2DWindows.GetSize(); i++ )
	{
		if ( l_GL2DWindows.GetAt( i ) == pGLW ) {
			l_GL2DWindows.RemoveAt( i );
			pGLW->DecRef();
			return;
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_ERR, "ERROR: IGL2DWindow* not found in QERApp_UnHookGL2DWindow\n" );
#endif
}

void Draw2DPluginEntities( VIEWTYPE vt ){
	for ( int i = 0; i < l_GL2DWindows.GetSize(); i++ )
		static_cast<IGL2DWindow*>( l_GL2DWindows.GetAt( i ) )->Draw2D( vt );
}

void WINAPI QERApp_HookGL3DWindow( IGL3DWindow* pGLW ){
	l_GL3DWindows.Add( pGLW );
	pGLW->IncRef();
}

void WINAPI QERApp_UnHookGL3DWindow( IGL3DWindow* pGLW ){
	for ( int i = 0; i < l_GL3DWindows.GetSize(); i++ )
	{
		if ( l_GL3DWindows.GetAt( i ) == pGLW ) {
			l_GL3DWindows.RemoveAt( i );
			pGLW->DecRef();
			return;
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_ERR, "ERROR: IGL3DWindow* not found in QERApp_UnHookGL3DWindow\n" );
#endif
}

void Draw3DPluginEntities(){
	for ( int i = 0; i < l_GL3DWindows.GetSize(); i++ )
		static_cast<IGL3DWindow*>( l_GL3DWindows.GetAt( i ) )->Draw3D();
}
