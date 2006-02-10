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

#include "nullmodel.h"

#include "debugging/debugging.h"

#include "iscenegraph.h"
#include "irender.h"
#include "iselection.h"
#include "iundo.h"
#include "ientity.h"
#include "ireference.h"
#include "igl.h"
#include "cullable.h"
#include "renderable.h"
#include "selectable.h"

#include "math/frustum.h"
#include "scenelib.h"
#include "instancelib.h"
#include "entitylib.h"

class NullModel :
public Bounded,
public Cullable
{
  Shader* m_state;
  AABB m_aabb_local;
  RenderableSolidAABB m_aabb_solid;
  RenderableWireframeAABB m_aabb_wire;
public:
  NullModel() : m_aabb_local(Vector3(0, 0, 0), Vector3(8, 8, 8)), m_aabb_solid(m_aabb_local), m_aabb_wire(m_aabb_local)
  {
    m_state = GlobalShaderCache().capture("");
  }
  ~NullModel()
  {
    GlobalShaderCache().release("");
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    return volume.TestAABB(m_aabb_local, localToWorld);
  }

  const AABB& localAABB() const
  {
    return m_aabb_local;
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_state, Renderer::eFullMaterials);
    renderer.addRenderable(m_aabb_solid, localToWorld);
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.addRenderable(m_aabb_wire, localToWorld);
  }

  void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld)
  {
    test.BeginMesh(localToWorld);

    SelectionIntersection best;
    aabb_testselect(m_aabb_local, test, best);
    if(best.valid())
    {
      selector.addIntersection(best);
    }
  }
};

class NullModelInstance : public scene::Instance, public Renderable, public SelectionTestable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      InstanceContainedCast<NullModelInstance, Bounded>::install(m_casts);
      InstanceContainedCast<NullModelInstance, Cullable>::install(m_casts);
      InstanceStaticCast<NullModelInstance, Renderable>::install(m_casts);
      InstanceStaticCast<NullModelInstance, SelectionTestable>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  NullModel& m_nullmodel;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Bounded& get(NullType<Bounded>)
  {
    return m_nullmodel;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_nullmodel;
  }

  NullModelInstance(const scene::Path& path, scene::Instance* parent, NullModel& nullmodel) :
    Instance(path, parent, this, StaticTypeCasts::instance().get()),
    m_nullmodel(nullmodel)
  {
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_nullmodel.renderSolid(renderer, volume, Instance::localToWorld());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_nullmodel.renderWireframe(renderer, volume, Instance::localToWorld());
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    m_nullmodel.testSelect(selector, test, Instance::localToWorld());
  }
};

class NullModelNode : public scene::Node::Symbiot, public scene::Instantiable
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<NullModelNode, scene::Instantiable>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  NullModel m_nullmodel;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  NullModelNode() : m_node(this, this, StaticTypeCasts::instance().get())
  {
    m_node.m_isRoot = true;
  }

  void release()
  {
    delete this;
  }
  scene::Node& node()
  {
    return m_node;
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new NullModelInstance(path, parent, m_nullmodel);
  }
  void forEachInstance(const scene::Instantiable::Visitor& visitor)
  {
    m_instances.forEachInstance(visitor);
  }
  void insert(scene::Instantiable::Observer* observer, const scene::Path& path, scene::Instance* instance)
  {
    m_instances.insert(observer, path, instance);
  }
  scene::Instance* erase(scene::Instantiable::Observer* observer, const scene::Path& path)
  {
    return m_instances.erase(observer, path);
  }
};

NodeSmartReference NewNullModel()
{
  return NodeSmartReference((new NullModelNode)->node());
}

void NullModel_construct()
{
}
void NullModel_destroy()
{
}

