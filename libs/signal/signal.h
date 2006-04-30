
#if !defined(INCLUDED_SIGNAL_H)
#define INCLUDED_SIGNAL_H

#include "isignal.h"
#include "memory/allocator.h"
#include "debugging/debugging.h"
#include <iterator>

namespace ListDetail
{
  struct ListNodeBase
  {
    ListNodeBase* next;
    ListNodeBase* prev;
  };

  inline void list_initialise(ListNodeBase& self)
  {
    self.next = self.prev = &self;
  }

  inline void list_swap(ListNodeBase& self, ListNodeBase& other)
  {
    ListNodeBase tmp(self);
    if(other.next == &other)
    {
      list_initialise(self);
    }
    else
    {
      self = other;
      self.next->prev = self.prev->next = &self;
    }
    if(tmp.next == &self)
    {
      list_initialise(other);
    }
    else
    {
      other = tmp;
      other.next->prev = other.prev->next = &other;
    }
  }

  inline void node_link(ListNodeBase* node, ListNodeBase* next)
  {
    node->next = next;
    node->prev = next->prev;
    next->prev = node;
    node->prev->next = node;
  }
  inline void node_unlink(ListNodeBase* node)
  {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  template<typename Value>
  struct ListNode : public ListNodeBase
  {
    Value value;

    ListNode(const Value& value) : value(value) 
    {
    }
    ListNode* getNext() const
    {
      return static_cast<ListNode*>(next);
    }
    ListNode* getPrev() const
    {
      return static_cast<ListNode*>(prev);
    }
  };

  template<typename Type>
  class NonConstTraits
  {
  public:
    typedef Type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;

    template<typename Other>
    struct rebind
    {
      typedef NonConstTraits<Other> other;
    };
  };

  template<typename Type>
  class ConstTraits
  {
  public:
    typedef Type value_type;
    typedef const value_type* pointer;
    typedef const value_type& reference;

    template<typename Other>
    struct rebind
    {
      typedef ConstTraits<Other> other;
    };
  };

  template<typename Traits>
  class ListIterator
  {
  public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef std::ptrdiff_t difference_type;
    typedef difference_type distance_type;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::reference reference;

  private:
    typedef ListNode<value_type> Node;
    typedef typename Traits::template rebind<Node>::other NodeTraits;
    typedef typename NodeTraits::pointer NodePointer;
    typedef typename Traits::template rebind< Opaque<value_type> >::other OpaqueTraits;
    typedef typename OpaqueTraits::pointer OpaquePointer;
    NodePointer m_node;

    void increment()
    {
      m_node = m_node->getNext();
    }
    void decrement()
    {
      m_node = m_node->getPrev();
    }


 public:
   explicit ListIterator(NodePointer node) : m_node(node)
    {
    }
    explicit ListIterator(OpaquePointer p) : m_node(reinterpret_cast<NodePointer>(p))
    {
    }

    NodePointer node()
    {
      return m_node;
    }
    OpaquePointer opaque() const
    {
      return reinterpret_cast<OpaquePointer>(m_node);
    }

    bool operator==(const ListIterator& other) const
    {
      return m_node == other.m_node;
    }
    bool operator!=(const ListIterator& other) const
    {
      return !operator==(other);
    }
    ListIterator& operator++()
    {
      increment();
      return *this;
    }
    ListIterator operator++(int)
    {
      ListIterator tmp = *this;
      increment();
      return tmp;
    }
    ListIterator& operator--()
    {
      decrement();
      return *this;
    }
    ListIterator operator--(int)
    {
      ListIterator tmp = *this;
      decrement();
      return tmp;
    }
    reference operator*() const
    {
      return m_node->value;
    }
    pointer operator->() const
    {
      return &(operator*());
    }
  };
}

template<typename Value, typename Allocator = DefaultAllocator<Value> >
class List : private Allocator
{
  typedef ListDetail::ListNode<Value> Node;
  ListDetail::ListNodeBase list;
  typedef typename Allocator::template rebind<Node>::other NodeAllocator;

  Node* newNode(const Value& value)
  {
    return new (NodeAllocator(*this).allocate(1)) Node(value);
  }
  void deleteNode(Node* node)
  {
    node->~Node();
    NodeAllocator(*this).deallocate(node, 1);
  }
public:
  typedef Value value_type;
  typedef ListDetail::ListIterator< ListDetail::NonConstTraits<Value> > iterator;
  typedef ListDetail::ListIterator< ListDetail::ConstTraits<Value> > const_iterator;

