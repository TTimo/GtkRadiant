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

#include "select.h"

#include "debugging/debugging.h"

#include "ientity.h"
#include "iselection.h"
#include "iundo.h"

#include <vector>

#include "stream/stringstream.h"
#include "signal/isignal.h"
#include "shaderlib.h"
#include "scenelib.h"

#include "gtkutil/idledraw.h"
#include "gtkutil/dialog.h"
#include "gtkutil/widget.h"
#include "brushmanip.h"
#include "brush.h"
#include "patchmanip.h"
#include "patchdialog.h"
#include "selection.h"
#include "texwindow.h"
#include "gtkmisc.h"
#include "mainframe.h"
#include "grid.h"
#include "map.h"



select_workzone_t g_select_workzone;


/**
  Loops over all selected brushes and stores their
  world AABBs in the specified array.
*/
class CollectSelectedBrushesBounds : public SelectionSystem::Visitor
{
  AABB* m_bounds;   // array of AABBs
  Unsigned m_max;   // max AABB-elements in array
  Unsigned& m_count;// count of valid AABBs stored in array

public:
  CollectSelectedBrushesBounds(AABB* bounds, Unsigned max, Unsigned& count)
    : m_bounds(bounds),
      m_max(max),
      m_count(count)
  {
    m_count = 0;
  }

  void visit(scene::Instance& instance) const
  {
    ASSERT_MESSAGE(m_count <= m_max, "Invalid m_count in CollectSelectedBrushesBounds");

    // stop if the array is already full
    if(m_count == m_max)
      return;

    Selectable* selectable = Instance_getSelectable(instance);
    if((selectable != 0)
      && instance.isSelected())
    {
      // brushes only
      if(Instance_getBrush(instance) != 0)
      {
        m_bounds[m_count] = instance.worldAABB();
        ++m_count;
      }
    }
  }
};

/**
  Selects all objects that intersect one of the bounding AABBs.
  The exact intersection-method is specified through TSelectionPolicy
*/
template<class TSelectionPolicy>
class SelectByBounds : public scene::Graph::Walker
{
  AABB* m_aabbs;           // selection aabbs
  Unsigned m_count;        // number of aabbs in m_aabbs
  TSelectionPolicy policy; // type that contains a custom intersection method aabb<->aabb

public:
  SelectByBounds(AABB* aabbs, Unsigned count)
      : m_aabbs(aabbs),
        m_count(count)
  {
  }

  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);

    // ignore worldspawn
    Entity* entity = Node_getEntity(path.top());
    if(entity)
    {
      if(string_equal(entity->getKeyValue("classname"), "worldspawn"))
        return true;
    }
    
    if( (path.size() > 1) &&
        (!path.top().get().isRoot()) &&
        (selectable != 0)
       )
    {
      for(Unsigned i = 0; i < m_count; ++i)
      {
        if(policy.Evaluate(m_aabbs[i], instance))
        {
          selectable->setSelected(true);
        }
      }
    }

    return true;
  }

  /**
    Performs selection operation on the global scenegraph.
    If delete_bounds_src is true, then the objects which were
    used as source for the selection aabbs will be deleted.
*/
  static void DoSelection(bool delete_bounds_src = true)
  {
    if(GlobalSelectionSystem().Mode() == SelectionSystem::ePrimitive)
    {
      // we may not need all AABBs since not all selected objects have to be brushes
      const Unsigned max = (Unsigned)GlobalSelectionSystem().countSelected();
      AABB* aabbs = new AABB[max];
            
      Unsigned count;
      CollectSelectedBrushesBounds collector(aabbs, max, count);
      GlobalSelectionSystem().foreachSelected(collector);

      // nothing usable in selection
      if(!count)
      {
        delete[] aabbs;
        return;
      }
      
      // delete selected objects
      if(delete_bounds_src)// see deleteSelection
      {
        UndoableCommand undo("deleteSelected");
        Select_Delete();
      }

      // select objects with bounds
      GlobalSceneGraph().traverse(SelectByBounds<TSelectionPolicy>(aabbs, count));
      
      SceneChangeNotify();
      delete[] aabbs;
    }
  }
};

