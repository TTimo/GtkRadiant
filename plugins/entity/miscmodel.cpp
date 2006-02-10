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
///\brief Represents the Quake3 misc_model entity.
///
/// This entity displays the model specified in its "model" key.
/// The "origin", "angles" and "modelscale*" keys directly control the entity's local-to-parent transform.

#include "cullable.h"
#include "renderable.h"
#include "editable.h"

#include "selectionlib.h"
#include "instancelib.h"
#include "transformlib.h"
#include "traverselib.h"
#include "entitylib.h"
#include "eclasslib.h"
#include "render.h"
#include "pivot.h"

#include "targetable.h"
#include "origin.h"
#include "angles.h"
#include "scale.h"
#include "model.h"
#include "filters.h"
#include "namedentity.h"
#include "keyobservers.h"
#include "namekeys.h"

#include "entity.h"

class MiscModel :
  public Snappable
{
  EntityKeyValues m_entity;
  KeyObserverMap m_keyObservers;
  MatrixTransform m_transform;

  OriginKey m_originKey;
  Vector3 m_origin;
  AnglesKey m_anglesKey;
  Vector3 m_angles;
  ScaleKey m_scaleKey;
  Vector3 m_scale;

  SingletonModel m_model;

  ClassnameFilter m_filter;
  NamedEntity m_named;
  NameKeys m_nameKeys;
  RenderablePivot m_renderOrigin;
  RenderableNamedEntity m_renderName;

  Callback m_transformChanged;
  Callback m_evaluateTransform;

  void construct()
  {
    m_keyObservers.insert("classname", ClassnameFilter::ClassnameChangedCaller(m_filter));
    m_keyObservers.insert(Static<KeyIsName>::instance().m_nameKey, NamedEntity::IdentifierChangedCaller(m_named));
    m_keyObservers.insert("model", SingletonModel::ModelChangedCaller(m_model));
    m_keyObservers.insert("origin", OriginKey::OriginChangedCaller(m_originKey));
    m_keyObservers.insert("angle", AnglesKey::AngleChangedCaller(m_anglesKey));
    m_keyObservers.insert("angles", AnglesKey::AnglesChangedCaller(m_anglesKey));
    m_keyObservers.insert("modelscale", ScaleKey::UniformScaleChangedCaller(m_scaleKey));
    m_keyObservers.insert("modelscale_vec", ScaleKey::ScaleChangedCaller(m_scaleKey));
  }

  void updateTransform()
  {
    m_transform.localToParent() = g_matrix4_identity;
    matrix4_transform_by_euler_xyz_degrees(m_transform.localToParent(), m_origin, m_angles, m_scale);
    m_transformChanged();
  }
  void originChanged()
  {
    m_origin = m_originKey.m_origin;
    updateTransform();
  }
  typedef MemberCaller<MiscModel, &MiscModel::originChanged> OriginChangedCaller;
  void anglesChanged()
  {
    m_angles = m_anglesKey.m_angles;
    updateTransform();
  }
  typedef MemberCaller<MiscModel, &MiscModel::anglesChanged> AnglesChangedCaller;
  void scaleChanged()
  {
    m_scale = m_scaleKey.m_scale;
    updateTransform();
  }
  typedef MemberCaller<MiscModel, &MiscModel::scaleChanged> ScaleChangedCaller;
public:

  MiscModel(EntityClass* eclass, scene::Node& node, const Callback& transformChanged, const Callback& evaluateTransform) :
    m_entity(eclass),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_anglesKey(AnglesChangedCaller(*this)),
    m_angles(ANGLESKEY_IDENTITY),
    m_scaleKey(ScaleChangedCaller(*this)),
    m_scale(SCALEKEY_IDENTITY),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_renderName(m_named, g_vector3_identity),
    m_transformChanged(transformChanged),
    m_evaluateTransform(evaluateTransform)
  {
    construct();
  }
  MiscModel(const MiscModel& other, scene::Node& node, const Callback& transformChanged, const Callback& evaluateTransform) :
    m_entity(other.m_entity),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_anglesKey(AnglesChangedCaller(*this)),
    m_angles(ANGLESKEY_IDENTITY),
    m_scaleKey(ScaleChangedCaller(*this)),
    m_scale(SCALEKEY_IDENTITY),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
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

  scene::Traversable& getTraversable()
  {
    return m_model.getTraversable();
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
    m_model.attach(observer);
  }
  void detach(scene::Traversable::Observer* observer)
  {
    m_model.detach(observer);
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld, bool selected) const
  {
    if(selected)
    {
      m_renderOrigin.render(renderer, volume, localToWorld);
    }

    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eWireframeOnly);
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld, bool selected) const
  {
    renderSolid(renderer, volume, localToWorld, selected);
    if(g_showNames)
    {
      renderer.addRenderable(m_renderName, localToWorld);
    }
  }

  void translate(const Vector3& translation)
  {
    m_origin = origin_translated(m_origin, translation);
  }
  void rotate(const Quaternion& rotation)
  {
    m_angles = angles_rotated(m_angles, rotation);
  }
  void scale(const Vector3& scaling)
  {
    m_scale = scale_scaled(m_scale, scaling);
  }
  void snapto(float snap)
  {
    m_originKey.m_origin = origin_snapped(m_originKey.m_origin, snap);
    m_originKey.write(&m_entity);
  }
  void revertTransform()
  {
    m_origin = m_originKey.m_origin;
    m_angles = m_anglesKey.m_angles;
    m_scale = m_scaleKey.m_scale;
  }
  void freezeTransform()
  {
    m_originKey.m_origin = m_origin;
    m_originKey.write(&m_entity);
    m_anglesKey.m_angles = m_angles;
    m_anglesKey.write(&m_entity);
    m_scaleKey.m_scale = m_scale;
    m_scaleKey.write(&m_entity);
  }
  void transformChanged()
  {
    revertTransform();
    m_evaluateTransform();
    updateTransform();
  }
  typedef MemberCaller<MiscModel, &MiscModel::transformChanged> TransformChangedCaller;
};

