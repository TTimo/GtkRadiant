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
// Quick interface hack for selected face interface
// this one really needs more work, but I'm in a hurry with TexTool

#ifndef __ISELECTEDFACE_H_
#define __ISELECTEDFACE_H_

#define SELECTEDFACE_MAJOR "selectedface"
// v2.0
// support for multiple faces selection (first use in textool v2)
// using the g_ptrSelectedFaces indexes, get the face_t* with GETFACE
// still relies on the _QERFaceData*, unless you cast the face_t* to do your own stuff
// removed PFN_TEXTUREFORNAME, it's in the IShaders API now

//++timo TODO: this interface needs some cleanup with the new texture / shaders interface

// number of selected textures
typedef int ( WINAPI * PFN_GETSELECTEDFACECOUNT )();
// retrieve the corresponding brush_t* (we need it when we need to explicitely rebuild stuff)
typedef brush_t*    ( WINAPI * PFN_GETFACEBRUSH )( int iface );
// retrieve a given face_t*
typedef face_t*     ( WINAPI * PFN_GETFACE )( int iface );
// winding_t is assumed to have MAX_POINTS_ON_WINDING allocated and waiting
typedef int ( WINAPI * PFN_GETFACEINFO )( int iface, _QERFaceData*, winding_t* );
// tell editor to update the selected face data
typedef int ( WINAPI * PFN_SETFACEINFO )( int iface, _QERFaceData* );
// retrieve the texture number to bind to
typedef int ( WINAPI * PFN_GETTEXTURENUMBER )( int iface );
// retrieving some texture information
typedef void ( WINAPI * PFN_GETTEXTURESIZE )( int iface, int Size[2] );
// straight func pointer to Select_SetTexture
// last parameter must be casted to an IPluginTexdef
typedef void ( WINAPI * PFN_SELECT_SETTEXTURE )( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, void* pPlugTexdef );

// NOTE: some things in there are not really related to the selected face
// having some stuff moved into a textures-dedicated part ?
struct _QERSelectedFaceTable
{
	int m_nSize;
	PFN_GETSELECTEDFACECOUNT m_pfnGetSelectedFaceCount;
	PFN_GETFACEBRUSH m_pfnGetFaceBrush;
	PFN_GETFACE m_pfnGetFace;
	PFN_GETFACEINFO m_pfnGetFaceInfo;
	PFN_SETFACEINFO m_pfnSetFaceInfo;
	PFN_GETTEXTURENUMBER m_pfnGetTextureNumber;
	PFN_GETTEXTURESIZE m_pfnGetTextureSize;
	PFN_SELECT_SETTEXTURE m_pfnSelect_SetTexture;
};

#ifdef USE_SELECTEDFACETABLE_DEFINE
  #ifndef __SELECTEDFACETABLENAME
	#define __SELECTEDFACETABLENAME g_SelectedFaceTable
  #endif

  #define GetSelectedFaceCount __SELECTEDFACETABLENAME.m_pfnGetSelectedFaceCount
  #define GetFaceBrush __SELECTEDFACETABLENAME.m_pfnGetFaceBrush
  #define GetFace __SELECTEDFACETABLENAME.m_pfnGetFace
  #define GetFaceInfo __SELECTEDFACETABLENAME.m_pfnGetFaceInfo
  #define SetFaceInfo __SELECTEDFACETABLENAME.m_pfnSetFaceInfo
  #define GetTextureNumber __SELECTEDFACETABLENAME.m_pfnGetTextureNumber
  #define GetTextureSize __SELECTEDFACETABLENAME.m_pfnGetTextureSize
  #define Select_SetTexture __SELECTEDFACETABLENAME.m_pfnSelect_SetTexture
#endif

#endif