/**
  SelectionPolicy for SelectByBounds
  Returns true if box and the AABB of instance intersect
*/
class SelectionPolicy_Touching
{
public:
  bool Evaluate(const AABB& box, scene::Instance& instance) const
  {
    const AABB& other(instance.worldAABB());
    for(Unsigned i = 0; i < 3; ++i)
    {
      if(fabsf(box.origin[i] - other.origin[i]) > (box.extents[i] + other.extents[i]))
        return false;
    }
    return true;
  }
};

/**
  SelectionPolicy for SelectByBounds
  Returns true if the AABB of instance is inside box
*/
class SelectionPolicy_Inside
{
public:
  bool Evaluate(const AABB& box, scene::Instance& instance) const
  {
    const AABB& other(instance.worldAABB());
    for(Unsigned i = 0; i < 3; ++i)
    {
      if(fabsf(box.origin[i] - other.origin[i]) > (box.extents[i] - other.extents[i]))
        return false;
    }
    return true;
  }
};

class DeleteSelected : public scene::Graph::Walker
{
  mutable bool m_remove;
  mutable bool m_removedChild;
public:
  DeleteSelected()
    : m_remove(false), m_removedChild(false)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    m_removedChild = false;

    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && selectable->isSelected()
      && path.size() > 1
      && !path.top().get().isRoot())
    {
      m_remove = true;

      return false;// dont traverse into child elements
    }
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    
    if(m_removedChild)
    {
      m_removedChild = false;

      // delete empty entities
      Entity* entity = Node_getEntity(path.top());
      if(entity != 0
        && path.top().get_pointer() != Map_FindWorldspawn(g_map)
        && Node_getTraversable(path.top())->empty())
      {
        Path_deleteTop(path);
      }
    }

	// node should be removed
    if(m_remove)
    {
      if(Node_isEntity(path.parent()) != 0)
      {
        m_removedChild = true;
      }

      m_remove = false;
      Path_deleteTop(path);
    }
  }
};

void Scene_DeleteSelected(scene::Graph& graph)
{
  graph.traverse(DeleteSelected());
  SceneChangeNotify();
}

void Select_Delete (void)
{
  Scene_DeleteSelected(GlobalSceneGraph());
}

class InvertSelectionWalker : public scene::Graph::Walker
{
  SelectionSystem::EMode m_mode;
  mutable Selectable* m_selectable;
public:
  InvertSelectionWalker(SelectionSystem::EMode mode)
    : m_mode(mode), m_selectable(0)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable)
    {
      switch(m_mode)
      {
      case SelectionSystem::eEntity:
        if(Node_isEntity(path.top()) != 0)
        {
          m_selectable = path.top().get().visible() ? selectable : 0;
        }
        break;
      case SelectionSystem::ePrimitive:
        m_selectable = path.top().get().visible() ? selectable : 0;
        break;
      case SelectionSystem::eComponent:
        break;
      }
    }
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    if(m_selectable != 0)
    {
      m_selectable->setSelected(!m_selectable->isSelected());
      m_selectable = 0;
    }
  }
};

void Scene_Invert_Selection(scene::Graph& graph)
{
  graph.traverse(InvertSelectionWalker(GlobalSelectionSystem().Mode()));
}

void Select_Invert()
{
  Scene_Invert_Selection(GlobalSceneGraph());
}

class ExpandSelectionToEntitiesWalker : public scene::Graph::Walker
{
  mutable std::size_t m_depth;
public:
  ExpandSelectionToEntitiesWalker() : m_depth(0)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    ++m_depth;
    if(m_depth == 2) // entity depth
    {
      // traverse and select children if any one is selected
      return Node_getEntity(path.top())->isContainer() && instance.childSelected();
    }
    else if(m_depth == 3) // primitive depth
    {
      Instance_setSelected(instance, true);
      return false;
    }
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    --m_depth;
  }
};

void Scene_ExpandSelectionToEntities()
{
  GlobalSceneGraph().traverse(ExpandSelectionToEntitiesWalker());
}


