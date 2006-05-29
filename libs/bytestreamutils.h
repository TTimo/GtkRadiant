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

#if !defined(INCLUDED_BYTESTREAMUTILS_H)
#define INCLUDED_BYTESTREAMUTILS_H

#if defined(__GNUC__)

#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1

#define	__USE_ISOC9X	1
#define	__USE_ISOC99	1

#include <stdint.h>

#endif

#include <algorithm>

// if C99 is unavailable, fall back to the types most likely to be the right sizes
#if !defined(int16_t)
typedef signed short int16_t;
#endif
#if !defined(uint16_t)
typedef unsigned short uint16_t;
#endif
#if !defined(int32_t)
typedef signed int int32_t;
#endif
#if !defined(uint32_t)
typedef unsigned int uint32_t;
#endif




template<typename InputStreamType, typename Type>
inline void istream_read_little_endian(InputStreamType& istream, Type& value)
{
  istream.read(reinterpret_cast<typename InputStreamType::byte_type*>(&value), sizeof(Type));
#if defined(__BIG_ENDIAN__)
  std::reverse(reinterpret_cast<typename InputStreamType::byte_type*>(&value), reinterpret_cast<typename InputStreamType::byte_type*>(&value) + sizeof(Type));
#endif
}

template<typename InputStreamType, typename Type>
inline void istream_read_big_endian(InputStreamType& istream, Type& value)
{
  istream.read(reinterpret_cast<typename InputStreamType::byte_type*>(&value), sizeof(Type));
#if !defined(__BIG_ENDIAN__)
  std::reverse(reinterpret_cast<typename InputStreamType::byte_type*>(&value), reinterpret_cast<typename InputStreamType::byte_type*>(&value) + sizeof(Type));
#endif
}

template<typename InputStreamType>
inline void istream_read_byte(InputStreamType& istream, typename InputStreamType::byte_type& b)
{
  istream.read(&b, 1);
}


template<typename InputStreamType>
inline int16_t istream_read_int16_le(InputStreamType& istream)
{
  int16_t value;
  istream_read_little_endian(istream, value);
  return value;
}

template<typename InputStreamType>
inline uint16_t istream_read_uint16_le(InputStreamType& istream)
{
  uint16_t value;
  istream_read_little_endian(istream, value);
  return value;
}

template<typename InputStreamType>
inline int32_t istream_read_int32_le(InputStreamType& istream)
{
  int32_t value;
  istream_read_little_endian(istream, value);
  return value;
}

template<typename InputStreamType>
inline uint32_t istream_read_uint32_le(InputStreamType& istream)
{
  uint32_t value;
  istream_read_little_endian(istream, value);
  return value;
}

template<typename InputStreamType>
inline float istream_read_float32_le(InputStreamType& istream)
{
  float value;
  istream_read_little_endian(istream, value);
  return value;
}

template<typename InputStreamType>
inline typename InputStreamType::byte_type istream_read_byte(InputStreamType& istream)
{
  typename InputStreamType::byte_type b;
  istream.read(&b, sizeof(typename InputStreamType::byte_type));
  return b;
}

#endif
