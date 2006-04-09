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
///\brief Represents any Doom3 entity which does not have a fixed size specified in its entity-definition (e.g. func_static).
///
/// This entity behaves as a group only when the "model" key is empty or is the same as the "name" key. Otherwise it behaves as a model.
/// When behaving as a group, the "origin" key is the translation to be applied to all brushes (not patches) grouped under this entity.
/// When behaving as a model, the "origin", "angle" and "rotation" keys directly control the entity's local-to-parent transform.
/// When either the "curve_Nurbs" or "curve_CatmullRomSpline" keys define a curve, the curve is rendered and can be edited.

#include "doom3group.h"

#include "cullable.h"
#include "renderable.h"
#include "editable.h"
#include "modelskin.h"

#include "selectionlib.h"
#include "instancelib.h"
#include "transformlib.h"
#include "traverselib.h"
#include "entitylib.h"
#include "render.h"
#include "eclasslib.h"
#include "stream/stringstream.h"
#include "pivot.h"

#include "targetable.h"
#include "origin.h"
#include "angle.h"
#include "rotation.h"
#include "model.h"
#include "filters.h"
#include "namedentity.h"
#include "keyobservers.h"
#include "namekeys.h"
#include "curve.h"
#include "modelskinkey.h"

#include "entity.h"

inline void PointVertexArray_testSelect(PointVertex* first, std::size_t count, SelectionTest& test, SelectionIntersection& best)
{
  test.TestLineStrip(
    VertexPointer(
      reinterpret_cast<VertexPointer::pointer>(&first->vertex),
      sizeof(PointVertex)
    ),
    IndexPointer::index_type(count),
    best
  );
}

