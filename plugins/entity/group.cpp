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

///\file
///\brief Represents any entity which does not have a fixed size specified in its entity-definition (except misc_model).
///
/// This entity behaves as a group, i.e. it contains brushes.

#include "cullable.h"
#include "renderable.h"
#include "editable.h"

#include "selectionlib.h"
#include "instancelib.h"
#include "transformlib.h"
#include "traverselib.h"
#include "entitylib.h"
#include "render.h"
#include "eclasslib.h"

#include "targetable.h"
#include "origin.h"
#include "angles.h"
#include "scale.h"
#include "filters.h"
#include "namedentity.h"
#include "keyobservers.h"
#include "namekeys.h"

#include "entity.h"

class Group
{
  EntityKeyValues m_entity;
  KeyObserverMap m_keyObservers;
  MatrixTransform m_transform;
  TraversableNodeSet m_traverse;

  ClassnameFilter m_filter;
  NamedEntity m_named;
  NameKeys m_nameKeys;

  RenderableNamedEntity m_renderName;

  Callback m_transformChanged;

  void construct()
  {
    m_keyObservers.insert("classname", ClassnameFilter::ClassnameChangedCaller(m_filter));
    m_keyObservers.insert(Static<KeyIsName>::instance().m_nameKey, NamedEntity::IdentifierChangedCaller(m_named));
  }
 
public:
  Group(EntityClass* eclass, scene::Node& node, const Callback& transformChanged) :
    m_entity(eclass),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_renderName(m_named, g_vector3_identity),
    m_transformChanged(transformChanged)
  {
    construct();
  }
  Group(const Group& other, scene::Node& node, const Callback& transformChanged) :
    m_entity(other.m_entity),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_renderName(m_named, g_vector3_identity),
    m_transformChanged(transformChanged)
  {
    construct();
  }

  InstanceCounter m_instanceCounter;
  void instanceAttach(const scene::Path& path)
  {
    if(++m_instanceCounter.m_count == 1)
    {
      m_filter.instanceAttach();
      m_entity.instanceAttach(path_find_mapfile(path.begin(), path.end()));
      m_traverse.instanceAttach(path_find_mapfile(path.begin(), path.end()));
      m_entity.attach(m_keyObservers);
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      m_entity.detach(m_keyObservers);
      m_traverse.instanceDetach(path_find_mapfile(path.begin(), path.end()));
      m_entity.instanceDetach(path_find_mapfile(path.begin(), path.end()));
      m_filter.instanceDetach();
    }
  }

  EntityKeyValues& getEntity()
  {
    return m_entity;
  }
  const EntityKeyValues& getEntity() const
  {
    return m_entity;
  }

  scene::Traversable& getTraversable()
  {
    return m_traverse;
  }
  Namespaced& getNamespaced()
  {
    return m_nameKeys;
  }
  Nameable& getNameable()
  {
    return m_named;
  }
  TransformNode& getTransformNode()
  {
    return m_transform;
  }

  void attach(scene::Traversable::Observer* observer)
  {
    m_traverse.attach(observer);
  }
  void detach(scene::Traversable::Observer* observer)
  {
    m_traverse.detach(observer);
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eWireframeOnly);
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderSolid(renderer, volume, localToWorld);
#if 0
    if(g_showNames)
    {
      renderer.addRenderable(m_renderName, g_matrix4_identity);
    }
#endif
  }
};

#if 0
class TransformableSetTranslation
{
  Translation m_value;
public:
  TransformableSetTranslation(const Translation& value) : m_value(value)
  {
  }
  void operator()(Transformable& transformable) const
  {
    transformable.setTranslation(m_value);
  }
};

class TransformableSetRotation
{
  Rotation m_value;
public:
  TransformableSetRotation(const Rotation& value) : m_value(value)
  {
  }
  void operator()(Transformable& transformable) const
  {
    transformable.setRotation(m_value);
  }
};

class TransformableSetScale
{
  Scale m_value;
public:
  TransformableSetScale(const Scale& value) : m_value(value)
  {
  }
  void operator()(Transformable& transformable) const
  {
    transformable.setScale(m_value);
  }
};

class TransformableSetType
{
  TransformModifierType m_value;
public:
  TransformableSetType(const TransformModifierType& value) : m_value(value)
  {
  }
  void operator()(Transformable& transformable) const
  {
    transformable.setType(m_value);
  }
};

