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

#if !defined(INCLUDED_ISHADERS_H)
#define INCLUDED_ISHADERS_H

#include "generic/constant.h"
#include "generic/callbackfwd.h"

enum
{
  QER_TRANS = 1 << 0,
  QER_NOCARVE = 1 << 1,
  QER_NODRAW = 1 << 2,
  QER_NONSOLID = 1 << 3,
  QER_WATER = 1 << 4,
  QER_LAVA = 1 << 5,
  QER_FOG = 1 << 6,
  QER_ALPHATEST = 1 << 7,
  QER_CULL = 1 << 8,
  QER_AREAPORTAL = 1 << 9,
  QER_CLIP = 1 << 10,
  QER_BOTCLIP = 1 << 11,
};

struct qtexture_t;

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
typedef Vector3 Colour3;

typedef unsigned char BlendFactor;
const BlendFactor BLEND_ZERO = 0;
const BlendFactor BLEND_ONE = 1;
const BlendFactor BLEND_SRC_COLOUR = 2;
const BlendFactor BLEND_ONE_MINUS_SRC_COLOUR = 3;
const BlendFactor BLEND_SRC_ALPHA = 4;
const BlendFactor BLEND_ONE_MINUS_SRC_ALPHA = 5;
const BlendFactor BLEND_DST_COLOUR = 6;
const BlendFactor BLEND_ONE_MINUS_DST_COLOUR = 7;
const BlendFactor BLEND_DST_ALPHA = 8;
const BlendFactor BLEND_ONE_MINUS_DST_ALPHA = 9;
const BlendFactor BLEND_SRC_ALPHA_SATURATE = 10;

class BlendFunc
{
public:
  BlendFunc(BlendFactor src, BlendFactor dst) : m_src(src), m_dst(dst)
  {
  }
  BlendFactor m_src;
  BlendFactor m_dst;
};

class ShaderLayer
{
public:
  virtual qtexture_t* texture() const = 0;
  virtual BlendFunc blendFunc() const = 0;
  virtual bool clampToBorder() const = 0;
  virtual float alphaTest() const = 0;
};

typedef Callback1<const ShaderLayer&> ShaderLayerCallback;


class IShader
{
public:
  enum EAlphaFunc
  {
    eAlways,
    eEqual,
    eLess,
    eGreater,
    eLEqual,
    eGEqual,
  };
  enum ECull
  {
    eCullNone,
    eCullBack,
  };
  // Increment the number of references to this object
  virtual void IncRef() = 0;
  // Decrement the reference count
  virtual void DecRef() = 0;
  // get/set the qtexture_t* Radiant uses to represent this shader object
  virtual qtexture_t* getTexture() const = 0;
  virtual qtexture_t* getDiffuse() const = 0;
  virtual qtexture_t* getBump() const = 0;
  virtual qtexture_t* getSpecular() const = 0;
  // get shader name
  virtual const char* getName() const = 0;
  virtual bool IsInUse() const = 0;
  virtual void SetInUse(bool bInUse) = 0;
  // get the editor flags (QER_NOCARVE QER_TRANS)
  virtual int getFlags() const = 0;
  // get the transparency value
  virtual float getTrans() const = 0;
  // test if it's a true shader, or a default shader created to wrap around a texture
  virtual bool IsDefault() const = 0;
  // get the alphaFunc
  virtual void getAlphaFunc(EAlphaFunc *func, float *ref) = 0;
  virtual BlendFunc getBlendFunc() const = 0;
  // get the cull type
  virtual ECull getCull() = 0;
  // get shader file name (ie the file where this one is defined)
  virtual const char* getShaderFileName() const = 0;

  virtual const ShaderLayer* firstLayer() const = 0;
  virtual void forEachLayer(const ShaderLayerCallback& layer) const = 0;

  virtual qtexture_t* lightFalloffImage() const = 0;
};

typedef struct _GSList GSList;
typedef Callback1<const char*> ShaderNameCallback;

class ModuleObserver;

class ShaderSystem
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "shaders");
  // NOTE: shader and texture names used must be full path.
  // Shaders usable as textures have prefix equal to getTexturePrefix()

  virtual void realise() = 0;
  virtual void unrealise() = 0;
  virtual void refresh() = 0;
  // activate the shader for a given name and return it
  // will return the default shader if name is not found
  virtual IShader* getShaderForName(const char* name) = 0;

  virtual void foreachShaderName(const ShaderNameCallback& callback) = 0;

  // iterate over the list of active shaders
  virtual void beginActiveShadersIterator() = 0;
  virtual bool endActiveShadersIterator() = 0;
  virtual IShader* dereferenceActiveShadersIterator() = 0;
  virtual void incrementActiveShadersIterator() = 0;

  virtual void setActiveShadersChangedNotify(const Callback& notify) = 0;

  virtual void attach(ModuleObserver& observer) = 0;
  virtual void detach(ModuleObserver& observer) = 0;

  virtual void setLightingEnabled(bool enabled) = 0;

  virtual const char* getTexturePrefix() const = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<ShaderSystem> GlobalShadersModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<ShaderSystem> GlobalShadersModuleRef;

inline ShaderSystem& GlobalShaderSystem()
{
  return GlobalShadersModule::getTable();
}


#define QERApp_Shader_ForName GlobalShaderSystem().getShaderForName
#define QERApp_ActiveShaders_IteratorBegin GlobalShaderSystem().beginActiveShadersIterator
#define QERApp_ActiveShaders_IteratorAtEnd GlobalShaderSystem().endActiveShadersIterator
#define QERApp_ActiveShaders_IteratorCurrent GlobalShaderSystem().dereferenceActiveShadersIterator
#define QERApp_ActiveShaders_IteratorIncrement GlobalShaderSystem().incrementActiveShadersIterator

#endif
