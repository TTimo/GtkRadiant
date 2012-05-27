
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
  Vector3 m_dragPlanesMin;
  Vector3 m_dragPlanesMax;
  Vector3 m_dragPlanesOrigin;
  Vector3 m_dragPlanesExtents;

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

    m_dragPlanesMin = aabb.origin - aabb.extents;
    m_dragPlanesMax = aabb.origin + aabb.extents;
    m_dragPlanesOrigin = aabb.origin;
    m_dragPlanesExtents = aabb.extents;
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
  void translate(const Vector3& translation)
  {
    if(m_dragPlanesExtents[0] != 0)
    {
      if(m_selectable_right.isSelected())
      {
        m_dragPlanesMax[0] += translation[0];
        //globalOutputStream() << "moving right\n";
      }
      if(m_selectable_left.isSelected())
      {
        m_dragPlanesMin[0] += translation[0];
        //globalOutputStream() << "moving left\n";
      }
    }
    if(m_dragPlanesExtents[1] != 0)
    {
      if(m_selectable_front.isSelected())
      {
        m_dragPlanesMax[1] += translation[1];
        //globalOutputStream() << "moving front\n";
      }
      if(m_selectable_back.isSelected())
      {
        m_dragPlanesMin[1] += translation[1];
        //globalOutputStream() << "moving back\n";
      }
    }
    if(m_dragPlanesExtents[2] != 0)
    {
      if(m_selectable_top.isSelected())
      {
        m_dragPlanesMax[2] += translation[2];
        //globalOutputStream() << "moving top\n";
      }
      if(m_selectable_bottom.isSelected())
      {
        m_dragPlanesMin[2] += translation[2];
        //globalOutputStream() << "moving bottom\n";
      }
    }
  }
  Matrix4 evaluateTransform() const
  {
    Vector3 originTransformed(vector3_mid(m_dragPlanesMin, m_dragPlanesMax));
    Vector3 scale(vector3_scaled(vector3_subtracted(m_dragPlanesMax, m_dragPlanesMin), 0.5));

    if(m_dragPlanesExtents[0] != 0)
    {
      scale[0] /= m_dragPlanesExtents[0];
    }
    else
    {
      scale[0] = 1;
    }
    if(m_dragPlanesExtents[1] != 0)
    {
      scale[1] /= m_dragPlanesExtents[1];
    }
    else
    {
      scale[1] = 1;
    }
    if(m_dragPlanesExtents[2] != 0)
    {
      scale[2] /= m_dragPlanesExtents[2];
    }
    else
    {
      scale[2] = 1;
    }

    Matrix4 matrix(matrix4_translation_for_vec3(originTransformed - m_dragPlanesOrigin));
    matrix4_pivoted_scale_by_vec3(matrix, scale, m_dragPlanesOrigin);

    return matrix;
  }
};

#endif