class TransformableFreezeTransform
{
  TransformModifierType m_value;
public:
  void operator()(Transformable& transformable) const
  {
    transformable.freezeTransform();
  }
};

template<typename Functor>
inline void Scene_forEachChildTransformable(const Functor& functor, const scene::Path& path)
{
  GlobalSceneGraph().traverse_subgraph(ChildInstanceWalker< InstanceApply<Transformable, Functor> >(functor), path);
}
#endif

class GroupInstance :
  public TargetableInstance,
#if 0
  public Transformable,
#endif
  public Renderable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      m_casts = TargetableInstance::StaticTypeCasts::instance().get();
      InstanceStaticCast<GroupInstance, Renderable>::install(m_casts);
#if 0
      InstanceStaticCast<GroupInstance, Transformable>::install(m_casts);
#endif
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  Group& m_contained;
public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  GroupInstance(const scene::Path& path, scene::Instance* parent, Group& group) :
    TargetableInstance(path, parent, this, StaticTypeCasts::instance().get(), group.getEntity(), *this),
    m_contained(group)
  {
    m_contained.instanceAttach(Instance::path());
    StaticRenderableConnectionLines::instance().attach(*this);
  }
  ~GroupInstance()
  {
    StaticRenderableConnectionLines::instance().detach(*this);
    m_contained.instanceDetach(Instance::path());
  }
  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderSolid(renderer, volume, Instance::localToWorld());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderWireframe(renderer, volume, Instance::localToWorld());
  }

#if 0
  void setType(TransformModifierType type)
  {
    Scene_forEachChildTransformable(TransformableSetType(type), Instance::path());
  }
  void setTranslation(const Translation& value)
  {
    Scene_forEachChildTransformable(TransformableSetTranslation(value), Instance::path());
  }
  void setRotation(const Rotation& value)
  {
    Scene_forEachChildTransformable(TransformableSetRotation(value), Instance::path());
  }
  void setScale(const Scale& value)
  {
    Scene_forEachChildTransformable(TransformableSetScale(value), Instance::path());
  }
  void freezeTransform()
  {
    Scene_forEachChildTransformable(TransformableFreezeTransform(), Instance::path());
  }

  void evaluateTransform()
  {
  }
#endif
};

class GroupNode :
  public scene::Node::Symbiot,
  public scene::Instantiable,
  public scene::Cloneable,
  public scene::Traversable::Observer
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<GroupNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<GroupNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<GroupNode, scene::Traversable>::install(m_casts);
      NodeContainedCast<GroupNode, TransformNode>::install(m_casts);
      NodeContainedCast<GroupNode, Entity>::install(m_casts);
      NodeContainedCast<GroupNode, Nameable>::install(m_casts);
      NodeContainedCast<GroupNode, Namespaced>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  Group m_contained;

  void construct()
  {
    m_contained.attach(this);
  }
  void destroy()
  {
    m_contained.detach(this);
  }

public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  scene::Traversable& get(NullType<scene::Traversable>)
  {
    return m_contained.getTraversable();
  }
  TransformNode& get(NullType<TransformNode>)
  {
    return m_contained.getTransformNode();
  }
  Entity& get(NullType<Entity>)
  {
    return m_contained.getEntity();
  }
  Nameable& get(NullType<Nameable>)
  {
    return m_contained.getNameable();
  }
  Namespaced& get(NullType<Namespaced>)
  {
    return m_contained.getNamespaced();
  }

  GroupNode(EntityClass* eclass) :
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(eclass, m_node, InstanceSet::TransformChangedCaller(m_instances))
  {
    construct();
  }
  GroupNode(const GroupNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    scene::Traversable::Observer(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(other.m_contained, m_node, InstanceSet::TransformChangedCaller(m_instances))
  {
    construct();
  }
  ~GroupNode()
  {
    destroy();
  }

  void release()
  {
    delete this;
  }
  scene::Node& node()
  {
    return m_node;
  }

  scene::Node& clone() const
  {
    return (new GroupNode(*this))->node();
  }

  void insert(scene::Node& child)
  {
    m_instances.insert(child);
  }
  void erase(scene::Node& child)
  {
    m_instances.erase(child);
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new GroupInstance(path, parent, m_contained);
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

scene::Node& New_Group(EntityClass* eclass)
{
  return (new GroupNode(eclass))->node();
}
