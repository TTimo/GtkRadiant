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
///\brief Represents any entity which has a fixed size specified in its entity-definition and does not display a model (e.g. info_player_start).
///
/// This entity displays an axis-aligned bounding box of the size and colour specified in its entity-definition.
/// The "origin" key directly controls the entity's local-to-parent transform.
/// An arrow is drawn to visualise the "angle" key.

#include "cullable.h"
#include "renderable.h"
#include "editable.h"

#include "math/frustum.h"
#include "selectionlib.h"
#include "instancelib.h"
#include "transformlib.h"
#include "entitylib.h"
#include "render.h"
#include "eclasslib.h"
#include "math/line.h"

#include "targetable.h"
#include "origin.h"
#include "angle.h"
#include "filters.h"
#include "namedentity.h"
#include "keyobservers.h"
#include "namekeys.h"
#include "rotation.h"

#include "entity.h"


class RenderableArrow : public OpenGLRenderable
{
  const Ray& m_ray;

public:
  RenderableArrow(const Ray& ray)
    : m_ray(ray)
  {
  }

  void render(RenderStateFlags state) const
  {
    arrow_draw(m_ray.origin, m_ray.direction);
  }
};

inline void read_aabb(AABB& aabb, const EntityClass& eclass)
{
  aabb = aabb_for_minmax(eclass.mins, eclass.maxs);
}


class GenericEntity :
  public Cullable,
  public Bounded,
  public Snappable
{
  EntityKeyValues m_entity;
  KeyObserverMap m_keyObservers;
  MatrixTransform m_transform;

  OriginKey m_originKey;
  Vector3 m_origin;
  AngleKey m_angleKey;
  float m_angle;

  ClassnameFilter m_filter;
  NamedEntity m_named;
  NameKeys m_nameKeys;

  AABB m_aabb_local;
  Ray m_ray;

  RenderableArrow m_arrow;
  RenderableSolidAABB m_aabb_solid;
  RenderableWireframeAABB m_aabb_wire;
  RenderableNamedEntity m_renderName;

  Callback m_transformChanged;
  Callback m_evaluateTransform;

  void construct()
  {
    read_aabb(m_aabb_local, m_entity.getEntityClass());
    m_ray.origin = m_aabb_local.origin;
    m_ray.direction[0] = 1;
    m_ray.direction[1] = 0;
    m_ray.direction[2] = 0;

    m_keyObservers.insert("classname", ClassnameFilter::ClassnameChangedCaller(m_filter));
    m_keyObservers.insert(Static<KeyIsName>::instance().m_nameKey, NamedEntity::IdentifierChangedCaller(m_named));
    m_keyObservers.insert("angle", AngleKey::AngleChangedCaller(m_angleKey));
    m_keyObservers.insert("origin", OriginKey::OriginChangedCaller(m_originKey));
  }

  void updateTransform()
  {
    m_transform.localToParent() = g_matrix4_identity;
    matrix4_translate_by_vec3(m_transform.localToParent(), m_origin);
    m_ray.direction = matrix4_transformed_direction(matrix4_rotation_for_z(degrees_to_radians(m_angle)), Vector3(1, 0, 0));
    m_transformChanged();
  }
  typedef MemberCaller<GenericEntity, &GenericEntity::updateTransform> UpdateTransformCaller;
  void originChanged()
  {
    m_origin = m_originKey.m_origin;
    updateTransform();
  }
  typedef MemberCaller<GenericEntity, &GenericEntity::originChanged> OriginChangedCaller;
  void angleChanged()
  {
    m_angle = m_angleKey.m_angle;
    updateTransform();
  }
  typedef MemberCaller<GenericEntity, &GenericEntity::angleChanged> AngleChangedCaller;
public:

  GenericEntity(EntityClass* eclass, scene::Node& node, const Callback& transformChanged, const Callback& evaluateTransform) :
    m_entity(eclass),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_angleKey(AngleChangedCaller(*this)),
    m_angle(ANGLEKEY_IDENTITY),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_arrow(m_ray),
    m_aabb_solid(m_aabb_local),
    m_aabb_wire(m_aabb_local),
    m_renderName(m_named, g_vector3_identity),
    m_transformChanged(transformChanged),
    m_evaluateTransform(evaluateTransform)
  {
    construct();
  }
  GenericEntity(const GenericEntity& other, scene::Node& node, const Callback& transformChanged, const Callback& evaluateTransform) :
    m_entity(other.m_entity),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_angleKey(AngleChangedCaller(*this)),
    m_angle(ANGLEKEY_IDENTITY),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_arrow(m_ray),
    m_aabb_solid(m_aabb_local),
    m_aabb_wire(m_aabb_local),
    m_renderName(m_named, g_vector3_identity),
    m_transformChanged(transformChanged),
    m_evaluateTransform(evaluateTransform)
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
      m_entity.attach(m_keyObservers);
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      m_entity.detach(m_keyObservers);
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

  const AABB& localAABB() const
  {
    return m_aabb_local;
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    return volume.TestAABB(localAABB(), localToWorld);
  }

  void renderArrow(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    if(g_showAngles)
    {
      renderer.addRenderable(m_arrow, localToWorld);
    }
  }
  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_entity.getEntityClass().m_state_fill, Renderer::eFullMaterials);
    renderer.addRenderable(m_aabb_solid, localToWorld);
    renderArrow(renderer, volume, localToWorld);
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eWireframeOnly);
    renderer.addRenderable(m_aabb_wire, localToWorld);
    renderArrow(renderer, volume, localToWorld);
    if(g_showNames)
    {
      renderer.addRenderable(m_renderName, localToWorld);
    }
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

  void translate(const Vector3& translation)
  {
    m_origin = origin_translated(m_origin, translation);
  }
  void rotate(const Quaternion& rotation)
  {
    m_angle = angle_rotated(m_angle, rotation);
  }
  void snapto(float snap)
  {
    m_originKey.m_origin = origin_snapped(m_originKey.m_origin, snap);
    m_originKey.write(&m_entity);
  }
  void revertTransform()
  {
    m_origin = m_originKey.m_origin;
    m_angle = m_angleKey.m_angle;
  }
  void freezeTransform()
  {
    m_originKey.m_origin = m_origin;
    m_originKey.write(&m_entity);
    m_angleKey.m_angle = m_angle;
    m_angleKey.write(&m_entity);
  }
  void transformChanged()
  {
    revertTransform();
    m_evaluateTransform();
    updateTransform();
  }
  typedef MemberCaller<GenericEntity, &GenericEntity::transformChanged> TransformChangedCaller;
};

