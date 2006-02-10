/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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
// camera interface
//

#if !defined(INCLUDED_ICAMERA_H)
#define INCLUDED_ICAMERA_H

#include "generic/constant.h"

class Matrix4;

class CameraView
{
public:
  virtual void setModelview(const Matrix4& modelview) = 0;
  virtual void setFieldOfView(float fieldOfView) = 0;
};

class Callback;

class CameraModel
{
public:
  STRING_CONSTANT(Name, "CameraModel");
  virtual void setCameraView(CameraView* view, const Callback& disconnect) = 0;
};

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;

typedef void		(* PFN_GETCAMERA)		( Vector3& origin, Vector3& angles );
typedef void		(* PFN_SETCAMERA)		( const Vector3& origin, const Vector3& angles );
typedef void		(* PFN_GETCAMWINDOWEXTENTS)	( int *x, int *y, int *width, int *height );

struct _QERCameraTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "camera");

	PFN_GETCAMERA		m_pfnGetCamera;
	PFN_SETCAMERA		m_pfnSetCamera;
	PFN_GETCAMWINDOWEXTENTS	m_pfnGetCamWindowExtents;
};

#endif