class Doom3Group :
  public Bounded,
  public Snappable
{
  EntityKeyValues m_entity;
  KeyObserverMap m_keyObservers;
  TraversableNodeSet m_traverse;
  MatrixTransform m_transform;

  SingletonModel m_model;
  OriginKey m_originKey;
  Vector3 m_origin;
  RotationKey m_rotationKey;
  Float9 m_rotation;

  ClassnameFilter m_filter;
  NamedEntity m_named;
  NameKeys m_nameKeys;
  TraversableObserverPairRelay m_traverseObservers;
  Doom3GroupOrigin m_funcStaticOrigin;
  RenderablePivot m_renderOrigin;
  RenderableNamedEntity m_renderName;
  ModelSkinKey m_skin;

public:
  NURBSCurve m_curveNURBS;
  SignalHandlerId m_curveNURBSChanged;
  CatmullRomSpline m_curveCatmullRom;
  SignalHandlerId m_curveCatmullRomChanged;
private:
  mutable AABB m_curveBounds;

  Callback m_transformChanged;
  Callback m_evaluateTransform;

  CopiedString m_name;
  CopiedString m_modelKey;
  bool m_isModel;

  scene::Traversable* m_traversable;

  void construct()
  {
    default_rotation(m_rotation);

    m_keyObservers.insert("classname", ClassnameFilter::ClassnameChangedCaller(m_filter));
    m_keyObservers.insert(Static<KeyIsName>::instance().m_nameKey, NamedEntity::IdentifierChangedCaller(m_named));
    m_keyObservers.insert("model", Doom3Group::ModelChangedCaller(*this));
    m_keyObservers.insert("origin", OriginKey::OriginChangedCaller(m_originKey));
    m_keyObservers.insert("angle", RotationKey::AngleChangedCaller(m_rotationKey));
    m_keyObservers.insert("rotation", RotationKey::RotationChangedCaller(m_rotationKey));
    m_keyObservers.insert("name", NameChangedCaller(*this));
    m_keyObservers.insert(curve_Nurbs, NURBSCurve::CurveChangedCaller(m_curveNURBS));
    m_keyObservers.insert(curve_CatmullRomSpline, CatmullRomSpline::CurveChangedCaller(m_curveCatmullRom));
    m_keyObservers.insert("skin", ModelSkinKey::SkinChangedCaller(m_skin));

    m_traverseObservers.attach(m_funcStaticOrigin);
    m_isModel = false;
    m_nameKeys.setKeyIsName(keyIsNameDoom3Doom3Group);
    attachTraverse();

    m_entity.attach(m_keyObservers);
  }
  void destroy()
  {
    m_entity.detach(m_keyObservers);

    if(isModel())
    {
      detachModel();
    }
    else
    {
      detachTraverse();
    }

    m_traverseObservers.detach(m_funcStaticOrigin);
  }

  void attachModel()
  {
    m_traversable = &m_model.getTraversable();
    m_model.attach(&m_traverseObservers);
  }
  void detachModel()
  {
    m_traversable = 0;
    m_model.detach(&m_traverseObservers);
  }
  void attachTraverse()
  {
    m_traversable = &m_traverse;
    m_traverse.attach(&m_traverseObservers);
  }
  void detachTraverse()
  {
    m_traversable = 0;
    m_traverse.detach(&m_traverseObservers);
  }

  bool isModel() const
  {
    return m_isModel;
  }
 
  void setIsModel(bool newValue)
  {
    if(newValue && !m_isModel)
    {
      detachTraverse();
      attachModel();

      m_nameKeys.setKeyIsName(Static<KeyIsName>::instance().m_keyIsName);
      m_model.modelChanged(m_modelKey.c_str());
    }
    else if(!newValue && m_isModel)
    {
      detachModel();
      attachTraverse();

      m_nameKeys.setKeyIsName(keyIsNameDoom3Doom3Group);
    }
    m_isModel = newValue;
    updateTransform();
  }

  void updateIsModel()
  {
    setIsModel(!string_empty(m_modelKey.c_str()) && !string_equal(m_modelKey.c_str(), m_name.c_str()));
  }

  void nameChanged(const char* value)
  {
    m_name = value;
    updateIsModel();
  }
  typedef MemberCaller1<Doom3Group, const char*, &Doom3Group::nameChanged> NameChangedCaller;

  void modelChanged(const char* value)
  {
    m_modelKey = value;
    updateIsModel();
    if(isModel())
    {
      m_model.modelChanged(value);
    }
    else
    {
      m_model.modelChanged("");
    }
  }
  typedef MemberCaller1<Doom3Group, const char*, &Doom3Group::modelChanged> ModelChangedCaller;

  void updateTransform()
  {
    m_transform.localToParent() = g_matrix4_identity;
    if(isModel())
    {
      matrix4_translate_by_vec3(m_transform.localToParent(), m_originKey.m_origin);
      matrix4_multiply_by_matrix4(m_transform.localToParent(), rotation_toMatrix(m_rotationKey.m_rotation));
    }
    m_transformChanged();
    if(!isModel())
    {
      m_funcStaticOrigin.originChanged();
    }
  }
  typedef MemberCaller<Doom3Group, &Doom3Group::updateTransform> UpdateTransformCaller;

  void originChanged()
  {
    m_origin = m_originKey.m_origin;
    updateTransform();
  }
  typedef MemberCaller<Doom3Group, &Doom3Group::originChanged> OriginChangedCaller;

  void rotationChanged()
  {
    rotation_assign(m_rotation, m_rotationKey.m_rotation);
    updateTransform();
  }
  typedef MemberCaller<Doom3Group, &Doom3Group::rotationChanged> RotationChangedCaller;

  void skinChanged()
  {
    if(isModel())
    {
      scene::Node* node = m_model.getNode();
      if(node != 0)
      {
        Node_modelSkinChanged(*node);
      }
    }
  }
  typedef MemberCaller<Doom3Group, &Doom3Group::skinChanged> SkinChangedCaller;

public:
  Doom3Group(EntityClass* eclass, scene::Node& node, const Callback& transformChanged, const Callback& boundsChanged, const Callback& evaluateTransform) :
    m_entity(eclass),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_rotationKey(RotationChangedCaller(*this)),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_funcStaticOrigin(m_traverse, m_origin),
    m_renderName(m_named, g_vector3_identity),
    m_skin(SkinChangedCaller(*this)),
    m_curveNURBS(boundsChanged),
    m_curveCatmullRom(boundsChanged),
    m_transformChanged(transformChanged),
    m_evaluateTransform(evaluateTransform),
    m_traversable(0)
  {
    construct();
  }
  Doom3Group(const Doom3Group& other, scene::Node& node, const Callback& transformChanged, const Callback& boundsChanged, const Callback& evaluateTransform) :
    m_entity(other.m_entity),
    m_originKey(OriginChangedCaller(*this)),
    m_origin(ORIGINKEY_IDENTITY),
    m_rotationKey(RotationChangedCaller(*this)),
    m_filter(m_entity, node),
    m_named(m_entity),
    m_nameKeys(m_entity),
    m_funcStaticOrigin(m_traverse, m_origin),
    m_renderName(m_named, g_vector3_identity),
    m_skin(SkinChangedCaller(*this)),
    m_curveNURBS(boundsChanged),
    m_curveCatmullRom(boundsChanged),
    m_transformChanged(transformChanged),
    m_evaluateTransform(evaluateTransform),
    m_traversable(0)
  {
    construct();
  }
  ~Doom3Group()
  {
    destroy();
  }

  InstanceCounter m_instanceCounter;
  void instanceAttach(const scene::Path& path)
  {
    if(++m_instanceCounter.m_count == 1)
    {
      m_filter.instanceAttach();
      m_entity.instanceAttach(path_find_mapfile(path.begin(), path.end()));
      m_traverse.instanceAttach(path_find_mapfile(path.begin(), path.end()));

      m_funcStaticOrigin.enable();
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      m_funcStaticOrigin.disable();

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
    return *m_traversable;
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
  ModelSkin& getModelSkin()
  {
    return m_skin.get();
  }

  void attach(scene::Traversable::Observer* observer)
  {
    m_traverseObservers.attach(*observer);
  }
  void detach(scene::Traversable::Observer* observer)
  {
    m_traverseObservers.detach(*observer);
  }

  const AABB& localAABB() const
  {
    m_curveBounds = m_curveNURBS.m_bounds;
    aabb_extend_by_aabb_safe(m_curveBounds, m_curveCatmullRom.m_bounds);
    return m_curveBounds;
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld, bool selected) const
  {
    if(isModel() && selected)
    {
      m_renderOrigin.render(renderer, volume, localToWorld);
    }

    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eWireframeOnly);
    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eFullMaterials);

    if(!m_curveNURBS.m_renderCurve.m_vertices.empty())
    {
      renderer.addRenderable(m_curveNURBS.m_renderCurve, localToWorld);
    }
    if(!m_curveCatmullRom.m_renderCurve.m_vertices.empty())
    {
      renderer.addRenderable(m_curveCatmullRom.m_renderCurve, localToWorld);
    }
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld, bool selected) const
  {
    renderSolid(renderer, volume, localToWorld, selected);
    if(g_showNames && isModel())
    {
      renderer.addRenderable(m_renderName, localToWorld);
    }
  }

  void testSelect(Selector& selector, SelectionTest& test, SelectionIntersection& best)
  {
    PointVertexArray_testSelect(&m_curveNURBS.m_renderCurve.m_vertices[0], m_curveNURBS.m_renderCurve.m_vertices.size(), test, best);
    PointVertexArray_testSelect(&m_curveCatmullRom.m_renderCurve.m_vertices[0], m_curveCatmullRom.m_renderCurve.m_vertices.size(), test, best);
  }

  void translate(const Vector3& translation)
  {
    m_origin = origin_translated(m_origin, translation);
  }
  void rotate(const Quaternion& rotation)
  {
    rotation_rotate(m_rotation, rotation);
  }
  void snapto(float snap)
  {
    m_originKey.m_origin = origin_snapped(m_originKey.m_origin, snap);
    m_originKey.write(&m_entity);
  }
  void revertTransform()
  {
    m_origin = m_originKey.m_origin;
    rotation_assign(m_rotation, m_rotationKey.m_rotation);
    m_curveNURBS.m_controlPointsTransformed = m_curveNURBS.m_controlPoints;
    m_curveCatmullRom.m_controlPointsTransformed = m_curveCatmullRom.m_controlPoints;
  }
  void freezeTransform()
  {
    m_originKey.m_origin = m_origin;
    m_originKey.write(&m_entity);
    rotation_assign(m_rotationKey.m_rotation, m_rotation);
    m_rotationKey.write(&m_entity);
    m_curveNURBS.m_controlPoints = m_curveNURBS.m_controlPointsTransformed;
    ControlPoints_write(m_curveNURBS.m_controlPoints, curve_Nurbs, m_entity);
    m_curveCatmullRom.m_controlPoints = m_curveCatmullRom.m_controlPointsTransformed;
    ControlPoints_write(m_curveCatmullRom.m_controlPoints, curve_CatmullRomSpline, m_entity);
  }
  void transformChanged()
  {
    revertTransform();
    m_evaluateTransform();
    updateTransform();
    m_curveNURBS.curveChanged();
    m_curveCatmullRom.curveChanged();
  }
  typedef MemberCaller<Doom3Group, &Doom3Group::transformChanged> TransformChangedCaller;
};