class GenericEntityInstance :
  public TargetableInstance,
  public TransformModifier,
  public Renderable,
  public SelectionTestable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      m_casts = TargetableInstance::StaticTypeCasts::instance().get();
      InstanceContainedCast<GenericEntityInstance, Bounded>::install(m_casts);
      InstanceContainedCast<GenericEntityInstance, Cullable>::install(m_casts);
      InstanceStaticCast<GenericEntityInstance, Renderable>::install(m_casts);
      InstanceStaticCast<GenericEntityInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<GenericEntityInstance, Transformable>::install(m_casts);
      InstanceIdentityCast<GenericEntityInstance>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  GenericEntity& m_contained;
  mutable AABB m_bounds;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Bounded& get(NullType<Bounded>)
  {
    return m_contained;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_contained;
  }

  STRING_CONSTANT(Name, "GenericEntityInstance");

  GenericEntityInstance(const scene::Path& path, scene::Instance* parent, GenericEntity& contained) :
    TargetableInstance(path, parent, this, StaticTypeCasts::instance().get(), contained.getEntity(), *this),
    TransformModifier(GenericEntity::TransformChangedCaller(contained), ApplyTransformCaller(*this)),
    m_contained(contained)
  {
    m_contained.instanceAttach(Instance::path());

    StaticRenderableConnectionLines::instance().attach(*this);
  }
  ~GenericEntityInstance()
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

  void testSelect(Selector& selector, SelectionTest& test)
  {
    m_contained.testSelect(selector, test, Instance::localToWorld());
  }

  void evaluateTransform()
  {
    if(getType() == TRANSFORM_PRIMITIVE)
    {
      m_contained.translate(getTranslation());
      m_contained.rotate(getRotation());
    }
  }
  void applyTransform()
  {
    m_contained.revertTransform();
    evaluateTransform();
    m_contained.freezeTransform();
  }
  typedef MemberCaller<GenericEntityInstance, &GenericEntityInstance::applyTransform> ApplyTransformCaller;
};

class GenericEntityNode :
  public scene::Node::Symbiot,
  public scene::Instantiable,
  public scene::Cloneable
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<GenericEntityNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<GenericEntityNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<GenericEntityNode, Snappable>::install(m_casts);
      NodeContainedCast<GenericEntityNode, TransformNode>::install(m_casts);
      NodeContainedCast<GenericEntityNode, Entity>::install(m_casts);
      NodeContainedCast<GenericEntityNode, Nameable>::install(m_casts);
      NodeContainedCast<GenericEntityNode, Namespaced>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  InstanceSet m_instances;

  scene::Node m_node;
  GenericEntity m_contained;

public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Snappable& get(NullType<Snappable>)
  {
    return m_contained;
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

  GenericEntityNode(EntityClass* eclass) :
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(eclass, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSetEvaluateTransform<GenericEntityInstance>::Caller(m_instances))
  {
  }
  GenericEntityNode(const GenericEntityNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(other.m_contained, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSetEvaluateTransform<GenericEntityInstance>::Caller(m_instances))
  {
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
    return (new GenericEntityNode(*this))->node();
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new GenericEntityInstance(path, parent, m_contained);
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

scene::Node& New_GenericEntity(EntityClass* eclass)
{
  return (new GenericEntityNode(eclass))->node();
}
