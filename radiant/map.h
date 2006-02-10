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

#if !defined(INCLUDED_MAP_H)
#define INCLUDED_MAP_H

#include "iscenegraph.h"
#include "generic/callback.h"
#include "string/stringfwd.h"

class Map;
extern Map g_map;

class MapFormat;

void Map_addValidCallback(Map& map, const Callback& callback);
bool Map_Valid(const Map& map);

class DeferredDraw
{
  Callback m_draw;
  bool m_defer;
  bool m_deferred;
public:
  DeferredDraw(const Callback& draw) : m_draw(draw), m_defer(false), m_deferred(false)
  {
  }
  void defer()
  {
    m_defer = true;
  }
  void draw()
  {
    if(m_defer)
    {
      m_deferred = true;
    }
    else
    {
      m_draw();
    }
  }
  void flush()
  {
    if(m_defer && m_deferred)
    {
      m_draw();
    }
    m_deferred = false;
    m_defer = false;
  }
};

inline void DeferredDraw_onMapValidChanged(DeferredDraw& self)
{
  if(Map_Valid(g_map))
  {
    self.flush();
  }
  else
  {
    self.defer();
  }
}
typedef ReferenceCaller<DeferredDraw, DeferredDraw_onMapValidChanged> DeferredDrawOnMapValidChangedCaller;



const char* Map_Name(const Map& map);
const MapFormat& Map_getFormat(const Map& map);
bool Map_Unnamed(const Map& map);


namespace scene
{
  class Node;
  class Graph;
}

scene::Node* Map_GetWorldspawn(const Map& map);
scene::Node* Map_FindWorldspawn(Map& map);
scene::Node& Map_FindOrInsertWorldspawn(Map& map);

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;

extern Vector3 region_mins, region_maxs;
extern bool region_active;

// used to be #defines, multiple engine support suggests we should go towards dynamic
extern float g_MaxWorldCoord;
extern float g_MinWorldCoord;

void Map_LoadFile(const char* filename);
bool Map_SaveFile(const char* filename);

void Map_New();
void Map_Free();

void Map_RegionOff();

bool Map_SaveRegion(const char* filename);

class TextInputStream;
class TextOutputStream;

void Map_ImportSelected(TextInputStream& in, const MapFormat& format);
void Map_ExportSelected(TextOutputStream& out, const MapFormat& format);

bool Map_Modified(const Map& map);
void Map_SetModified(Map& map, bool modified);

bool Map_Save();
bool Map_SaveAs();

scene::Node& Node_Clone(scene::Node& node);

void DoMapInfo();

void Scene_parentSelectedBrushesToEntity(scene::Graph& graph, scene::Node& parent);
std::size_t Scene_countSelectedBrushes(scene::Graph& graph);

void Scene_parentSelected();

void OnUndoSizeChanged();

void NewMap();
void OpenMap();
void ImportMap();
void SaveMapAs();
void SaveMap();
void ExportMap();
void SaveRegion();


void Map_Traverse(scene::Node& root, const scene::Traversable::Walker& walker);


void SelectBrush (int entitynum, int brushnum);

extern CopiedString g_strLastMap;
extern bool g_bLoadLastMap;

void Map_Construct();
void Map_Destroy();


void Map_gatherNamespaced(scene::Node& root);
void Map_mergeClonedNames();

#endif