namespace
{
  void Selection_UpdateWorkzone()
  {
    if(GlobalSelectionSystem().countSelected() != 0)
    {
      Select_GetBounds(g_select_workzone.d_work_min, g_select_workzone.d_work_max);
    }
  }
  typedef FreeCaller<Selection_UpdateWorkzone> SelectionUpdateWorkzoneCaller;

  IdleDraw g_idleWorkzone = IdleDraw(SelectionUpdateWorkzoneCaller());
}

const select_workzone_t& Select_getWorkZone()
{
  g_idleWorkzone.flush();
  return g_select_workzone;
}

void UpdateWorkzone_ForSelection()
{
  g_idleWorkzone.queueDraw();
}

// update the workzone to the current selection
void UpdateWorkzone_ForSelectionChanged(const Selectable& selectable)
{
  if(selectable.isSelected())
  {
    UpdateWorkzone_ForSelection();
  }
}

void Select_SetShader(const char* shader)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushSetShader_Selected(GlobalSceneGraph(), shader);
    Scene_PatchSetShader_Selected(GlobalSceneGraph(), shader);
  }
  Scene_BrushSetShader_Component_Selected(GlobalSceneGraph(), shader);
}

void Select_SetTexdef(const TextureProjection& projection)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushSetTexdef_Selected(GlobalSceneGraph(), projection);
  }
  Scene_BrushSetTexdef_Component_Selected(GlobalSceneGraph(), projection);
}

void Select_SetFlags(const ContentsFlagsValue& flags)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushSetFlags_Selected(GlobalSceneGraph(), flags);
  }
  Scene_BrushSetFlags_Component_Selected(GlobalSceneGraph(), flags);
}

void Select_GetBounds (Vector3& mins, Vector3& maxs)
{
  AABB bounds;
  Scene_BoundsSelected(GlobalSceneGraph(), bounds);
  maxs = vector3_added(bounds.origin, bounds.extents);
  mins = vector3_subtracted(bounds.origin, bounds.extents);
}

void Select_GetMid (Vector3& mid)
{
  AABB bounds;
  Scene_BoundsSelected(GlobalSceneGraph(), bounds);
  mid = vector3_snapped(bounds.origin);
}


void Select_FlipAxis (int axis)
{
  Vector3 flip(1, 1, 1);
  flip[axis] = -1;
  GlobalSelectionSystem().scaleSelected(flip);
}


void Select_Scale(float x, float y, float z)
{
  GlobalSelectionSystem().scaleSelected(Vector3(x, y, z));
}

enum axis_t
{
  eAxisX = 0,
  eAxisY = 1,
  eAxisZ = 2,
};

enum sign_t
{
  eSignPositive = 1,
  eSignNegative = -1,
};

inline Matrix4 matrix4_rotation_for_axis90(axis_t axis, sign_t sign)
{
  switch(axis)
  {
  case eAxisX:
    if(sign == eSignPositive)
    {
      return matrix4_rotation_for_sincos_x(1, 0);
    }
    else
    {
      return matrix4_rotation_for_sincos_x(-1, 0);
    }
  case eAxisY:
    if(sign == eSignPositive)
    {
      return matrix4_rotation_for_sincos_y(1, 0);
    }
    else
    {
      return matrix4_rotation_for_sincos_y(-1, 0);
    }
  default://case eAxisZ:
    if(sign == eSignPositive)
    {
      return matrix4_rotation_for_sincos_z(1, 0);
    }
    else
    {
      return matrix4_rotation_for_sincos_z(-1, 0);
    }
  }
}

inline void matrix4_rotate_by_axis90(Matrix4& matrix, axis_t axis, sign_t sign)
{
  matrix4_multiply_by_matrix4(matrix, matrix4_rotation_for_axis90(axis, sign));
}

inline void matrix4_pivoted_rotate_by_axis90(Matrix4& matrix, axis_t axis, sign_t sign, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(matrix, pivotpoint);
  matrix4_rotate_by_axis90(matrix, axis, sign);
  matrix4_translate_by_vec3(matrix, vector3_negated(pivotpoint));
}

