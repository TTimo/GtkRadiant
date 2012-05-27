
#if !defined (INCLUDED_TRAVERSELIB_H)
#define INCLUDED_TRAVERSELIB_H

#include "debugging/debugging.h"

#include "scenelib.h"
#include "undolib.h"
#include "container/container.h"

#include <list>
#include <vector>
#include <algorithm>

class TraversableObserverInsertOutputIterator 
{
protected:
  scene::Traversable::Observer* m_observer;
public:
  typedef std::output_iterator_tag iterator_category;
  typedef void difference_type;
  typedef void value_type;
  typedef void pointer;
  typedef void reference;

  TraversableObserverInsertOutputIterator(scene::Traversable::Observer* observer) 
    : m_observer(observer)
  {
  }
  TraversableObserverInsertOutputIterator& operator=(const NodeReference& node)
  { 
    m_observer->insert(node);
    return *this;
  }
  TraversableObserverInsertOutputIterator& operator*() { return *this; }
  TraversableObserverInsertOutputIterator& operator++() { return *this; }
  TraversableObserverInsertOutputIterator& operator++(int) { return *this; }
};

class TraversableObserverEraseOutputIterator 
{
protected:
  scene::Traversable::Observer* m_observer;
public:
  typedef std::output_iterator_tag iterator_category;
  typedef void difference_type;
  typedef void value_type;
  typedef void pointer;
  typedef void reference;

  TraversableObserverEraseOutputIterator(scene::Traversable::Observer* observer) 
    : m_observer(observer)
  {
  }
  TraversableObserverEraseOutputIterator& operator=(const NodeReference& node)
  { 
    m_observer->erase(node);
    return *this;
  }
  TraversableObserverEraseOutputIterator& operator*() { return *this; }
  TraversableObserverEraseOutputIterator& operator++() { return *this; }
  TraversableObserverEraseOutputIterator& operator++(int) { return *this; }
};

typedef UnsortedSet<NodeSmartReference> UnsortedNodeSet;

/// \brief Calls \p observer->\c insert for each node that exists only in \p other and \p observer->\c erase for each node that exists only in \p self
inline void nodeset_diff(const UnsortedNodeSet& self, const UnsortedNodeSet& other, scene::Traversable::Observer* observer)
{
  std::vector<NodeReference> sorted(self.begin(), self.end());
  std::vector<NodeReference> other_sorted(other.begin(), other.end());

  std::sort(sorted.begin(), sorted.end());
  std::sort(other_sorted.begin(), other_sorted.end());

  std::set_difference(sorted.begin(), sorted.end(), other_sorted.begin(), other_sorted.end(), TraversableObserverEraseOutputIterator(observer));
  std::set_difference(other_sorted.begin(), other_sorted.end(), sorted.begin(), sorted.end(), TraversableObserverInsertOutputIterator(observer));
}

/// \brief A sequence of node references which notifies an observer of inserts and deletions, and uses the global undo system to provide undo for modifications.
class TraversableNodeSet : public scene::Traversable
{
  UnsortedNodeSet m_children;
  UndoableObject<TraversableNodeSet> m_undo;
  Observer* m_observer;

  void copy(const TraversableNodeSet& other)
  {
    m_children = other.m_children;
  }
  void notifyInsertAll()
  {
    if(m_observer)
    {
      for(UnsortedNodeSet::iterator i = m_children.begin(); i != m_children.end(); ++i)
      {
        m_observer->insert(*i);
      }
    }
  }
  void notifyEraseAll()
  {
    if(m_observer)
    {
      for(UnsortedNodeSet::iterator i = m_children.begin(); i != m_children.end(); ++i)
      {
        m_observer->erase(*i);
      }
    }
  }
public:
  TraversableNodeSet()
    : m_undo(*this), m_observer(0)
  {
  }
  TraversableNodeSet(const TraversableNodeSet& other)
    : scene::Traversable(other), m_undo(*this), m_observer(0)
  {
    copy(other);
    notifyInsertAll();
  }
  ~TraversableNodeSet()
  {
    notifyEraseAll();
  }
  TraversableNodeSet& operator=(const TraversableNodeSet& other)
  {
#if 1 // optimised change-tracking using diff algorithm
    if(m_observer)
    {
      nodeset_diff(m_children, other.m_children, m_observer);
    }
    copy(other);
#else
    TraversableNodeSet tmp(other);
    tmp.swap(*this);
#endif
    return *this;
  }
  void swap(TraversableNodeSet& other)
  {
    std::swap(m_children, other.m_children);
    std::swap(m_observer, other.m_observer);
  }