  List()
  {
    list_initialise(list);
  }
  explicit List(const Allocator& allocator) : Allocator(allocator)
  {
    list_initialise(list);
  }
  ~List()
  {
    for(; list.next != &list;)
    {
      Node* node = static_cast<Node*>(list.next);
      list.next = list.next->next;
      deleteNode(node);
    }
  }
  iterator begin()
  {
    return iterator(static_cast<Node*>(list.next));
  }
  iterator end()
  {
    return iterator(static_cast<Node*>(&list));
  }
  const_iterator begin() const
  {
    return const_iterator(static_cast<const Node*>(list.next));
  }
  const_iterator end() const
  {
    return const_iterator(static_cast<const Node*>(&list));
  }
  void push_back(const Value& value)
  {
    insert(end(), value);
  }
  void pop_back(const Value& value)
  {
    erase(--end(), value);
  }
  void push_front(const Value& value)
  {
    insert(begin(), value);
  }
  void pop_front(const Value& value)
  {
    erase(begin(), value);
  }
  iterator insert(iterator pos, const Value& value)
  {
    Node* node = newNode(value);
    node_link(node, pos.node());
    return iterator(node);
  }
  iterator erase(iterator pos)
  {
    Node* node = pos.node();
    Node* next = node->getNext();
    node_unlink(node);
    deleteNode(node);
    return iterator(next);
  }
};

template<typename Functor>
class SignalBase
{
  typedef List<Functor> SignalList;
  SignalList events;

public:

  typedef Functor handler_type;
  typedef Handle< Opaque<Functor> > handler_id_type;
  typedef typename SignalList::iterator iterator;
  typedef typename SignalList::const_iterator const_iterator;
  iterator begin()
  {
    return events.begin();
  }
  iterator end()
  {
    return events.end();
  }
  const_iterator begin() const
  {
    return events.begin();
  }
  const_iterator end() const
  {
    return events.end();
  }
  handler_id_type connectFirst(const Functor& event)
  {
    events.push_front(event);
    return handler_id_type(begin().opaque());
  }
  handler_id_type connectLast(const Functor& event)
  {
    events.push_back(event);
    return handler_id_type((--end()).opaque());
  }
  bool isConnected(handler_id_type id)
  {
    for(iterator i = begin(); i != end(); ++i)
    {
      if(id.get() == i.opaque())
      {
        return true;
      }
    }
    return false;
  }
  handler_id_type connectBefore(handler_id_type id, const Functor& event)
  {
    ASSERT_MESSAGE(isConnected(id), "SignalBase::connectBefore: invalid id");
    return events.insert(iterator(id.get()), event).opaque();
  }
  handler_id_type connectAfter(handler_id_type id, const Functor& event)
  {
    ASSERT_MESSAGE(isConnected(id), "SignalBase::connectAfter: invalid id");
    return events.insert(++iterator(id.get()), event).opaque();
  }
  void disconnect(handler_id_type id)
  {
    ASSERT_MESSAGE(isConnected(id), "SignalBase::disconnect: invalid id");
    events.erase(iterator(id.get()));
  }
};

///\brief
// It is safe to disconnect the signal handler currently being invoked.
template<typename InputIterator, typename SignalHandlerInvoke>
inline void invokeSignalHandlers(InputIterator first, InputIterator last, SignalHandlerInvoke invoke)
{
  while(first != last && invoke(*first++) != SIGNAL_STOP_EMISSION);
}

class Signal0 : public SignalBase<SignalHandler>
{
public:
  void operator()() const
  {
    invokeSignalHandlers(begin(), end(), FunctorInvoke<handler_type>());
  }
};

template<typename FirstArgument>
class Signal1 : public SignalBase< SignalHandler1<FirstArgument> >
{
  typedef SignalBase< SignalHandler1<FirstArgument> > Base;
public:
  void operator()(FirstArgument a1) const
  {
    invokeSignalHandlers(Base::begin(), Base::end(), Functor1Invoke<typename Base::handler_type>(a1));
  }
};

template<typename FirstArgument, typename SecondArgument>
class Signal2 : public SignalBase< SignalHandler2<FirstArgument, SecondArgument> >
{
  typedef SignalBase< SignalHandler2<FirstArgument, SecondArgument> > Base;
public:
  void operator()(FirstArgument a1, SecondArgument a2) const
  {
    invokeSignalHandlers(Base::begin(), Base::end(), Functor2Invoke<typename Base::handler_type>(a1, a2));
  }
};

template<typename FirstArgument, typename SecondArgument, typename ThirdArgument>
class Signal3 : public SignalBase< SignalHandler3<FirstArgument, SecondArgument, ThirdArgument> >
{
  typedef SignalBase< SignalHandler3<FirstArgument, SecondArgument, ThirdArgument> > Base;
public:
  void operator()(FirstArgument a1, SecondArgument a2, ThirdArgument a3) const
  {
    invokeSignalHandlers(Base::begin(), Base::end(), Functor3Invoke<typename Base::handler_type>(a1, a2, a3));
  }
};

#endif
