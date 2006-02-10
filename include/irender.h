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

#if !defined(INCLUDED_IRENDER_H)
#define INCLUDED_IRENDER_H

#include "generic/constant.h"


// Rendering states to sort by.
// Higher bits have the most effect - slowest state changes should be highest.

const unsigned int RENDER_DEFAULT = 0;
const unsigned int RENDER_LINESTIPPLE = 1 << 0; // glEnable(GL_LINE_STIPPLE)
const unsigned int RENDER_POLYGONSTIPPLE = 1 << 1; // glEnable(GL_POLYGON_STIPPLE)
const unsigned int RENDER_ALPHATEST = 1 << 2; // glEnable(GL_ALPHA_TEST)
const unsigned int RENDER_DEPTHTEST = 1 << 3; // glEnable(GL_DEPTH_TEST)
const unsigned int RENDER_DEPTHWRITE = 1 << 4; // glDepthMask(GL_TRUE)
const unsigned int RENDER_COLOURWRITE = 1 << 5; // glColorMask(GL_TRUE; GL_TRUE; GL_TRUE; GL_TRUE)
const unsigned int RENDER_CULLFACE = 1 << 6; // glglEnable(GL_CULL_FACE)
const unsigned int RENDER_SCALED = 1 << 7; // glEnable(GL_NORMALIZE)
const unsigned int RENDER_SMOOTH = 1 << 8; // glShadeModel
const unsigned int RENDER_LIGHTING = 1 << 9; // glEnable(GL_LIGHTING)
const unsigned int RENDER_BLEND = 1 << 10; // glEnable(GL_BLEND)
const unsigned int RENDER_OFFSETLINE = 1 << 11; // glEnable(GL_POLYGON_OFFSET_LINE)
const unsigned int RENDER_FILL = 1 << 12; // glPolygonMode
const unsigned int RENDER_COLOUR = 1 << 13; // glEnableClientState(GL_COLOR_ARRAY)
const unsigned int RENDER_TEXTURE = 1 << 14; // glEnable(GL_TEXTURE_2D)
const unsigned int RENDER_BUMP = 1 << 15;
const unsigned int RENDER_PROGRAM = 1 << 16;
const unsigned int RENDER_SCREEN = 1 << 17;
const unsigned int RENDER_OVERRIDE = 1 << 18;
typedef unsigned int RenderStateFlags;


class AABB;
class Matrix4;

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;

class Shader;

class RendererLight
{
public:
  virtual Shader* getShader() const = 0;
  virtual const AABB& aabb() const = 0;
  virtual bool testAABB(const AABB& other) const = 0;
  virtual const Matrix4& rotation() const = 0;
  virtual const Vector3& offset() const = 0;
  virtual const Vector3& colour() const = 0;
  virtual bool isProjected() const = 0;
  virtual const Matrix4& projection() const = 0;
};

class LightCullable
{
public:
  virtual bool testLight(const RendererLight& light) const = 0;
  virtual void insertLight(const RendererLight& light)
  {
  }
  virtual void clearLights()
  {
  }
};

template<typename FirstArgument>
class Callback1;

class Renderable;
typedef Callback1<const Renderable&> RenderableCallback;

typedef Callback1<const RendererLight&> RendererLightCallback;

class LightList
{
public:
  virtual void evaluateLights() const = 0;
  virtual void lightsChanged() const = 0;
  virtual void forEachLight(const RendererLightCallback& callback) const = 0;
};

const int c_attr_TexCoord0 = 1;
const int c_attr_Tangent = 3;
const int c_attr_Binormal = 4;

class OpenGLRenderable
{
public:
  virtual void render(RenderStateFlags state) const = 0;
};

class Matrix4;
struct qtexture_t;
class ModuleObserver;

#include "math/vector.h"

class Shader
{
public:
  virtual void addRenderable(const OpenGLRenderable& renderable, const Matrix4& modelview, const LightList* lights = 0) = 0;
  virtual void incrementUsed() = 0;
  virtual void decrementUsed() = 0;
  virtual void attach(ModuleObserver& observer) = 0;
  virtual void detach(ModuleObserver& observer) = 0;
  virtual qtexture_t& getTexture() const = 0;
  virtual unsigned int getFlags() const = 0;
};

class ShaderCache
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "renderstate");

  virtual Shader* capture(const char* name) = 0;
  virtual void release(const char* name) = 0;
  /*! Render all Shader objects. */
  virtual void render(RenderStateFlags globalstate, const Matrix4& modelview, const Matrix4& projection, const Vector3& viewer = Vector3(0, 0, 0)) = 0;

  virtual void realise() = 0;
  virtual void unrealise() = 0;

  virtual bool lightingSupported() const = 0;
  virtual bool useShaderLanguage() const = 0;

  virtual const LightList& attach(LightCullable& cullable) = 0;
  virtual void detach(LightCullable& cullable) = 0;
  virtual void changed(LightCullable& cullable) = 0;
  virtual void attach(RendererLight& light) = 0;
  virtual void detach(RendererLight& light) = 0;
  virtual void changed(RendererLight& light) = 0;

  virtual void attachRenderable(const Renderable& renderable) = 0;
  virtual void detachRenderable(const Renderable& renderable) = 0;
  virtual void forEachRenderable(const RenderableCallback& callback) const = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<ShaderCache> GlobalShaderCacheModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<ShaderCache> GlobalShaderCacheModuleRef;

inline ShaderCache& GlobalShaderCache()
{
  return GlobalShaderCacheModule::getTable();
}

#endif
