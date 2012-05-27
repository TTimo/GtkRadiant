
#if !defined (INCLUDED_SCENELIB_H)
#define INCLUDED_SCENELIB_H

#include "iscenegraph.h"
#include "iselection.h"

#include "warnings.h"
#include <cstddef>
#include <string.h>

#include "math/aabb.h"
#include "transformlib.h"
#include "generic/callback.h"
#include "generic/reference.h"
#include "container/stack.h"
#include "typesystem.h"

class Selector;
class SelectionTest;
class VolumeTest;
template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
class Matrix4;
class Vector4;
typedef Vector4 Quaternion;
class AABB;

class ComponentSelectionTestable
{
public:
  static const char* getTypeName()
  {
    return "ComponentSelectionTestable";
  }

  virtual void setSelectedComponents(bool select, SelectionSystem::EComponentMode mode) = 0;
  virtual void testSelectComponents(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode) = 0;
};

class ComponentEditable
{
public:
  static const char* getTypeName()
  {
    return "ComponentEditable";
  }

  virtual const AABB& getSelectedComponentsBounds() const = 0;
  virtual void translateComponents(const Vector3& translation) = 0;
  virtual void rotateComponents(const Quaternion& rotation) = 0;
  virtual void scaleComponents(const Vector3& scaling) = 0;
  virtual void snapComponents(float snap) = 0;
  virtual void freezeComponents() = 0;
};

class Bounded
{
public:
  static const char* getTypeName()
  {
    return "Bounded";
  }

  virtual const AABB& localAABB() const = 0;
};

class BrushDoom3
{
public:
  static const char* getTypeName()
  {
    return "BrushDoom3";
  }

  virtual void setDoom3GroupOrigin(const Vector3& origin) = 0;
};




typedef TypeCastTable<NODETYPEID_MAX> NodeTypeCastTable;

template<typename Type>
class NodeType : public StaticTypeSystemInitialiser
{
  TypeId m_typeId;
public:
  static const char* getTypeName()
  {
    return Type::getTypeName();
  }
  NodeType() : m_typeId(NODETYPEID_NONE)
  {
    StaticTypeSystemInitialiser::instance().addInitialiser(InitialiseCaller(*this));
  }
  void initialise()
  {
    m_typeId = GlobalSceneGraph().getNodeTypeId(getTypeName());
  }
  typedef MemberCaller<NodeType<Type>, &NodeType<Type>::initialise> InitialiseCaller;
  TypeId getTypeId()
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(m_typeId != NODETYPEID_NONE, "node-type " << makeQuoted(getTypeName()) << " used before being initialised");
#endif
    return m_typeId;
  }
};

template<typename Type>
class StaticNodeType
{
public:
  enum { SIZE = NODETYPEID_MAX };
  static TypeId getTypeId()
  {
    return Static< NodeType<Type> >::instance().getTypeId();
  }
};

template<typename Type, typename Base>
class NodeStaticCast :
  public CastInstaller<
    StaticNodeType<Base>,
    StaticCast<Type, Base>
  >
{
};

template<typename Type, typename Contained>
class NodeContainedCast :
  public CastInstaller<
    StaticNodeType<Contained>,
    ContainedCast<Type, Contained>
  >
{
};

template<typename Type>
class NodeIdentityCast :
  public CastInstaller<
    StaticNodeType<Type>,
    IdentityCast<Type>
  >
{
};

namespace scene
{
  class Node
  {
  public:
    enum { eVisible = 0 };
    enum { eHidden = 1 << 0 };
    enum { eFiltered = 1 << 1 };
    enum { eExcluded = 1 << 2 };

    class Symbiot
    {
    public:
      virtual void release() = 0;
    };

  private:
    unsigned int m_state;
    std::size_t m_refcount;
    Symbiot* m_symbiot;
    void* m_node;
    NodeTypeCastTable& m_casts;

  public:
    bool m_isRoot;

    bool isRoot()
    {
      return m_isRoot;
    }

    Node(Symbiot* symbiot, void* node, NodeTypeCastTable& casts) :
      m_state(eVisible),
      m_refcount(0),
      m_symbiot(symbiot),
      m_node(node),
      m_casts(casts),
      m_isRoot(false)
    {
    }
    ~Node()
    {
    }