inline Quaternion quaternion_for_axis90(axis_t axis, sign_t sign)
{
#if 1
  switch(axis)
  {
  case eAxisX:
    if(sign == eSignPositive)
    {
      return Quaternion(c_half_sqrt2f, 0, 0, c_half_sqrt2f);
    }
    else
    {
      return Quaternion(-c_half_sqrt2f, 0, 0, -c_half_sqrt2f);
    }
  case eAxisY:
    if(sign == eSignPositive)
    {
      return Quaternion(0, c_half_sqrt2f, 0, c_half_sqrt2f);
    }
    else
    {
      return Quaternion(0, -c_half_sqrt2f, 0, -c_half_sqrt2f);
    }
  default://case eAxisZ:
    if(sign == eSignPositive)
    {
      return Quaternion(0, 0, c_half_sqrt2f, c_half_sqrt2f);
    }
    else
    {
      return Quaternion(0, 0, -c_half_sqrt2f, -c_half_sqrt2f);
    }
  }
#else
  quaternion_for_matrix4_rotation(matrix4_rotation_for_axis90((axis_t)axis, (deg > 0) ? eSignPositive : eSignNegative));
#endif
}

void Select_RotateAxis (int axis, float deg)
{
  if(fabs(deg) == 90.f)
  {
    GlobalSelectionSystem().rotateSelected(quaternion_for_axis90((axis_t)axis, (deg > 0) ? eSignPositive : eSignNegative));
  }
  else
  {
    switch(axis)
    {
    case 0:
      GlobalSelectionSystem().rotateSelected(quaternion_for_matrix4_rotation(matrix4_rotation_for_x_degrees(deg)));
      break;
    case 1:
      GlobalSelectionSystem().rotateSelected(quaternion_for_matrix4_rotation(matrix4_rotation_for_y_degrees(deg)));
      break;
    case 2:
      GlobalSelectionSystem().rotateSelected(quaternion_for_matrix4_rotation(matrix4_rotation_for_z_degrees(deg)));
      break;
    }
  }
}


void Select_ShiftTexture(float x, float y)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushShiftTexdef_Selected(GlobalSceneGraph(), x, y);
    Scene_PatchTranslateTexture_Selected(GlobalSceneGraph(), x, y);
  }
  //globalOutputStream() << "shift selected face textures: s=" << x << " t=" << y << '\n';
  Scene_BrushShiftTexdef_Component_Selected(GlobalSceneGraph(), x, y);
}

void Select_ScaleTexture(float x, float y)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushScaleTexdef_Selected(GlobalSceneGraph(), x, y);
    Scene_PatchScaleTexture_Selected(GlobalSceneGraph(), x, y);
  }
  Scene_BrushScaleTexdef_Component_Selected(GlobalSceneGraph(), x, y);
}

void Select_RotateTexture(float amt)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushRotateTexdef_Selected(GlobalSceneGraph(), amt);
    Scene_PatchRotateTexture_Selected(GlobalSceneGraph(), amt);
  }
  Scene_BrushRotateTexdef_Component_Selected(GlobalSceneGraph(), amt);
}

// TTimo modified to handle shader architecture:
// expects shader names at input, comparison relies on shader names .. texture names no longer relevant
void FindReplaceTextures(const char* pFind, const char* pReplace, bool bSelected)
{
  if(!texdef_name_valid(pFind))
  {
    globalErrorStream() << "FindReplaceTextures: invalid texture name: '" << pFind << "', aborted\n";
    return;
  }
  if(!texdef_name_valid(pReplace))
  {
    globalErrorStream() << "FindReplaceTextures: invalid texture name: '" << pReplace << "', aborted\n";
    return;
  }

  StringOutputStream command;
  command << "textureFindReplace -find " << pFind << " -replace " << pReplace;
  UndoableCommand undo(command.c_str());

  if(bSelected)
  {
    if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
    {
      Scene_BrushFindReplaceShader_Selected(GlobalSceneGraph(), pFind, pReplace);
      Scene_PatchFindReplaceShader_Selected(GlobalSceneGraph(), pFind, pReplace);
    }
    Scene_BrushFindReplaceShader_Component_Selected(GlobalSceneGraph(), pFind, pReplace);
  }
  else
  {
    Scene_BrushFindReplaceShader(GlobalSceneGraph(), pFind, pReplace);
    Scene_PatchFindReplaceShader(GlobalSceneGraph(), pFind, pReplace);
  }
}

