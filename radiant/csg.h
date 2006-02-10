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

#if !defined(INCLUDED_CSG_H)
#define INCLUDED_CSG_H

void CSG_MakeHollow (void);
void CSG_Subtract (void);
void CSG_Merge (void);

namespace scene
{
  class Graph;
}
template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
class Plane3;

void Scene_BrushSetClipPlane(scene::Graph& graph, const Plane3& plane);
enum EBrushSplit
{
  eFront,
  eBack,
  eFrontAndBack,
};
void Scene_BrushSplitByPlane(scene::Graph& graph, const Vector3& p0, const Vector3& p1, const Vector3& p2, const char* shader, EBrushSplit split);

#endif