    void IncRef()
    {
      ASSERT_MESSAGE(m_refcount < (1 << 24), "Node::decref: uninitialised refcount");
      ++m_refcount;
    }
    void DecRef()
    {
      ASSERT_MESSAGE(m_refcount < (1 << 24), "Node::decref: uninitialised refcount");
      if(--m_refcount == 0)
      {
        m_symbiot->release();
      }
    }
    std::size_t getReferenceCount() const
    {
      return m_refcount;
    }

    void* cast(TypeId typeId) const
    {
      return m_casts.cast(typeId, m_node);
    }

    void enable(unsigned int state)
    {
      m_state |= state;
    }
    void disable(unsigned int state)
    {
      m_state &= ~state;
    }
    bool visible()
    {
      return m_state == eVisible;
    }
    bool excluded()
    {
      return (m_state & eExcluded) != 0;
    }
  };

  class NullNode : public Node::Symbiot
  {
    NodeTypeCastTable m_casts;
    Node m_node;
  public:
    NullNode() : m_node(this, 0, m_casts)
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
  };
}

template<typename Type>
class NodeTypeCast
{
public:
  static Type* cast(scene::Node& node)
  {
    return static_cast<Type*>(node.cast(StaticNodeType<Type>::getTypeId()));
  }
  static const Type* cast(const scene::Node& node)
  {
    return static_cast<const Type*>(node.cast(StaticNodeType<Type>::getTypeId()));
  }
};


inline Transformable* Node_getTransformable(scene::Node& node)
{
  return NodeTypeCast<Transformable>::cast(node);
}

inline scene::Instantiable* Node_getInstantiable(scene::Node& node)
{
  return NodeTypeCast<scene::Instantiable>::cast(node);
}

inline scene::Traversable* Node_getTraversable(scene::Node& node)
{
  return NodeTypeCast<scene::Traversable>::cast(node);
}

inline void Node_traverseSubgraph(scene::Node& node, const scene::Traversable::Walker& walker)
{
  if(walker.pre(node))
  {
    scene::Traversable* traversable = Node_getTraversable(node);
    if(traversable != 0)
    {
      traversable->traverse(walker);
    }
  }
  walker.post(node);
}

inline bool operator<(scene::Node& node, scene::Node& other)
{
  return &node < &other;
}
inline bool operator==(scene::Node& node, scene::Node& other)
{
  return &node == &other;
}
inline bool operator!=(scene::Node& node, scene::Node& other)
{
  return !::operator==(node, other);
}


inline scene::Node& NewNullNode()
{
  return (new scene::NullNode)->node();
}

inline void Path_deleteTop(const scene::Path& path)
{
  Node_getTraversable(path.parent())->erase(path.top());
}





class delete_all : public scene::Traversable::Walker
{
  scene::Node& m_parent;
public:
  delete_all(scene::Node& parent) : m_parent(parent)
  {
  }
  bool pre(scene::Node& node) const
  {
    return false;
  }
  void post(scene::Node& node) const
  {
    Node_getTraversable(m_parent)->erase(node);
  }
};

inline void DeleteSubgraph(scene::Node& subgraph)
{
  Node_getTraversable(subgraph)->traverse(delete_all(subgraph));
}


class EntityUndefined
{
public:
  static const char* getTypeName()
  {
    return "Entity";
  }
};

inline bool Node_isEntity(scene::Node& node)
{
  return NodeTypeCast<EntityUndefined>::cast(node);
}

class BrushUndefined
{
public:
  static const char* getTypeName()
  {
    return "Brush";
  }
};

inline bool Node_isBrush(scene::Node& node)
{
  return NodeTypeCast<BrushUndefined>::cast(node) != 0;
}

class PatchUndefined
{
public:
  static const char* getTypeName()
  {
    return "Patch";
  }
};

inline bool Node_isPatch(scene::Node& node)
{
  return NodeTypeCast<PatchUndefined>::cast(node) != 0;
}

inline bool Node_isPrimitive(scene::Node& node)
{
#if 1
  return Node_isBrush(node) || Node_isPatch(node);
#else
  return !node.isRoot();
#endif
}

class ParentBrushes : public scene::Traversable::Walker
{
  scene::Node& m_parent;
public:
  ParentBrushes(scene::Node& parent)
    : m_parent(parent)
  {
  }
  bool pre(scene::Node& node) const
  {
    return false;
  }
  void post(scene::Node& node) const
  {
    if(Node_isPrimitive(node))
    {
      Node_getTraversable(m_parent)->insert(node);
    }
  }
};

