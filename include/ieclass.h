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

/// \file ieclass.h
/// \brief Entity Class definition loader API.


#if !defined(INCLUDED_IECLASS_H)
#define INCLUDED_IECLASS_H

#include "generic/constant.h"

#define	MAX_FLAGS	16

// eclass show flags

#define     ECLASS_LIGHT      0x00000001
#define     ECLASS_ANGLE      0x00000002
#define     ECLASS_PATH       0x00000004
#define     ECLASS_MISCMODEL  0x00000008

class Shader;

class EntityClass;
class ListAttributeType;

class EntityClassCollector
{
public:
  virtual void insert(EntityClass* eclass) = 0;
  virtual void insert(const char* name, const ListAttributeType& list)
  {
  }
};

struct EntityClassScanner
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "eclass");

  void (*scanFile)(EntityClassCollector& collector, const char* filename);
  const char* (*getExtension)();
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<EntityClassScanner> GlobalEClassModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<EntityClassScanner> GlobalEClassModuleRef;

inline EntityClassScanner& GlobalEClassLoader()
{
  return GlobalEClassModule::getTable();
}





class EntityClassVisitor
{
public:
  virtual void visit(EntityClass* eclass) = 0;
};

class ModuleObserver;


struct EntityClassManager
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "eclassmanager");

  EntityClass* (*findOrInsert)(const char* name, bool has_brushes);
  const ListAttributeType* (*findListType)(const char* name);
  void (*forEach)(EntityClassVisitor& visitor);
  void (*attach)(ModuleObserver& observer);
  void (*detach)(ModuleObserver& observer);
  void (*realise)();
  void (*unrealise)();
};

template<typename Type>
class GlobalModule;
typedef GlobalModule<EntityClassManager> GlobalEntityClassManagerModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<EntityClassManager> GlobalEntityClassManagerModuleRef;

inline EntityClassManager& GlobalEntityClassManager()
{
  return GlobalEntityClassManagerModule::getTable();
}

#endif
