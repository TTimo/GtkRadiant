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

#if !defined(INCLUDED_PLUGINAPI_H)
#define INCLUDED_PLUGINAPI_H

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;

// camera API
void QERApp_GetCamera( Vector3& origin, Vector3& angles );
void QERApp_SetCamera( const Vector3& origin, const Vector3& angles );
void QERApp_GetCamWindowExtents( int *x, int *y, int *width, int *height );

#endif