inline void parentBrushes(scene::Node& subgraph, scene::Node& parent)
{
  Node_getTraversable(subgraph)->traverse(ParentBrushes(parent));
}

class HasBrushes : public scene::Traversable::Walker
{
  bool& m_hasBrushes;
public:
  HasBrushes(bool& hasBrushes)
    : m_hasBrushes(hasBrushes)
  {
    m_hasBrushes = true;
  }
  bool pre(scene::Node& node) const
  {
    if(!Node_isPrimitive(node))
    {
      m_hasBrushes = false;
    }
    return false;
  }
};

inline bool node_is_group(scene::Node& node)
{
  scene::Traversable* traversable = Node_getTraversable(node);
  if(traversable != 0)
  {
    bool hasBrushes = false;
    traversable->traverse(HasBrushes(hasBrushes));
    return hasBrushes;
  }
  return false;
}

typedef TypeCastTable<INSTANCETYPEID_MAX> InstanceTypeCastTable;

template<typename Type>
class InstanceType : public StaticTypeSystemInitialiser
{
  TypeId m_typeId;
public:
  static const char* getTypeName()
  {
    return Type::getTypeName();
  }
  InstanceType() : m_typeId(INSTANCETYPEID_NONE)
  {
    StaticTypeSystemInitialiser::instance().addInitialiser(InitialiseCaller(*this));
  }
  void initialise()
  {
    m_typeId = GlobalSceneGraph().getInstanceTypeId(getTypeName());
  }
  typedef MemberCaller<InstanceType<Type>, &InstanceType<Type>::initialise> InitialiseCaller;
  TypeId getTypeId()
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(m_typeId != INSTANCETYPEID_NONE, "instance-type " << makeQuoted(getTypeName()) << " used before being initialised");
#endif
    return m_typeId;
  }
};

template<typename Type>
class StaticInstanceType
{
public:
  enum { SIZE = INSTANCETYPEID_MAX };
  static TypeId getTypeId()
  {
    return Static< InstanceType<Type> >::instance().getTypeId();
  }
};

template<typename Type, typename Base>
class InstanceStaticCast :
  public CastInstaller<
    StaticInstanceType<Base>,
    StaticCast<Type, Base>
  >
{
};

template<typename Type, typename Contained>
class InstanceContainedCast :
  public CastInstaller<
    StaticInstanceType<Contained>,
    ContainedCast<Type, Contained>
  >
{
};

template<typename Type>
class InstanceIdentityCast :
  public CastInstaller<
    StaticInstanceType<Type>,
    IdentityCast<Type>
  >
{
};


inline Selectable* Instance_getSelectable(scene::Instance& instance);
inline const Selectable* Instance_getSelectable(const scene::Instance& instance);

inline Bounded* Instance_getBounded(scene::Instance& instance);
inline const Bounded* Instance_getBounded(const scene::Instance& instance);

namespace scene
{
  class Instance
  {
    class AABBAccumulateWalker : public scene::Graph::Walker
    {
      AABB& m_aabb;
      mutable std::size_t m_depth;
    public:
      AABBAccumulateWalker(AABB& aabb) : m_aabb(aabb), m_depth(0)
      {
      }
      bool pre(const scene::Path& path, scene::Instance& instance) const
      {
        if(m_depth == 1)
        {
          aabb_extend_by_aabb_safe(m_aabb, instance.worldAABB());
        }
        return ++m_depth != 2;
      }
      void post(const scene::Path& path, scene::Instance& instance) const
      {
        --m_depth;
      }
    };


    class TransformChangedWalker : public scene::Graph::Walker
    {
    public:
      bool pre(const scene::Path& path, scene::Instance& instance) const
      {
        instance.transformChangedLocal();
        return true;
      }
    };

    class ParentSelectedChangedWalker : public scene::Graph::Walker
    {
    public:
      bool pre(const scene::Path& path, scene::Instance& instance) const
      {
        instance.parentSelectedChanged();
        return true;
      }
    };

