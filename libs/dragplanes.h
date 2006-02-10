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

#if !defined(INCLUDED_DRAGPLANES_H)
#define INCLUDED_DRAGPLANES_H

#include "selectable.h"
#include "selectionlib.h"
#include "math/aabb.h"
#include "math/line.h"

class DragPlanes
{
public:
  ObservedSelectable m_selectable_right; // +x
  ObservedSelectable m_selectable_left; // -x
  ObservedSelectable m_selectable_front; // +y
  ObservedSelectable m_selectable_back; // -y
  ObservedSelectable m_selectable_top; // +z
  ObservedSelectable m_selectable_bottom; // -z
  AABB m_bounds;

  DragPlanes(const SelectionChangeCallback& onchanged) :
    m_selectable_right(onchanged),
    m_selectable_left(onchanged),
    m_selectable_front(onchanged),
    m_selectable_back(onchanged),
    m_selectable_top(onchanged),
    m_selectable_bottom(onchanged)
  {
  }
  bool isSelected() const
  {
    return m_selectable_right.isSelected()
      || m_selectable_left.isSelected()
      || m_selectable_front.isSelected()
      || m_selectable_back.isSelected()
      || m_selectable_top.isSelected()
      || m_selectable_bottom.isSelected();
  }
  void setSelected(bool selected)
  {
    m_selectable_right.setSelected(selected);
    m_selectable_left.setSelected(selected);
    m_selectable_front.setSelected(selected);
    m_selectable_back.setSelected(selected);
    m_selectable_top.setSelected(selected);
    m_selectable_bottom.setSelected(selected);
  }
  void selectPlanes(const AABB& aabb, Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback)
  {
    Line line(test.getNear(), test.getFar());
    Vector3 corners[8];
    aabb_corners(aabb, corners);
    Plane3 planes[6];
    aabb_planes(aabb, planes);

    for(Vector3* i = corners; i != corners + 8; ++i)
    {
      *i = vector3_subtracted(line_closest_point(line, *i), *i);
    }

    if(vector3_dot(planes[0].normal(), corners[1]) > 0
      && vector3_dot(planes[0].normal(), corners[2]) > 0
      && vector3_dot(planes[0].normal(), corners[5]) > 0
      && vector3_dot(planes[0].normal(), corners[6]) > 0)
    {
      Selector_add(selector, m_selectable_right);
      selectedPlaneCallback(planes[0]);
      //globalOutputStream() << "right\n";
    }
    if(vector3_dot(planes[1].normal(), corners[0]) > 0
      && vector3_dot(planes[1].normal(), corners[3]) > 0
      && vector3_dot(planes[1].normal(), corners[4]) > 0
      && vector3_dot(planes[1].normal(), corners[7]) > 0)
    {
      Selector_add(selector, m_selectable_left);
      selectedPlaneCallback(planes[1]);
      //globalOutputStream() << "left\n";
    }
    if(vector3_dot(planes[2].normal(), corners[0]) > 0
      && vector3_dot(planes[2].normal(), corners[1]) > 0
      && vector3_dot(planes[2].normal(), corners[4]) > 0
      && vector3_dot(planes[2].normal(), corners[5]) > 0)
    {
      Selector_add(selector, m_selectable_front);
      selectedPlaneCallback(planes[2]);
      //globalOutputStream() << "front\n";
    }
    if(vector3_dot(planes[3].normal(), corners[2]) > 0
      && vector3_dot(planes[3].normal(), corners[3]) > 0
      && vector3_dot(planes[3].normal(), corners[6]) > 0
      && vector3_dot(planes[3].normal(), corners[7]) > 0)
    {
      Selector_add(selector, m_selectable_back);
      selectedPlaneCallback(planes[3]);
      //globalOutputStream() << "back\n";
    }
    if(vector3_dot(planes[4].normal(), corners[0]) > 0
      && vector3_dot(planes[4].normal(), corners[1]) > 0
      && vector3_dot(planes[4].normal(), corners[2]) > 0
      && vector3_dot(planes[4].normal(), corners[3]) > 0)
    {
      Selector_add(selector, m_selectable_top);
      selectedPlaneCallback(planes[4]);
      //globalOutputStream() << "top\n";
    }
    if(vector3_dot(planes[5].normal(), corners[4]) > 0
      && vector3_dot(planes[5].normal(), corners[5]) > 0
      && vector3_dot(planes[5].normal(), corners[6]) > 0
      && vector3_dot(planes[5].normal(), corners[7]) > 0)
    {
      Selector_add(selector, m_selectable_bottom);
      //globalOutputStream() << "bottom\n";
      selectedPlaneCallback(planes[5]);
    }

    m_bounds = aabb;
  }
  void selectReversedPlanes(const AABB& aabb, Selector& selector, const SelectedPlanes& selectedPlanes)
  {
    Plane3 planes[6];
    aabb_planes(aabb, planes);

    if(selectedPlanes.contains(plane3_flipped(planes[0])))
    {
      Selector_add(selector, m_selectable_right);
    }
    if(selectedPlanes.contains(plane3_flipped(planes[1])))
    {
      Selector_add(selector, m_selectable_left);
    }
    if(selectedPlanes.contains(plane3_flipped(planes[2])))
    {
      Selector_add(selector, m_selectable_front);
    }
    if(selectedPlanes.contains(plane3_flipped(planes[3])))
    {
      Selector_add(selector, m_selectable_back);
    }
    if(selectedPlanes.contains(plane3_flipped(planes[4])))
    {
      Selector_add(selector, m_selectable_top);
    }
    if(selectedPlanes.contains(plane3_flipped(planes[5])))
    {
      Selector_add(selector, m_selectable_bottom);
    }
  }
  Matrix4 evaluateTransform(const Vector3& translation) const
  {
    Vector3 min = m_bounds.origin - m_bounds.extents;
    Vector3 max = m_bounds.origin + m_bounds.extents;
    Vector3 origin = m_bounds.origin;
    Vector3 extents = m_bounds.extents;
    if(extents[0] != 0)
    {
      if(m_selectable_right.isSelected())
      {
        max[0] += translation[0];
        //globalOutputStream() << "moving right\n";
      }
      if(m_selectable_left.isSelected())
      {
        min[0] += translation[0];
        //globalOutputStream() << "moving left\n";
      }
    }
    if(extents[1] != 0)
    {
      if(m_selectable_front.isSelected())
      {
        max[1] += translation[1];
        //globalOutputStream() << "moving front\n";
      }
      if(m_selectable_back.isSelected())
      {
        min[1] += translation[1];
        //globalOutputStream() << "moving back\n";
      }
    }
    if(extents[2] != 0)
    {
      if(m_selectable_top.isSelected())
      {
        max[2] += translation[2];
        //globalOutputStream() << "moving top\n";
      }
      if(m_selectable_bottom.isSelected())
      {
        min[2] += translation[2];
        //globalOutputStream() << "moving bottom\n";
      }
    }

    Vector3 originTransformed(vector3_mid(min, max));
    Vector3 scale(vector3_scaled(vector3_subtracted(max, min), 0.5));

    if(extents[0] != 0)
    {
      scale[0] /= extents[0];
    }
    else
    {
      scale[0] = 1;
    }
    if(extents[1] != 0)
    {
      scale[1] /= extents[1];
    }
    else
    {
      scale[1] = 1;
    }
    if(extents[2] != 0)
    {
      scale[2] /= extents[2];
    }
    else
    {
      scale[2] = 1;
    }

    Matrix4 matrix(matrix4_translation_for_vec3(originTransformed - origin));
    matrix4_pivoted_scale_by_vec3(matrix, scale, origin);

    return matrix;
  }
};

#endif
