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

#if !defined(INCLUDED_ITEXTURES_H)
#define INCLUDED_ITEXTURES_H

#include "iimage.h"
#include "generic/constant.h"

struct qtexture_t;

class LoadImageCallback
{
  typedef Image* (*LoadFunc)(void* environment, const char* name);
public:
  void* m_environment;
  LoadFunc m_func;

  LoadImageCallback(void* environment, LoadFunc func) : m_environment(environment), m_func(func)
  {
  }
  Image* loadImage(const char* name) const
  {
    return m_func(m_environment, name);
  }
};

inline bool operator==(const LoadImageCallback& self, const LoadImageCallback& other)
{
  return self.m_environment == other.m_environment && self.m_func == other.m_func; 
}
inline bool operator<(const LoadImageCallback& self, const LoadImageCallback& other)
{
  return self.m_environment < other.m_environment || 
        (!(other.m_environment < self.m_environment) && self.m_func < other.m_func); 
}

class TexturesCacheObserver
{
public:
  virtual void unrealise() = 0;
  virtual void realise() = 0;
};

class TexturesCache
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "textures");
  virtual LoadImageCallback defaultLoader() const = 0;
  virtual Image* loadImage(const char* name) = 0;
  virtual qtexture_t* capture(const char* name) = 0;
  virtual qtexture_t* capture(const LoadImageCallback& load, const char* name) = 0;
  virtual void release(qtexture_t* texture) = 0;
  virtual void attach(TexturesCacheObserver& observer) = 0;
  virtual void detach(TexturesCacheObserver& observer) = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<TexturesCache> GlobalTexturesModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<TexturesCache> GlobalTexturesModuleRef;

inline TexturesCache& GlobalTexturesCache()
{
  return GlobalTexturesModule::getTable();
}

#endif