typedef std::vector<const char*> Classnames;

bool classnames_match_entity(const Classnames& classnames, Entity* entity)
{
  for(Classnames::const_iterator i = classnames.begin(); i != classnames.end(); ++i)
  {
    if(string_equal(entity->getKeyValue("classname"), *i))
    {
      return true;
    }
  }
  return false;
}

class EntityFindByClassnameWalker : public scene::Graph::Walker
{
  const Classnames& m_classnames;
public:
  EntityFindByClassnameWalker(const Classnames& classnames)
    : m_classnames(classnames)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Entity* entity = Node_getEntity(path.top());
    if(entity != 0
      && classnames_match_entity(m_classnames, entity))
    {
      Instance_getSelectable(instance)->setSelected(true);
    }
    return true;
  }
};

void Scene_EntitySelectByClassnames(scene::Graph& graph, const Classnames& classnames)
{
  graph.traverse(EntityFindByClassnameWalker(classnames));
}

class EntityGetSelectedClassnamesWalker : public scene::Graph::Walker
{
  Classnames& m_classnames;
public:
  EntityGetSelectedClassnamesWalker(Classnames& classnames)
    : m_classnames(classnames)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && selectable->isSelected())
    {
      Entity* entity = Node_getEntity(path.top());
      if(entity != 0)
      {
        m_classnames.push_back(entity->getKeyValue("classname"));
      }
    }
    return true;
  }
};

void Scene_EntityGetClassnames(scene::Graph& graph, Classnames& classnames)
{
  graph.traverse(EntityGetSelectedClassnamesWalker(classnames));
}

void Select_AllOfType()
{
  if(GlobalSelectionSystem().Mode() == SelectionSystem::eComponent)
  {
    if(GlobalSelectionSystem().ComponentMode() == SelectionSystem::eFace)
    {
      GlobalSelectionSystem().setSelectedAllComponents(false);
      Scene_BrushSelectByShader_Component(GlobalSceneGraph(), TextureBrowser_GetSelectedShader(GlobalTextureBrowser()));
    }
  }
  else
  {
    Classnames classnames;
    Scene_EntityGetClassnames(GlobalSceneGraph(), classnames);
    GlobalSelectionSystem().setSelectedAll(false);
    if(!classnames.empty())
    {
      Scene_EntitySelectByClassnames(GlobalSceneGraph(), classnames);
    }
    else
    {
      Scene_BrushSelectByShader(GlobalSceneGraph(), TextureBrowser_GetSelectedShader(GlobalTextureBrowser()));
      Scene_PatchSelectByShader(GlobalSceneGraph(), TextureBrowser_GetSelectedShader(GlobalTextureBrowser()));
    }
  }
}

void Select_Inside(void)
{
	SelectByBounds<SelectionPolicy_Inside>::DoSelection();
}

void Select_Touching(void)
{
	SelectByBounds<SelectionPolicy_Touching>::DoSelection(false);
}

void Select_FitTexture(float horizontal, float vertical)
{
  if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
  {
    Scene_BrushFitTexture_Selected(GlobalSceneGraph(), horizontal, vertical);
  }
  Scene_BrushFitTexture_Component_Selected(GlobalSceneGraph(), horizontal, vertical);

  SceneChangeNotify();
}

inline void hide_node(scene::Node& node, bool hide)
{
  hide
    ? node.enable(scene::Node::eHidden)
    : node.disable(scene::Node::eHidden);
}

class HideSelectedWalker : public scene::Graph::Walker
{
  bool m_hide;
public:
  HideSelectedWalker(bool hide)
    : m_hide(hide)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && selectable->isSelected())
    {
      hide_node(path.top(), m_hide);
    }
    return true;
  }
};

