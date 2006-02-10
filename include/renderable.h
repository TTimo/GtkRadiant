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

#if !defined(INCLUDED_RENDERABLE_H)
#define INCLUDED_RENDERABLE_H

#include "generic/constant.h"

class Shader;
class OpenGLRenderable;
class LightList;
class Matrix4;

class Renderer
{
public:
  enum EHighlightMode
  {
    eFace = 1 << 0,
    /*! Full highlighting. */
    ePrimitive = 1 << 1,
  };

  enum EStyle
  {
    eWireframeOnly,
    eFullMaterials,
  };

  virtual void PushState() = 0;
  virtual void PopState() = 0;
  virtual void SetState(Shader* state, EStyle mode) = 0;
  virtual const EStyle getStyle() const = 0;
  virtual void Highlight(EHighlightMode mode, bool bEnable = true) = 0;
  virtual void setLights(const LightList& lights)
  {
  }
  virtual void addRenderable(const OpenGLRenderable& renderable, const Matrix4& world) = 0;
};

class VolumeTest;

class Renderable
{
public:
  STRING_CONSTANT(Name, "Renderable");

  virtual void renderSolid(Renderer& renderer, const VolumeTest& volume) const = 0;
  virtual void renderWireframe(Renderer& renderer, const VolumeTest& volume) const = 0;
  virtual void renderComponents(Renderer&, const VolumeTest&) const
  {
  }
  virtual void viewChanged() const
  {
  }
};

#endif
