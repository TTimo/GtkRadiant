
#if !defined(INCLUDED_BYTESTREAMUTILS_H)
#define INCLUDED_BYTESTREAMUTILS_H

#include <algorithm>

#if defined(_MSC_VER)

typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

#else

#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1

#define	__USE_ISOC9X	1
#define	__USE_ISOC99	1

#include <stdint.h>

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
