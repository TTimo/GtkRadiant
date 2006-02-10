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

#if !defined(INCLUDED_CONTAINER_HASHTABLE_H)
#define INCLUDED_CONTAINER_HASHTABLE_H

#include <cstddef>
#include <algorithm>
#include <functional>
#include "debugging/debugging.h"


namespace HashTableDetail
{
  inline std::size_t next_power_of_two(std::size_t size)
  {
    std::size_t result = 1;
    while(result < size)
    {
      result <<= 1;
    }
    return result;
  }

  struct BucketNodeBase
  {
    BucketNodeBase* next;
    BucketNodeBase* prev;
  };

  inline void list_initialise(BucketNodeBase& self)
  {
    self.next = self.prev = &self;
  }

  inline void list_swap(BucketNodeBase& self, BucketNodeBase& other)
  {
    BucketNodeBase tmp(self);
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

  inline void node_link(BucketNodeBase* node, BucketNodeBase* next)
  {
    node->next = next;
    node->prev = next->prev;
    next->prev = node;
    node->prev->next = node;
  }
  inline void node_unlink(BucketNodeBase* node)
  {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  template<typename Key, typename Value>
  struct KeyValue
  {
    const Key key;
    Value value;

    KeyValue(const Key& key_, const Value& value_)
      : key(key_), value(value_)
    {
    }
  };

  template<typename Key, typename Value, typename Hash>
  struct BucketNode : public BucketNodeBase
  {
    Hash m_hash;
    KeyValue<Key, Value> m_value;

    BucketNode(Hash hash, const Key& key, const Value& value) 
     : m_hash(hash), m_value(key, value)
    {
    }
    BucketNode* getNext()
    {
      return static_cast<BucketNode*>(next);
    }
    BucketNode* getPrev()
    {
      return static_cast<BucketNode*>(prev);
    }
  };

  template<typename Key, typename Value, typename Hash>
  class BucketIterator
  {
    typedef BucketNode<Key, Value, Hash> Node;
    Node* m_node;

    void increment()
    {
      m_node = m_node->getNext();
    }

  public:
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t difference_type;
    typedef difference_type distance_type;
    typedef KeyValue<Key, Value> value_type;
    typedef value_type* pointer;
    typedef value_type& reference;

    BucketIterator(Node* node) : m_node(node)
    {
    }

    Node* node()
    {
      return m_node;
    }

    bool operator==(const BucketIterator& other) const
    {
      return m_node == other.m_node;
    }
    bool operator!=(const BucketIterator& other) const
    {
      return !operator==(other);
    }
    BucketIterator& operator++()
    {
      increment();
      return *this;
    }
    BucketIterator operator++(int)
    {
      BucketIterator tmp = *this;
      increment();
      return tmp;
    }
    value_type& operator*()
    {
      return m_node->m_value;
    }
    value_type* operator->()
    {
      return &(operator*());
    }
  };
}


/// A hash-table container which maps keys to values.
///
/// - Inserting or removing elements does not invalidate iterators.
/// - Inserting or retrieving an element for a given key takes O(1) time on average.
/// - Elements are stored in no particular order.
///
/// \param Key Uniquely identifies a value. Must provide a copy-constructor.
/// \param Value The value to be stored . Must provide a default-constructor and a copy-constructor.
/// \param Hasher Must provide 'std::size_t operator()(const Key&) const' which always returns the same result if the same argument is given.
/// \param KeyEqual Must provide 'bool operator==(const Key&, const Key&) const' which returns true only if both arguments are equal.
///
/// \dontinclude container/hashtable.cpp
/// \skipline HashTable example
/// \until end example
template<typename Key, typename Value, typename Hasher, typename KeyEqual = std::equal_to<Key> >
class HashTable : private KeyEqual, private Hasher
{
  typedef typename Hasher::hash_type hash_type;
  typedef HashTableDetail::KeyValue<Key, Value> KeyValue;
  typedef HashTableDetail::BucketNode<Key, Value, hash_type> BucketNode;

  inline BucketNode* node_create(hash_type hash, const Key& key, const Value& value)
  {
    return new BucketNode(hash, key, value);
  }
  inline void node_destroy(BucketNode* node)
  {
    delete node;
  }

  typedef BucketNode* Bucket;

  static Bucket* buckets_new(std::size_t count)
  {
    Bucket* buckets = new Bucket[count];
    std::uninitialized_fill(buckets, buckets + count, Bucket(0));
    return buckets;
  }
  static void buckets_delete(Bucket* buckets)
  {
    delete[] buckets;
  }

  std::size_t m_bucketCount;
  Bucket* m_buckets;
  std::size_t m_size;
  HashTableDetail::BucketNodeBase m_list;

  BucketNode* getFirst()
  {
    return static_cast<BucketNode*>(m_list.next);
  }
  BucketNode* getLast()
  {
    return static_cast<BucketNode*>(&m_list);
  }

public:

  typedef KeyValue value_type;
  typedef HashTableDetail::BucketIterator<Key, Value, hash_type> iterator;

private:

  void initialise()
  {
    list_initialise(m_list);
  }
  hash_type hashKey(const Key& key)
  {
    return Hasher::operator()(key);
  }

  std::size_t getBucketId(hash_type hash) const
  {
    return hash & (m_bucketCount - 1);
  }
  Bucket& getBucket(hash_type hash)
  {
    return m_buckets[getBucketId(hash)];
  }
  BucketNode* bucket_find(Bucket bucket, hash_type hash, const Key& key)
  {
    std::size_t bucketId = getBucketId(hash);
    for(iterator i(bucket); i != end(); ++i)
    {
      hash_type nodeHash = i.node()->m_hash;

      if(getBucketId(nodeHash) != bucketId)
      {
        return 0;
      }

      if(nodeHash == hash && KeyEqual::operator()((*i).key, key))
      {
        return i.node();
      }
    }
    return 0;
  }
  BucketNode* bucket_insert(Bucket& bucket, BucketNode* node)
  {
    // link node into list
    node_link(node, bucket_next(bucket));
    bucket = node;
    return node;
  }
  BucketNode* bucket_next(Bucket& bucket)
  {
    Bucket* end = m_buckets + m_bucketCount;
    for(Bucket* i = &bucket; i != end; ++i)
    {
      if(*i != 0)
      {
        return *i;
      }
    }
    return getLast();
  }

  void buckets_resize(std::size_t count)
  {
    BucketNode* first = getFirst();
    BucketNode* last = getLast();

    buckets_delete(m_buckets);

    m_bucketCount = count;

    m_buckets = buckets_new(m_bucketCount);
    initialise();

    for(BucketNode* i = first; i != last;)
    {
      BucketNode* node = i;
      i = i->getNext();
      bucket_insert(getBucket((*node).m_hash), node);
    }
  }
  void size_increment()
  {
    if(m_size == m_bucketCount)
    {
      buckets_resize(m_bucketCount == 0 ? 8 : m_bucketCount << 1);
    }
    ++m_size;
  }
  void size_decrement()
  {
    --m_size;
  }

  HashTable(const HashTable& other);
  HashTable& operator=(const HashTable& other);
public:
  HashTable() : m_bucketCount(0), m_buckets(0), m_size(0)
  {
    initialise();
  }
  HashTable(std::size_t bucketCount) : m_bucketCount(HashTableDetail::next_power_of_two(bucketCount)), m_buckets(buckets_new(m_bucketCount)), m_size(0)
  {
    initialise();
  }
  ~HashTable()
  {
    for(BucketNode* i = getFirst(); i != getLast();)
    {
      BucketNode* node = i;
      i = i->getNext();
      node_destroy(node);
    }
    buckets_delete(m_buckets);
  }

  iterator begin()
  {
    return iterator(getFirst());
  }
  iterator end()
  {
    return iterator(getLast());
  }

  bool empty() const
  {
    return m_size == 0;
  }
  std::size_t size() const
  {
    return m_size;
  }

  /// \brief Returns an iterator pointing to the value associated with \p key if it is contained by the hash-table, else \c end().
  iterator find(const Key& key)
  {
    hash_type hash = hashKey(key);
    if(m_bucketCount != 0)
    {
      Bucket bucket = getBucket(hash);
      if(bucket != 0)
      {
        BucketNode* node = bucket_find(bucket, hash, key);
        if(node != 0)
        {
          return iterator(node);
        }
      }
    }
    
    return end();
  }
  /// \brief Adds \p value to the hash-table associated with \p key if it does not exist.
  iterator insert(const Key& key, const Value& value)
  {
    hash_type hash = hashKey(key);
    if(m_bucketCount != 0)
    {
      Bucket& bucket = getBucket(hash);
      if(bucket != 0)
      {
        BucketNode* node = bucket_find(bucket, hash, key);
        if(node != 0)
        {
          return iterator(node);
        }
      }
    }

    size_increment();
    return iterator(bucket_insert(getBucket(hash), node_create(hash, key, value)));
  }

  /// \brief Removes the value pointed to by \p i from the hash-table.
  ///
  /// \p i must be a deferenceable iterator into the hash-table.
  void erase(iterator i)
  {
    Bucket& bucket = getBucket(i.node()->m_hash);
    BucketNode* node = i.node();

    // if this was the last node in the bucket
    if(bucket == node)
    {
      bucket = (node->getNext() == getLast() || &getBucket(node->getNext()->m_hash) != &bucket) ? 0 : node->getNext();
    }

    node_unlink(node);
    ASSERT_MESSAGE(node != 0, "tried to erase a non-existent key/value");
    node_destroy(node);

    size_decrement();
  }

  /// \brief Returns the value identified by \p key if it is contained by the hash-table, else inserts and returns a new default-constructed value associated with \p key.
  Value& operator[](const Key& key)
  {
    hash_type hash = hashKey(key);
    if(m_bucketCount != 0)
    {
      Bucket& bucket = getBucket(hash);
      if(bucket != 0)
      {
        BucketNode* node = bucket_find(bucket, hash, key);
        if(node != 0)
        {
          return node->m_value.value;
        }
      }
    }
    size_increment();
    return bucket_insert(getBucket(hash), node_create(hash, key, Value()))->m_value.value;
  }
  /// \brief Removes the value associated with \p key from the hash-table.
  void erase(const Key& key)
  {
    erase(find(key));
  }
  /// \brief Swaps the contents of the hash-table with \p other.
  void swap(HashTable& other)
  {
    std::swap(m_buckets, other.m_buckets);
    std::swap(m_bucketCount, other.m_bucketCount);
    std::swap(m_size, other.m_size);
    HashTableDetail::list_swap(m_list, other.m_list);
  }
  /// \brief Removes all values from the hash-table.
  void clear()
  {
    HashTable tmp;
    tmp.swap(*this);
  }
};

#endif