  void attach(Observer* observer)
  {
    ASSERT_MESSAGE(m_observer == 0, "TraversableNodeSet::attach: observer cannot be attached");
    m_observer = observer;
    notifyInsertAll();
  }
  void detach(Observer* observer)
  {
    ASSERT_MESSAGE(m_observer == observer, "TraversableNodeSet::detach: observer cannot be detached");
    notifyEraseAll();
    m_observer = 0;
  }
  /// \brief \copydoc scene::Traversable::insert()
  void insert(scene::Node& node)
  {
    ASSERT_MESSAGE(&node != 0, "TraversableNodeSet::insert: sanity check failed");
    m_undo.save();

    ASSERT_MESSAGE(m_children.find(NodeSmartReference(node)) == m_children.end(), "TraversableNodeSet::insert - element already exists");

    m_children.insert(NodeSmartReference(node));

    if(m_observer)
    {
      m_observer->insert(node);
    }
  }
  /// \brief \copydoc scene::Traversable::erase()
  void erase(scene::Node& node)
  {
    ASSERT_MESSAGE(&node != 0, "TraversableNodeSet::erase: sanity check failed");
    m_undo.save();

    ASSERT_MESSAGE(m_children.find(NodeSmartReference(node)) != m_children.end(), "TraversableNodeSet::erase - failed to find element");

    if(m_observer)
    {
      m_observer->erase(node);
    }

    m_children.erase(NodeSmartReference(node));
  }
  /// \brief \copydoc scene::Traversable::traverse()
  void traverse(const Walker& walker)
  {
    UnsortedNodeSet::iterator i = m_children.begin();
    while(i != m_children.end())
    {
      // post-increment the iterator
      Node_traverseSubgraph(*i++, walker);
      // the Walker can safely remove the current node from
      // this container without invalidating the iterator
    }
  }
  /// \brief \copydoc scene::Traversable::empty()
  bool empty() const
  {
    return m_children.empty();
  }

  void instanceAttach(MapFile* map)
  {
    m_undo.instanceAttach(map);
  }
  void instanceDetach(MapFile* map)
  {
    m_undo.instanceDetach(map);
  }
};

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap.
  inline void swap(TraversableNodeSet& self, TraversableNodeSet& other)
  {
    self.swap(other);
  }
}


class TraversableNode : public scene::Traversable
{
public:
  TraversableNode() : m_node(0), m_observer(0)
  {
  }

  // traverse
  void attach(Observer* observer)
  {
    ASSERT_MESSAGE(m_observer == 0, "TraversableNode::attach - cannot attach observer");
    m_observer = observer;
    if(m_node != 0)
    {
      m_observer->insert(*m_node);
    }
  }
  void detach(Observer* observer)
  {
    ASSERT_MESSAGE(m_observer == observer, "TraversableNode::detach - cannot detach observer");
    if(m_node != 0)
    {
      m_observer->erase(*m_node);
    }
    m_observer = 0;
  }
  void insert(scene::Node& node)
  {
    ASSERT_MESSAGE(m_node == 0, "TraversableNode::insert - element already exists");

    m_node = &node;
    node.IncRef();

    if(m_observer != 0)
    {
      m_observer->insert(node);
    }
  }
  void erase(scene::Node& node)
  {
    ASSERT_MESSAGE(m_node == &node, "TraversableNode::erase - failed to find element");

    if(m_observer != 0)
    {
      m_observer->erase(node);
    }

    m_node = 0;
    node.DecRef();
  }
  void traverse(const scene::Traversable::Walker& walker)
  {
    if(m_node != 0)
    {
      Node_traverseSubgraph(*m_node, walker);
    }
  }
  bool empty() const
  {
    return m_node != 0;
  }

  scene::Node& get()
  {
    return *m_node;
  }

private:
  scene::Node* m_node;
  Observer* m_observer;
};


class TraversableObserverPairRelay : public scene::Traversable::Observer
{
  scene::Traversable::Observer* m_first;
  scene::Traversable::Observer* m_second;
public:
  TraversableObserverPairRelay() : m_first(0), m_second(0)
  {
  }
  void attach(scene::Traversable::Observer* observer)
  {
    ASSERT_MESSAGE(m_first == 0 || m_second == 0, "TraversableObserverPair::attach: cannot attach observer");
    if(m_first == 0)
    {
      m_first = observer;
    }
    else if(m_second == 0)
    {
      m_second = observer;
    }
  }
  void detach(scene::Traversable::Observer* observer)
  {
    ASSERT_MESSAGE(m_first == observer || m_second == observer, "TraversableObserverPair::detach: cannot detach observer");
    if(m_first == observer)
    {
      m_first = 0;
    }
    else if(m_second == observer)
    {
      m_second = 0;
    }
  }
  void insert(scene::Node& node)
  {
    if(m_first != 0)
    {
      m_first->insert(node);
    }
    if(m_second != 0)
    {
      m_second->insert(node);
    }
  }
  void erase(scene::Node& node)
  {
    if(m_second != 0)
    {
      m_second->erase(node);
    }
    if(m_first != 0)
    {
      m_first->erase(node);
    }
  }
};


#endif