    class ChildSelectedWalker : public scene::Graph::Walker
    {
      bool& m_childSelected;
      mutable std::size_t m_depth;
    public:
      ChildSelectedWalker(bool& childSelected) : m_childSelected(childSelected), m_depth(0)
      {
        m_childSelected = false;
      }
      bool pre(const scene::Path& path, scene::Instance& instance) const
      {
        if(m_depth == 1 && !m_childSelected)
        {
          m_childSelected = instance.isSelected() || instance.childSelected();
        }
        return ++m_depth != 2;
      }
      void post(const scene::Path& path, scene::Instance& instance) const
      {
        --m_depth;
      }
    };

    Path m_path;
    Instance* m_parent;
    void* m_instance;
    InstanceTypeCastTable& m_casts;

    mutable Matrix4 m_local2world;
    mutable AABB m_bounds;
    mutable AABB m_childBounds;
    mutable bool m_transformChanged;
    mutable bool m_transformMutex;
    mutable bool m_boundsChanged;
    mutable bool m_boundsMutex;
    mutable bool m_childBoundsChanged;
    mutable bool m_childBoundsMutex;
    mutable bool m_isSelected;
    mutable bool m_isSelectedChanged;
    mutable bool m_childSelected;
    mutable bool m_childSelectedChanged;
    mutable bool m_parentSelected;
    mutable bool m_parentSelectedChanged;
    Callback m_childSelectedChangedCallback;
    Callback m_transformChangedCallback;


    void evaluateTransform() const
    {
      if(m_transformChanged)
      {
        ASSERT_MESSAGE(!m_transformMutex, "re-entering transform evaluation");
        m_transformMutex = true;

        m_local2world = (m_parent != 0) ? m_parent->localToWorld() : g_matrix4_identity;
        Transformable* transformable = Node_getTransformable(m_path.top());
        if(transformable != 0)
        {
          matrix4_multiply_by_matrix4(m_local2world, transformable->localToParent());
        }

        m_transformMutex = false;
        m_transformChanged = false;
      }
    }
    void evaluateChildBounds() const
    {
      if(m_childBoundsChanged)
      {
        ASSERT_MESSAGE(!m_childBoundsMutex, "re-entering bounds evaluation");
        m_childBoundsMutex = true;

        m_childBounds = AABB();

        GlobalSceneGraph().traverse_subgraph(AABBAccumulateWalker(m_childBounds), m_path);

        m_childBoundsMutex = false;
        m_childBoundsChanged = false;
      }
    }
    void evaluateBounds() const
    {
      if(m_boundsChanged)
      {
        ASSERT_MESSAGE(!m_boundsMutex, "re-entering bounds evaluation");
        m_boundsMutex = true;

        m_bounds = childBounds();

        const Bounded* bounded = Instance_getBounded(*this);
        if(bounded != 0)
        {
          aabb_extend_by_aabb_safe(
            m_bounds,
            aabb_for_oriented_aabb_safe(bounded->localAABB(), localToWorld())
          );
        }

        m_boundsMutex = false;
        m_boundsChanged = false;
      }
    }

    Instance(const scene::Instance& other);
    Instance& operator=(const scene::Instance& other);
  public:

    Instance(const scene::Path& path, Instance* parent, void* instance, InstanceTypeCastTable& casts) :
      m_path(path),
      m_parent(parent),
      m_instance(instance),
      m_casts(casts),
      m_transformChanged(true),
      m_transformMutex(false),
      m_boundsChanged(true),
      m_boundsMutex(false),
      m_childBoundsChanged(true),
      m_childBoundsMutex(false),
      m_isSelectedChanged(true),
      m_childSelectedChanged(true),
      m_parentSelectedChanged(true)
    {
      ASSERT_MESSAGE((parent == 0) == (path.size() == 1), "instance has invalid parent");
    }
    virtual ~Instance()
    {
    }

    const scene::Path& path() const
    {
      return m_path;
    }

    void* cast(TypeId typeId) const
    {
      return m_casts.cast(typeId, m_instance);
    }

    const Matrix4& localToWorld() const
    {
      evaluateTransform();
      return m_local2world;
    }
    void transformChangedLocal()
    {
      ASSERT_NOTNULL(m_parent);
      m_transformChanged = true;
      m_boundsChanged = true;
      m_childBoundsChanged = true;
      m_transformChangedCallback();
    }
    void transformChanged()
    {
      GlobalSceneGraph().traverse_subgraph(TransformChangedWalker(), m_path);
      boundsChanged();
    }
    void setTransformChangedCallback(const Callback& callback)
    {
      m_transformChangedCallback = callback;
    }