class ControlPointAddBounds
{
  AABB& m_bounds;
public:
  ControlPointAddBounds(AABB& bounds) : m_bounds(bounds)
  {
  }
  void operator()(const Vector3& point) const
  {
    aabb_extend_by_point_safe(m_bounds, point);
  }
};

class Doom3GroupInstance :
  public TargetableInstance,
  public TransformModifier,
  public Renderable,
  public SelectionTestable,
  public ComponentSelectionTestable,
  public ComponentEditable,
  public ComponentSnappable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      m_casts = TargetableInstance::StaticTypeCasts::instance().get();
      InstanceContainedCast<Doom3GroupInstance, Bounded>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, Renderable>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, ComponentSelectionTestable>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, ComponentEditable>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, ComponentSnappable>::install(m_casts);
      InstanceStaticCast<Doom3GroupInstance, Transformable>::install(m_casts);
      InstanceIdentityCast<Doom3GroupInstance>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  Doom3Group& m_contained;
  CurveEdit m_curveNURBS;
  CurveEdit m_curveCatmullRom;
  mutable AABB m_aabb_component;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;


  Bounded& get(NullType<Bounded>)
  {
    return m_contained;
  }

  STRING_CONSTANT(Name, "Doom3GroupInstance");

  Doom3GroupInstance(const scene::Path& path, scene::Instance* parent, Doom3Group& contained) :
    TargetableInstance(path, parent, this, StaticTypeCasts::instance().get(), contained.getEntity(), *this),
    TransformModifier(Doom3Group::TransformChangedCaller(contained), ApplyTransformCaller(*this)),
    m_contained(contained),
    m_curveNURBS(m_contained.m_curveNURBS.m_controlPointsTransformed, SelectionChangedComponentCaller(*this)),
    m_curveCatmullRom(m_contained.m_curveCatmullRom.m_controlPointsTransformed, SelectionChangedComponentCaller(*this))
  {
    m_contained.instanceAttach(Instance::path());
    m_contained.m_curveNURBSChanged = m_contained.m_curveNURBS.connect(CurveEdit::CurveChangedCaller(m_curveNURBS));
    m_contained.m_curveCatmullRomChanged = m_contained.m_curveCatmullRom.connect(CurveEdit::CurveChangedCaller(m_curveCatmullRom));

    StaticRenderableConnectionLines::instance().attach(*this);
  }
  ~Doom3GroupInstance()
  {
    StaticRenderableConnectionLines::instance().detach(*this);

    m_contained.m_curveCatmullRom.disconnect(m_contained.m_curveCatmullRomChanged);
    m_contained.m_curveNURBS.disconnect(m_contained.m_curveNURBSChanged);
    m_contained.instanceDetach(Instance::path());
  }
  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderSolid(renderer, volume, Instance::localToWorld(), getSelectable().isSelected());

    m_curveNURBS.renderComponentsSelected(renderer, volume, localToWorld());
    m_curveCatmullRom.renderComponentsSelected(renderer, volume, localToWorld());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_contained.renderWireframe(renderer, volume, Instance::localToWorld(), getSelectable().isSelected());

    m_curveNURBS.renderComponentsSelected(renderer, volume, localToWorld());
    m_curveCatmullRom.renderComponentsSelected(renderer, volume, localToWorld());
  }
  void renderComponents(Renderer& renderer, const VolumeTest& volume) const
  {
    if(GlobalSelectionSystem().ComponentMode() == SelectionSystem::eVertex)
    {
      m_curveNURBS.renderComponents(renderer, volume, localToWorld());
      m_curveCatmullRom.renderComponents(renderer, volume, localToWorld());
    }
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    test.BeginMesh(localToWorld());
    SelectionIntersection best;

    m_contained.testSelect(selector, test, best);

    if(best.valid())
    {
      Selector_add(selector, getSelectable(), best);
    }
  }

  bool isSelectedComponents() const
  {
    return m_curveNURBS.isSelected() || m_curveCatmullRom.isSelected();
  }
  void setSelectedComponents(bool selected, SelectionSystem::EComponentMode mode)
  {
    if(mode == SelectionSystem::eVertex)
    {
      m_curveNURBS.setSelected(selected);
      m_curveCatmullRom.setSelected(selected);
    }
  }
  void testSelectComponents(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode)
  {
    if(mode == SelectionSystem::eVertex)
    {
      test.BeginMesh(localToWorld());
      m_curveNURBS.testSelect(selector, test);
      m_curveCatmullRom.testSelect(selector, test);
    }
  }

  void transformComponents(const Matrix4& matrix)
  {
    if(m_curveNURBS.isSelected())
    {
      m_curveNURBS.transform(matrix);
    }
    if(m_curveCatmullRom.isSelected())
    {
      m_curveCatmullRom.transform(matrix);
    }
  }

  const AABB& getSelectedComponentsBounds() const
  {
    m_aabb_component = AABB();
    m_curveNURBS.forEachSelected(ControlPointAddBounds(m_aabb_component));
    m_curveCatmullRom.forEachSelected(ControlPointAddBounds(m_aabb_component));
    return m_aabb_component;
  }

  void snapComponents(float snap)
  {
    if(m_curveNURBS.isSelected())
    {
      m_curveNURBS.snapto(snap);
      m_curveNURBS.write(curve_Nurbs, m_contained.getEntity());
    }
    if(m_curveCatmullRom.isSelected())
    {
      m_curveCatmullRom.snapto(snap);
      m_curveCatmullRom.write(curve_CatmullRomSpline, m_contained.getEntity());
    }
  }

  void evaluateTransform()
  {
    if(getType() == TRANSFORM_PRIMITIVE)
    {
      m_contained.translate(getTranslation());
      m_contained.rotate(getRotation());
    }
    else
    {
      transformComponents(calculateTransform());
    }
  }
  void applyTransform()
  {
    m_contained.revertTransform();
    evaluateTransform();
    m_contained.freezeTransform();
  }
  typedef MemberCaller<Doom3GroupInstance, &Doom3GroupInstance::applyTransform> ApplyTransformCaller;

  void selectionChangedComponent(const Selectable& selectable)
  {
    GlobalSelectionSystem().getObserver(SelectionSystem::eComponent)(selectable);
    GlobalSelectionSystem().onComponentSelection(*this, selectable);
  }
  typedef MemberCaller1<Doom3GroupInstance, const Selectable&, &Doom3GroupInstance::selectionChangedComponent> SelectionChangedComponentCaller;
};