void Scene_Hide_Selected(bool hide)
{
  GlobalSceneGraph().traverse(HideSelectedWalker(hide));
}

void Select_Hide()
{
  Scene_Hide_Selected(true);
  SceneChangeNotify();
}

void HideSelected()
{
  Select_Hide();
  GlobalSelectionSystem().setSelectedAll(false);
}


class HideAllWalker : public scene::Graph::Walker
{
  bool m_hide;
public:
  HideAllWalker(bool hide)
    : m_hide(hide)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    hide_node(path.top(), m_hide);
    return true;
  }
};

void Scene_Hide_All(bool hide)
{
  GlobalSceneGraph().traverse(HideAllWalker(hide));
}

void Select_ShowAllHidden()
{
  Scene_Hide_All(false);
  SceneChangeNotify();
}



void Selection_Flipx()
{
  UndoableCommand undo("mirrorSelected -axis x");
  Select_FlipAxis(0);
}

void Selection_Flipy()
{
  UndoableCommand undo("mirrorSelected -axis y");
  Select_FlipAxis(1);
}

void Selection_Flipz()
{
  UndoableCommand undo("mirrorSelected -axis z");
  Select_FlipAxis(2);
}

void Selection_Rotatex()
{
  UndoableCommand undo("rotateSelected -axis x -angle -90");
  Select_RotateAxis(0,-90);
}

void Selection_Rotatey()
{
  UndoableCommand undo("rotateSelected -axis y -angle 90");
  Select_RotateAxis(1, 90);
}

void Selection_Rotatez()
{
  UndoableCommand undo("rotateSelected -axis z -angle -90");
  Select_RotateAxis(2,-90);
}



void Nudge(int nDim, float fNudge)
{
  Vector3 translate(0, 0, 0);
  translate[nDim] = fNudge;
  
  GlobalSelectionSystem().translateSelected(translate);
}

void Selection_NudgeZ(float amount)
{
  StringOutputStream command;
  command << "nudgeSelected -axis z -amount " << amount;
  UndoableCommand undo(command.c_str());

  Nudge(2, amount);
}

void Selection_MoveDown()
{
  Selection_NudgeZ(-GetGridSize());
}

void Selection_MoveUp()
{
  Selection_NudgeZ(GetGridSize());
}

void SceneSelectionChange(const Selectable& selectable)
{
  SceneChangeNotify();
}

SignalHandlerId Selection_boundsChanged;

void Selection_construct()
{
  typedef FreeCaller1<const Selectable&, SceneSelectionChange> SceneSelectionChangeCaller;
  GlobalSelectionSystem().addSelectionChangeCallback(SceneSelectionChangeCaller());
  typedef FreeCaller1<const Selectable&, UpdateWorkzone_ForSelectionChanged> UpdateWorkzoneForSelectionChangedCaller;
  GlobalSelectionSystem().addSelectionChangeCallback(UpdateWorkzoneForSelectionChangedCaller());
  typedef FreeCaller<UpdateWorkzone_ForSelection> UpdateWorkzoneForSelectionCaller;
  Selection_boundsChanged = GlobalSceneGraph().addBoundsChangedCallback(UpdateWorkzoneForSelectionCaller());
}

void Selection_destroy()
{
  GlobalSceneGraph().removeBoundsChangedCallback(Selection_boundsChanged);
}


#include "gtkdlgs.h"
#include <gtk/gtkbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkkeysyms.h>


