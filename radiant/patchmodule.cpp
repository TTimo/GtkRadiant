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

#include "patchmodule.h"

#include "qerplugin.h"
#include "ipatch.h"

#include "patch.h"
#include "patchmanip.h"

namespace
{
  std::size_t g_patchModuleCount = 0;
}

void Patch_Construct(EPatchType type)
{
  if(++g_patchModuleCount != 1)
  {
    return;
  }

  PatchFilters_construct();

  PatchPreferences_construct();

  Patch_registerPreferencesPage();

  Patch::constructStatic(type);
  PatchInstance::constructStatic();

  if(type == ePatchTypeDoom3)
  {
    MAX_PATCH_WIDTH = MAX_PATCH_HEIGHT = 99;
  }
  else
  {
    MAX_PATCH_WIDTH = MAX_PATCH_HEIGHT = 15;
  }
}

void Patch_Destroy()
{
  if(--g_patchModuleCount != 0)
  {
    return;
  }

  Patch::destroyStatic();
  PatchInstance::destroyStatic();
}

class Quake3PatchCreator : public PatchCreator
{
public:
  scene::Node& createPatch()
  {
    return (new PatchNodeQuake3())->node();
  }
};

Quake3PatchCreator g_Quake3PatchCreator;

PatchCreator& GetQuake3PatchCreator()
{
  return g_Quake3PatchCreator;
}

class Doom3PatchCreator : public PatchCreator
{
public:
  scene::Node& createPatch()
  {
    return (new PatchNodeDoom3(true))->node();
  }
};

Doom3PatchCreator g_Doom3PatchCreator;

PatchCreator& GetDoom3PatchCreator()
{
  return g_Doom3PatchCreator;
}

class Doom3PatchDef2Creator : public PatchCreator
{
public:
  scene::Node& createPatch()
  {
    return (new PatchNodeDoom3())->node();
  }
};

Doom3PatchDef2Creator g_Doom3PatchDef2Creator;

PatchCreator& GetDoom3PatchDef2Creator()
{
  return g_Doom3PatchDef2Creator;
}

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class PatchDependencies :
  public GlobalRadiantModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalUndoModuleRef,
  public GlobalFilterModuleRef
{
};

class PatchQuake3API : public TypeSystemRef
{
  PatchCreator* m_patchquake3;
public:
  typedef PatchCreator Type;
  STRING_CONSTANT(Name, "quake3");

  PatchQuake3API()
  {
    Patch_Construct(ePatchTypeQuake3);
 
    m_patchquake3 = &GetQuake3PatchCreator();
    g_patchCreator = m_patchquake3;
  }
  ~PatchQuake3API()
  {
    Patch_Destroy();
  }
  PatchCreator* getTable()
  {
    return m_patchquake3;
  }
};

typedef SingletonModule<PatchQuake3API, PatchDependencies> PatchQuake3Module;
typedef Static<PatchQuake3Module> StaticPatchQuake3Module;
StaticRegisterModule staticRegisterPatchQuake3(StaticPatchQuake3Module::instance());



class PatchDoom3API : public TypeSystemRef
{
  PatchCreator* m_patchdoom3;
public:
  typedef PatchCreator Type;
  STRING_CONSTANT(Name, "doom3");

  PatchDoom3API()
  {
    Patch_Construct(ePatchTypeDoom3);

    m_patchdoom3 = &GetDoom3PatchCreator();
  }
  ~PatchDoom3API()
  {
    Patch_Destroy();
  }
  PatchCreator* getTable()
  {
    return m_patchdoom3;
  }
};

typedef SingletonModule<PatchDoom3API, PatchDependencies> PatchDoom3Module;
typedef Static<PatchDoom3Module> StaticPatchDoom3Module;
StaticRegisterModule staticRegisterPatchDoom3(StaticPatchDoom3Module::instance());


class PatchDef2Doom3API : public TypeSystemRef
{
  PatchCreator* m_patchdef2doom3;
public:
  typedef PatchCreator Type;
  STRING_CONSTANT(Name, "def2doom3");

  PatchDef2Doom3API()
  {
    Patch_Construct(ePatchTypeDoom3);

    m_patchdef2doom3 = &GetDoom3PatchDef2Creator();
    g_patchCreator = m_patchdef2doom3;
  }
  ~PatchDef2Doom3API()
  {
    Patch_Destroy();
  }
  PatchCreator* getTable()
  {
    return m_patchdef2doom3;
  }
};

typedef SingletonModule<PatchDef2Doom3API, PatchDependencies> PatchDef2Doom3Module;
typedef Static<PatchDef2Doom3Module> StaticPatchDef2Doom3Module;
StaticRegisterModule staticRegisterPatchDef2Doom3(StaticPatchDef2Doom3Module::instance());