class MiscModelInstance : public TargetableInstance, public TransformModifier, public Renderable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      m_casts = TargetableInstance::StaticTypeCasts::instance().get();
      InstanceStaticCast<MiscModelInstance, Renderable>::install(m_casts);
      InstanceStaticCast<MiscModelInstance, Transformable>::install(m_casts);
      InstanceIdentityCast<MiscModelInstance>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  MiscModel& m_contained;
public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  STRING_CONSTANT(Name, "MiscModelInstance");

  MiscModelInstance(const scene::Path& path, scene::Instance* parent, MiscModel& miscmodel) :
    TargetableInstance(path, parent, this, StaticTypeCasts::instance().get(), miscmodel.getEntity(), *this),
    TransformModifier(MiscModel::TransformChangedCaller(miscmodel), ApplyTransformCaller(*this)),
    m_contained(miscmodel)
  {
    m_contained.instanceAttach(Instance::path());
    StaticRenderableConnectionLines::instance().attach(*this);
  }
  ~MiscModelInstance()
  {
    StaticRenderableConnectionLines::instance().detach(*this);
    m_contained.instanceDetach(Instance::path());
  }
  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderSolid(renderer, volume, Instance::localToWorld(), getSelectable().isSelected());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderWireframe(renderer, volume, Instance::localToWorld(), getSelectable().isSelected());
  }
  void evaluateTransform()
  {
    if(getType() == TRANSFORM_PRIMITIVE)
    {
      m_contained.translate(getTranslation());
      m_contained.rotate(getRotation());
      m_contained.scale(getScale());
    }
  }
  void applyTransform()
  {
    m_contained.revertTransform();
    evaluateTransform();
    m_contained.freezeTransform();
  }
  typedef MemberCaller<MiscModelInstance, &MiscModelInstance::applyTransform> ApplyTransformCaller;
};

class MiscModelNode :
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
      NodeStaticCast<MiscModelNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<MiscModelNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<MiscModelNode, scene::Traversable>::install(m_casts);
      NodeContainedCast<MiscModelNode, Snappable>::install(m_casts);
      NodeContainedCast<MiscModelNode, TransformNode>::install(m_casts);
      NodeContainedCast<MiscModelNode, Entity>::install(m_casts);
      NodeContainedCast<MiscModelNode, Nameable>::install(m_casts);
      NodeContainedCast<MiscModelNode, Namespaced>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  MiscModel m_contained;

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

  MiscModelNode(EntityClass* eclass) :
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(eclass, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSetEvaluateTransform<MiscModelInstance>::Caller(m_instances))
  {
    construct();
  }
  MiscModelNode(const MiscModelNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    scene::Traversable::Observer(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(other.m_contained, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSetEvaluateTransform<MiscModelInstance>::Caller(m_instances))
  {
    construct();
  }
  ~MiscModelNode()
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
    return (new MiscModelNode(*this))->node();
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
    return new MiscModelInstance(path, parent, m_contained);
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

scene::Node& New_MiscModel(EntityClass* eclass)
{
  return (new MiscModelNode(eclass))->node();
}

void MiscModel_construct()
{
}
void MiscModel_destroy()
{
}