inline Quaternion quaternion_for_euler_xyz_degrees(const Vector3& eulerXYZ)
{
#if 0
  return quaternion_for_matrix4_rotation(matrix4_rotation_for_euler_xyz_degrees(eulerXYZ));
#elif 0
  return quaternion_multiplied_by_quaternion(
    quaternion_multiplied_by_quaternion(
      quaternion_for_z(degrees_to_radians(eulerXYZ[2])),
      quaternion_for_y(degrees_to_radians(eulerXYZ[1]))
    ),
    quaternion_for_x(degrees_to_radians(eulerXYZ[0]))
  );
#elif 1
  double cx = cos(degrees_to_radians(eulerXYZ[0] * 0.5));
  double sx = sin(degrees_to_radians(eulerXYZ[0] * 0.5));
  double cy = cos(degrees_to_radians(eulerXYZ[1] * 0.5));
  double sy = sin(degrees_to_radians(eulerXYZ[1] * 0.5));
  double cz = cos(degrees_to_radians(eulerXYZ[2] * 0.5));
  double sz = sin(degrees_to_radians(eulerXYZ[2] * 0.5));

  return Quaternion(
    cz * cy * sx - sz * sy * cx,
    cz * sy * cx + sz * cy * sx,
    sz * cy * cx - cz * sy * sx,
    cz * cy * cx + sz * sy * sx
  );
#endif
}

struct RotateDialog
{
  GtkSpinButton* x;
  GtkSpinButton* y;
  GtkSpinButton* z;
};

static void rotatedlg_apply (GtkWidget *widget, RotateDialog* rotateDialog)
{
  Vector3 eulerXYZ;

  eulerXYZ[0] = static_cast<float>(gtk_spin_button_get_value(rotateDialog->x));
  gtk_spin_button_set_value(rotateDialog->x, 0.0f); // reset to 0 on Apply
  
  eulerXYZ[1] = static_cast<float>(gtk_spin_button_get_value(rotateDialog->y));
  gtk_spin_button_set_value(rotateDialog->y, 0.0f);
  
  eulerXYZ[2] = static_cast<float>(gtk_spin_button_get_value(rotateDialog->z));
  gtk_spin_button_set_value(rotateDialog->z, 0.0f);

  StringOutputStream command;
  command << "rotateSelectedEulerXYZ -x " << eulerXYZ[0] << " -y " << eulerXYZ[1] << " -z " << eulerXYZ[2];
  UndoableCommand undo(command.c_str());

  GlobalSelectionSystem().rotateSelected(quaternion_for_euler_xyz_degrees(eulerXYZ));
}

void DoRotateDlg()
{
  ModalDialog dialog;
  RotateDialog rotateDialog;

  GtkWindow* window = create_dialog_window(MainFrame_getWindow(), "Arbitrary rotation", G_CALLBACK(dialog_delete_callback), &dialog);

  GtkAccelGroup* accel = gtk_accel_group_new();
  gtk_window_add_accel_group(window, accel);

  {
    GtkHBox* hbox = create_dialog_hbox(4, 4);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(hbox));
    {
      GtkTable* table = create_dialog_table(3, 2, 4, 4);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(table), TRUE, TRUE, 0);
      {
        GtkWidget* label = gtk_label_new ("  X  ");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 0, 1,
                          (GtkAttachOptions) (0),
                          (GtkAttachOptions) (0), 0, 0);
      }
      {
        GtkWidget* label = gtk_label_new ("  Y  ");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 1, 2,
                          (GtkAttachOptions) (0),
                          (GtkAttachOptions) (0), 0, 0);
      }
      {
        GtkWidget* label = gtk_label_new ("  Z  ");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 2, 3,
                          (GtkAttachOptions) (0),
                          (GtkAttachOptions) (0), 0, 0);
      }
      {
        GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -359, 359, 1, 10, 10));
        GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
        gtk_widget_show(GTK_WIDGET(spin));
        gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_widget_set_size_request(GTK_WIDGET(spin), 64, -1);
        gtk_spin_button_set_wrap(spin, TRUE);

        gtk_widget_grab_focus(GTK_WIDGET(spin));

        rotateDialog.x = spin;
      }
      {
        GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -359, 359, 1, 10, 10));
        GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
        gtk_widget_show(GTK_WIDGET(spin));
        gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 1, 2,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_widget_set_size_request(GTK_WIDGET(spin), 64, -1);
        gtk_spin_button_set_wrap(spin, TRUE);

        rotateDialog.y = spin;
      }
      {
        GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -359, 359, 1, 10, 10));
        GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
        gtk_widget_show(GTK_WIDGET(spin));
        gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 2, 3,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_widget_set_size_request(GTK_WIDGET(spin), 64, -1);
        gtk_spin_button_set_wrap(spin, TRUE);

        rotateDialog.z = spin;
      }
    }
    {
      GtkVBox* vbox = create_dialog_vbox(4);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(vbox), TRUE, TRUE, 0);
      {
        GtkButton* button = create_dialog_button("OK", G_CALLBACK(dialog_button_ok), &dialog);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
        widget_make_default(GTK_WIDGET(button));
        gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
      }
      {
        GtkButton* button = create_dialog_button("Cancel", G_CALLBACK(dialog_button_cancel), &dialog);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
        gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
      }
      {
        GtkButton* button = create_dialog_button("Apply", G_CALLBACK(rotatedlg_apply), &rotateDialog);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
    }
  }

  if(modal_dialog_show(window, dialog) == eIDOK)
  {
    rotatedlg_apply(0, &rotateDialog);
  }

  gtk_widget_destroy(GTK_WIDGET(window));
}

