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

#if !defined(INCLUDED_SELECTION_H)
#define INCLUDED_SELECTION_H

#include "windowobserver.h"

struct rect_t
{
  float min[2];
  float max[2];
};

template<typename FirstArgument>
class Callback1;
typedef Callback1<rect_t> RectangleCallback;

class View;
class Callback;

class SelectionSystemWindowObserver : public WindowObserver
{
public:
  virtual void setView(const View& view) = 0;
  virtual void setRectangleDrawCallback(const RectangleCallback& callback) = 0;
};

SelectionSystemWindowObserver* NewWindowObserver();

class AABB;
namespace scene
{
  class Graph;
}
void Scene_BoundsSelected(scene::Graph& graph, AABB& bounds);

#endif
