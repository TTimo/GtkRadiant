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

#if !defined(INCLUDED_NAMESPACE_H)
#define INCLUDED_NAMESPACE_H

#include "generic/constant.h"

template<typename FirstArgument>
class Callback1;
typedef Callback1<const char*> NameCallback;
typedef Callback1<const NameCallback&> NameCallbackCallback;

class Namespace
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "namespace");
  virtual void attach(const NameCallback& setName, const NameCallbackCallback& attachObserver) = 0;
  virtual void detach(const NameCallback& setName, const NameCallbackCallback& detachObserver) = 0;
  virtual void makeUnique(const char* name, const NameCallback& setName) const = 0;
};

class Namespaced
{
public:
  STRING_CONSTANT(Name, "Namespaced");

  virtual void setNamespace(Namespace& space) = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<Namespace> GlobalNamespaceModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<Namespace> GlobalNamespaceModuleRef;

inline Namespace& GlobalNamespace()
{
  return GlobalNamespaceModule::getTable();
}
#endif