void DoScaleDlg()
{
  ModalDialog dialog;
  GtkWidget* x;
  GtkWidget* y;
  GtkWidget* z;

  GtkWindow* window = create_dialog_window(MainFrame_getWindow(), "Scale", G_CALLBACK(dialog_delete_callback), &dialog);

  GtkAccelGroup* accel = gtk_accel_group_new();
  gtk_window_add_accel_group(window, accel);

  {
    GtkHBox* hbox = create_dialog_hbox(4, 4);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(hbox));
    {
      GtkTable* table = create_dialog_table(3, 2, 4, 4);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(table), TRUE, TRUE, 0);
      {
        GtkWidget* label = gtk_label_new ("X:");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 0, 1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      }
      {
        GtkWidget* label = gtk_label_new ("Y:");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 1, 2,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      }
      {
        GtkWidget* label = gtk_label_new ("Z:");
        gtk_widget_show (label);
        gtk_table_attach(table, label, 0, 1, 2, 3,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      }
      {
        GtkWidget* entry = gtk_entry_new();
        gtk_widget_show (entry);
        gtk_table_attach(table, entry, 1, 2, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

        gtk_widget_grab_focus(entry);

        x = entry;
      }
      {
        GtkWidget* entry = gtk_entry_new();
        gtk_widget_show (entry);
        gtk_table_attach(table, entry, 1, 2, 1, 2,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

        y = entry;
      }
      {
        GtkWidget* entry = gtk_entry_new();
        gtk_widget_show (entry);
        gtk_table_attach(table, entry, 1, 2, 2, 3,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

        z = entry;
      }
    }
    {
      GtkVBox* vbox = create_dialog_vbox(4);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(vbox), TRUE, TRUE, 0);
      {
        GtkButton* button = create_dialog_button("OK", G_CALLBACK(dialog_button_ok), &dialog);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
        widget_make_default(GTK_WIDGET(button));
        gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
      }
      {
        GtkButton* button = create_dialog_button("Cancel", G_CALLBACK(dialog_button_cancel), &dialog);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
        gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
      }
    }
  }

  // Initialize dialog
  gtk_entry_set_text (GTK_ENTRY (x), "1.0");
  gtk_entry_set_text (GTK_ENTRY (y), "1.0");
  gtk_entry_set_text (GTK_ENTRY (z), "1.0");

  if(modal_dialog_show(window, dialog) == eIDOK)
  {
    float sx, sy, sz;
    sx = static_cast<float>(atof(gtk_entry_get_text (GTK_ENTRY (x))));
    sy = static_cast<float>(atof(gtk_entry_get_text (GTK_ENTRY (y))));
    sz = static_cast<float>(atof(gtk_entry_get_text (GTK_ENTRY (z))));

    if (sx > 0 && sy > 0 && sz > 0)
    {
      StringOutputStream command;
      command << "scaleSelected -x " << sx << " -y " << sy << " -z " << sz;
      UndoableCommand undo(command.c_str());

      Select_Scale(sx, sy, sz);
    }
    else
    {
      globalOutputStream() << "Warning.. Tried to scale by a zero value.";
    }
  }

  gtk_widget_destroy(GTK_WIDGET(window));
}
