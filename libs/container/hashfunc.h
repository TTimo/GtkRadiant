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

#if !defined(INCLUDED_CONTAINER_HASHFUNC_H)
#define INCLUDED_CONTAINER_HASHFUNC_H

#include <cctype>
#include "string/string.h"
#include "container/array.h"
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;

inline ub1 ub1_as_ub1_nocase(ub1 byte)
{
  return std::tolower(byte);
}

inline ub4 ub1x4_as_ub4_nocase(const ub1 bytes[4])
{
  ub4 result;
  reinterpret_cast<ub1*>(&result)[0] = ub1_as_ub1_nocase(bytes[0]);
  reinterpret_cast<ub1*>(&result)[1] = ub1_as_ub1_nocase(bytes[1]);
  reinterpret_cast<ub1*>(&result)[2] = ub1_as_ub1_nocase(bytes[2]);
  reinterpret_cast<ub1*>(&result)[3] = ub1_as_ub1_nocase(bytes[3]);
  return result;
}

class ub1_default_traits
{
public:
  static ub1 as_ub1(ub1 byte)
  {
    return byte;
  }
};

class ub1_nocase_traits
{
public:
  static ub1 as_ub1(ub1 byte)
  {
    return ub1_as_ub1_nocase(byte);
  }
};

class ub1x4_default_traits
{
public:
  static ub4 as_ub4(const ub1 bytes[4])
  {
    return *reinterpret_cast<const ub4*>(bytes);
  }
};

class ub1x4_nocase_traits
{
public:
  static ub4 as_ub4(const ub1 bytes[4])
  {
    return ub1x4_as_ub4_nocase(bytes);
  }
};

class ub4_default_traits
{
public:
  static ub4 as_ub4(ub4 i)
  {
    return i;
  }
};

class ub4_nocase_traits
{
public:
  static ub4 as_ub4(ub4 i)
  {
    return ub1x4_as_ub4_nocase(reinterpret_cast<const ub1*>(&i));
  }
};

// lookup2.c
// By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
// code any way you wish, private, educational, or commercial.  It's free.

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bit set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/* same, but slower, works on systems that might have 8 byte ub4's */
#define mix2(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<< 8); \
  c -= a; c -= b; c ^= ((b&0xffffffff)>>13); \
  a -= b; a -= c; a ^= ((c&0xffffffff)>>12); \
  b -= c; b -= a; b = (b ^ (a<<16)) & 0xffffffff; \
  c -= a; c -= b; c = (c ^ (b>> 5)) & 0xffffffff; \
  a -= b; a -= c; a = (a ^ (c>> 3)) & 0xffffffff; \
  b -= c; b -= a; b = (b ^ (a<<10)) & 0xffffffff; \
  c -= a; c -= b; c = (c ^ (b>>15)) & 0xffffffff; \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k     : the key (the unaligned variable-length array of bytes)
  len   : the length of the key, counting by bytes
  level : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 36+6len instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

See http://burlteburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

template<typename UB1Traits, typename UB4x1Traits>
inline ub4 hash(
const ub1 *k,        /* the key */
ub4  length,   /* the length of the key */
ub4  initval,    /* the previous hash, or an arbitrary value */
const UB1Traits& ub1traits,
const UB4x1Traits& ub4x1traits
)
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;           /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)UB1Traits::as_ub1(k[1])<<8) +((ub4)UB1Traits::as_ub1(k[2])<<16) +((ub4)UB1Traits::as_ub1(k[3])<<24));
      b += (k[4] +((ub4)UB1Traits::as_ub1(k[5])<<8) +((ub4)UB1Traits::as_ub1(k[6])<<16) +((ub4)UB1Traits::as_ub1(k[7])<<24));
      c += (k[8] +((ub4)UB1Traits::as_ub1(k[9])<<8) +((ub4)UB1Traits::as_ub1(k[10])<<16)+((ub4)UB1Traits::as_ub1(k[11])<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c += ((ub4)UB1Traits::as_ub1(k[10]) << 24);
   case 10: c += ((ub4)UB1Traits::as_ub1(k[9]) << 16);
   case 9 : c += ((ub4)UB1Traits::as_ub1(k[8]) << 8);
      /* the first byte of c is reserved for the length */
   case 8 : b += ((ub4)UB1Traits::as_ub1(k[7]) << 24);
   case 7 : b += ((ub4)UB1Traits::as_ub1(k[6]) << 16);
   case 6 : b += ((ub4)UB1Traits::as_ub1(k[5]) << 8);
   case 5 : b += UB1Traits::as_ub1(k[4]);
   case 4 : a += ((ub4)UB1Traits::as_ub1(k[3]) << 24);
   case 3 : a += ((ub4)UB1Traits::as_ub1(k[2]) << 16);
   case 2 : a += ((ub4)UB1Traits::as_ub1(k[1]) << 8);
   case 1 : a += UB1Traits::as_ub1(k[0]);
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}

/*
--------------------------------------------------------------------
 This works on all machines.  hash2() is identical to hash() on 
 little-endian machines, except that the length has to be measured
 in ub4s instead of bytes.  It is much faster than hash().  It 
 requires
 -- that the key be an array of ub4's, and
 -- that all your machines have the same endianness, and
 -- that the length be the number of ub4's in the key
--------------------------------------------------------------------
*/
template<typename UB4Traits>
inline ub4 hash2(
const ub4 *k,        /* the key */
ub4  length,   /* the length of the key, in ub4s */
ub4  initval, /* the previous hash, or an arbitrary value */
const UB4Traits& ub4traits
)  
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;           /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 3)
   {
     a += UB4Traits::as_ub4(k[0]);
     b += UB4Traits::as_ub4(k[1]);
     c += UB4Traits::as_ub4(k[2]);
     mix(a,b,c);
     k += 3; len -= 3;
   }

   /*-------------------------------------- handle the last 2 ub4's */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
     /* c is reserved for the length */
   case 2 : b += UB4Traits::as_ub4(k[1]);
   case 1 : a += UB4Traits::as_ub4(k[0]);
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}