    const AABB& worldAABB() const
    {
      evaluateBounds();
      return m_bounds;
    }
    const AABB& childBounds() const
    {
      evaluateChildBounds();
      return m_childBounds;
    }
    void boundsChanged()
    {
      m_boundsChanged = true;
      m_childBoundsChanged = true;
      if(m_parent != 0)
      {
        m_parent->boundsChanged();
      }
      GlobalSceneGraph().boundsChanged();
    }

    void childSelectedChanged()
    {
      m_childSelectedChanged = true;
      m_childSelectedChangedCallback();
      selectedChanged();
    }
    bool childSelected() const
    {
      if(m_childSelectedChanged)
      {
        m_childSelectedChanged = false;
        GlobalSceneGraph().traverse_subgraph(ChildSelectedWalker(m_childSelected), m_path);
      }
      return m_childSelected;
    }

    void setChildSelectedChangedCallback(const Callback& callback)
    {
      m_childSelectedChangedCallback = callback;
    }
    void selectedChanged()
    {
      m_isSelectedChanged = true;
      if(m_parent != 0)
      {
        m_parent->childSelectedChanged();
      }
      GlobalSceneGraph().traverse_subgraph(ParentSelectedChangedWalker(), m_path);
    }
    bool isSelected() const
    {
      if(m_isSelectedChanged)
      {
        m_isSelectedChanged = false;
        const Selectable* selectable = Instance_getSelectable(*this);
        m_isSelected = selectable != 0 && selectable->isSelected();
      }
      return m_isSelected;
    }

    void parentSelectedChanged()
    {
      m_parentSelectedChanged = true;
    }
    bool parentSelected() const
    {
      if(m_parentSelectedChanged)
      {
        m_parentSelectedChanged = false;
        m_parentSelected = m_parent != 0 && (m_parent->isSelected() || m_parent->parentSelected());
      }
      return m_parentSelected;
    }
  };
}

template<typename Type>
class InstanceTypeCast
{
public:
  static Type* cast(scene::Instance& instance)
  {
    return static_cast<Type*>(instance.cast(StaticInstanceType<Type>::getTypeId()));
  }
  static const Type* cast(const scene::Instance& instance)
  {
    return static_cast<const Type*>(instance.cast(StaticInstanceType<Type>::getTypeId()));
  }
};

inline Selectable* Instance_getSelectable(scene::Instance& instance)
{
  return InstanceTypeCast<Selectable>::cast(instance);
}
inline const Selectable* Instance_getSelectable(const scene::Instance& instance)
{
  return InstanceTypeCast<Selectable>::cast(instance);
}

inline Bounded* Instance_getBounded(scene::Instance& instance)
{
  return InstanceTypeCast<Bounded>::cast(instance);
}
inline const Bounded* Instance_getBounded(const scene::Instance& instance)
{
  return InstanceTypeCast<Bounded>::cast(instance);
}


inline ComponentSelectionTestable* Instance_getComponentSelectionTestable(scene::Instance& instance)
{
  return InstanceTypeCast<ComponentSelectionTestable>::cast(instance);
}

inline ComponentEditable* Instance_getComponentEditable(scene::Instance& instance)
{
  return InstanceTypeCast<ComponentEditable>::cast(instance);
}



class InstanceCounter
{
public:
  unsigned int m_count;
  InstanceCounter() : m_count(0)
  {
  }
};


class Counter
{
public:
  virtual void increment() = 0;
  virtual void decrement() = 0;
};

#include "generic/callback.h"

class SimpleCounter : public Counter
{
  Callback m_countChanged;
  std::size_t m_count;
public:
  void setCountChangedCallback(const Callback& countChanged)
  {
    m_countChanged = countChanged;
  }
  void increment()
  {
    ++m_count;
    m_countChanged();
  }
  void decrement()
  {
    --m_count;
    m_countChanged();
  }
  std::size_t get() const
  {
    return m_count;
  }
};


template<typename Contained>
class ConstReference;
typedef ConstReference<scene::Path> PathConstReference;

#include "generic/referencecounted.h"
typedef SmartReference<scene::Node, IncRefDecRefCounter<scene::Node> > NodeSmartReference;


#endif
