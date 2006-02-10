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

#if !defined (INCLUDED_PATCHMANIP_H)
#define INCLUDED_PATCHMANIP_H

#include "string/stringfwd.h"

void Patch_registerCommands();
typedef struct _GtkToolbar GtkToolbar;
typedef struct _GtkMenu GtkMenu;
void Patch_constructToolbar(GtkToolbar* toolbar);
void Patch_constructMenu(GtkMenu* menu);

namespace scene
{
  class Graph;
}

void Scene_PatchSetShader_Selected(scene::Graph& graph, const char* name);
void Scene_PatchGetShader_Selected(scene::Graph& graph, CopiedString& name);
void Scene_PatchSelectByShader(scene::Graph& graph, const char* name);
void Scene_PatchFindReplaceShader(scene::Graph& graph, const char* find, const char* replace);
void Scene_PatchFindReplaceShader_Selected(scene::Graph& graph, const char* find, const char* replace);

void Scene_PatchCapTexture_Selected(scene::Graph& graph);
void Scene_PatchNaturalTexture_Selected(scene::Graph& graph);
void Scene_PatchTileTexture_Selected(scene::Graph& graph, float s, float t);

void PatchFilters_construct();

void PatchPreferences_construct();

void Patch_registerPreferencesPage();

class PatchCreator;
extern PatchCreator* g_patchCreator;

#endif
