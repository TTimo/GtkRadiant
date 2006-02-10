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

#include "pluginapi.h"

#include "modulesystem.h"
#include "qerplugin.h"

#include "generic/callback.h"
#include "math/vector.h"

#include "gtkmisc.h"

#include "camwindow.h"

#include "mainframe.h"


// camera API
void QERApp_GetCamera( Vector3& origin, Vector3& angles )
{
  CamWnd& camwnd = *g_pParentWnd->GetCamWnd();
  origin = Camera_getOrigin(camwnd);
  angles = Camera_getAngles(camwnd);
}

void QERApp_SetCamera( const Vector3& origin, const Vector3& angles )
{
  CamWnd& camwnd = *g_pParentWnd->GetCamWnd();
  Camera_setOrigin(camwnd, origin);
  Camera_setAngles(camwnd, angles);
}

void QERApp_GetCamWindowExtents( int *x, int *y, int *width, int *height )
{
#if 0
  CamWnd* camwnd = g_pParentWnd->GetCamWnd();

  gtk_window_get_position(GTK_WINDOW(camwnd->m_window), x, y);

  *width = camwnd->Camera()->width;
  *height = camwnd->Camera()->height;
#endif
}

#include "icamera.h"

class CameraAPI
{
  _QERCameraTable m_camera;
public:
  typedef _QERCameraTable Type;
  STRING_CONSTANT(Name, "*");

  CameraAPI()
  {
    m_camera.m_pfnGetCamera = &QERApp_GetCamera;
    m_camera.m_pfnSetCamera = &QERApp_SetCamera;
    m_camera.m_pfnGetCamWindowExtents = &QERApp_GetCamWindowExtents;
  }
  _QERCameraTable* getTable()
  {
    return &m_camera;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<CameraAPI> CameraModule;
typedef Static<CameraModule> StaticCameraModule;
StaticRegisterModule staticRegisterCamera(StaticCameraModule::instance());


