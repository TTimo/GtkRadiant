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

#if !defined(INCLUDED_IPLUGIN_H)
#define INCLUDED_IPLUGIN_H

#include "generic/constant.h"

typedef const char* (* PFN_QERPLUG_INIT) (void* hApp, void* pMainWidget);
typedef const char* (* PFN_QERPLUG_GETNAME)();
typedef const char* (* PFN_QERPLUG_GETCOMMANDLIST)();
typedef const char* (* PFN_QERPLUG_GETCOMMANDTITLELIST)();
typedef void        (* PFN_QERPLUG_DISPATCH) (const char* p, float* vMin, float* vMax, bool bSingleBrush);

struct _QERPluginTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "plugin");

  PFN_QERPLUG_INIT m_pfnQERPlug_Init;
  PFN_QERPLUG_GETNAME m_pfnQERPlug_GetName;
  PFN_QERPLUG_GETCOMMANDLIST m_pfnQERPlug_GetCommandList;
  PFN_QERPLUG_GETCOMMANDTITLELIST m_pfnQERPlug_GetCommandTitleList;
  PFN_QERPLUG_DISPATCH m_pfnQERPlug_Dispatch;
};

template<typename Type>
class Modules;
typedef Modules<_QERPluginTable> PluginModules;

template<typename Type>
class ModulesRef;
typedef ModulesRef<_QERPluginTable> PluginModulesRef;

#endif