typedef ub4 hash_t;

inline hash_t hash_ub1(const ub1* key, std::size_t len, hash_t previous = 0)
{
  return hash(key, ub4(len), previous, ub1_default_traits(), ub1x4_default_traits());
}

inline hash_t hash_ub1_nocase(const ub1* key, std::size_t len, hash_t previous = 0)
{
  return hash(key, ub4(len), previous, ub1_nocase_traits(), ub1x4_nocase_traits());
}

template<typename UB4Traits>
inline hash_t hash_ub4(const ub4* key, std::size_t len, const UB4Traits& traits, hash_t previous = 0)
{
  return hash2(key,ub4(len), previous, traits);
}

inline ub4 hash_combine(ub4 left, ub4 right)
{
  return hash_ub1(reinterpret_cast<const ub1*>(&left), 4, right);
}

template<typename POD>
inline hash_t pod_hash(const POD& pod)
{
  return hash_ub1(reinterpret_cast<const ub1*>(&pod), sizeof(POD));
}

inline hash_t string_hash(const char* string, hash_t previous = 0)
{
  return hash_ub1(reinterpret_cast<const ub1*>(string), string_length(string), previous);
}

inline hash_t string_hash_nocase(const char* string, hash_t previous = 0)
{
  return hash_ub1_nocase(reinterpret_cast<const ub1*>(string), string_length(string), previous);
}

struct HashString
{
  typedef hash_t hash_type;
  hash_type operator()(const CopiedString& string) const
  {
    return string_hash(string.c_str());
  }
};

struct HashStringNoCase
{
  typedef hash_t hash_type;
  hash_type operator()(const CopiedString& string) const
  {
    return string_hash_nocase(string.c_str());
  }
};

/// \brief Length of a string in ub4.
/// "wibble" (6) gives 2,
/// "and" (3) gives 1,
/// "bleh" (4) gives 2
inline std::size_t string_length_ub4(const char* string)
{
  return ((string_length(string)>>2)+1)<<2;
}

/// \brief Hashable key type that stores a string as an array of ub4 - making hashing faster.
/// Also caches the 32-bit result of the hash to speed up comparison of keys.
template<typename UB4Traits = ub4_default_traits>
class HashKey
{
  Array<ub4> m_key;
  hash_t m_hash;

  void copy(const HashKey& other)
  {
    std::copy(other.m_key.begin(), other.m_key.end(), m_key.begin());
    m_hash = other.m_hash;
  }
  void copy(const char* string)
  {
    strncpy(reinterpret_cast<char*>(m_key.data()), string, m_key.size());
    for(Array<ub4>::iterator i = m_key.begin(); i != m_key.end(); ++i)
    {
      *i = UB4Traits::as_ub4(*i);
    }
    m_hash = hash_ub4(m_key.data(), m_key.size(), ub4_default_traits());
  }
  bool equal(const HashKey& other) const
  {
    return m_hash == other.m_hash && m_key.size() == other.m_key.size()
      && std::equal(m_key.begin(), m_key.end(), other.m_key.begin());
  }

public:
  HashKey(const HashKey& other) : m_key(other.m_key.size())
  {
    copy(other);
  }
  HashKey(const char* string) : m_key(string_length_ub4(string))
  {
    copy(string);
  }
  HashKey& operator=(const char* string)
  {
    m_key.resize(string_length_ub4(string));
    copy(string);
    return *this;
  }
  bool operator==(const HashKey& other) const
  {
    return equal(other);
  }
  bool operator!=(const HashKey& other) const
  {
    return !equal(other);
  }
  hash_t hash() const
  {
    return m_hash;
  }
#if 0
  const char* c_str() const
  {
    return reinterpret_cast<const char*>(m_key.data());
  }
#endif
};

/// \brief Hash function to use with HashKey.
struct HashKeyHasher
{
  typedef hash_t hash_type;
  hash_type operator()(const HashKey<ub4_default_traits>& key) const
  {
    return key.hash();
  }
};



#endif