class Doom3GroupNode :
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
      NodeStaticCast<Doom3GroupNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<Doom3GroupNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, scene::Traversable>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, Snappable>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, TransformNode>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, Entity>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, Nameable>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, Namespaced>::install(m_casts);
      NodeContainedCast<Doom3GroupNode, ModelSkin>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  Doom3Group m_contained;

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
  ModelSkin& get(NullType<ModelSkin>)
  {
    return m_contained.getModelSkin();
  }

  Doom3GroupNode(EntityClass* eclass) :
    m_node(this, this, StaticTypeCasts::instance().get()),
      m_contained(eclass, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSet::BoundsChangedCaller(m_instances), InstanceSetEvaluateTransform<Doom3GroupInstance>::Caller(m_instances))
  {
    construct();
  }
  Doom3GroupNode(const Doom3GroupNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    scene::Traversable::Observer(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_contained(other.m_contained, m_node, InstanceSet::TransformChangedCaller(m_instances), InstanceSet::BoundsChangedCaller(m_instances), InstanceSetEvaluateTransform<Doom3GroupInstance>::Caller(m_instances))
  {
    construct();
  }
  ~Doom3GroupNode()
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
    return (new Doom3GroupNode(*this))->node();
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
    return new Doom3GroupInstance(path, parent, m_contained);
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

void Doom3Group_construct()
{
  CurveEdit::Type::instance().m_controlsShader = GlobalShaderCache().capture("$POINT");
  CurveEdit::Type::instance().m_selectedShader = GlobalShaderCache().capture("$SELPOINT");
}

void Doom3Group_destroy()
{
  GlobalShaderCache().release("$SELPOINT");
  GlobalShaderCache().release("$POINT");
}

scene::Node& New_Doom3Group(EntityClass* eclass)
{
  return (new Doom3GroupNode(eclass))->node();
}
