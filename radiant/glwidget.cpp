/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#include "glwidget.h"

#include "igtkgl.h"
#include "modulesystem.h"
#include "gtkutil/glwidget.h"

class GtkGLAPI
{
  _QERGtkGLTable m_gtkgl;
public:
  typedef _QERGtkGLTable Type;
  STRING_CONSTANT(Name, "*");

  GtkGLAPI()
  {
    m_gtkgl.glwidget_new = &glwidget_new;
    m_gtkgl.glwidget_swap_buffers = &glwidget_swap_buffers;
    m_gtkgl.glwidget_make_current = &glwidget_make_current;
    m_gtkgl.glwidget_destroy_context = &glwidget_destroy_context;
    m_gtkgl.glwidget_create_context = &glwidget_create_context;
  }
  _QERGtkGLTable* getTable()
  {
    return &m_gtkgl;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<GtkGLAPI> GtkGLModule;
typedef Static<GtkGLModule> StaticGtkGLModule;
StaticRegisterModule staticRegisterGtkGL(StaticGtkGLModule::instance());

