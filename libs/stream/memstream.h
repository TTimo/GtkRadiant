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

#if !defined(INCLUDED_STREAM_MEMSTREAM_H)
#define INCLUDED_STREAM_MEMSTREAM_H

#include "itextstream.h"
#include <algorithm>
#include <vector>

class BufferOutputStream : public TextOutputStream
{
  std::vector<char> m_buffer;
public:
  std::size_t write(const char* buffer, std::size_t length)
  {
    m_buffer.insert(m_buffer.end(), buffer, buffer+length);
    return length;
  }
  const char* data() const
  {
    return &(*m_buffer.begin());
  }
  std::size_t size() const
  {
    return m_buffer.size();
  }
  void clear()
  {
    std::vector<char> empty;
    std::swap(empty, m_buffer);
  }
};

template<typename T>
inline BufferOutputStream& operator<<(BufferOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}


class BufferInputStream : public TextInputStream
{
  const char* m_read;
  const char* m_end;
public:
  BufferInputStream(const char* buffer, std::size_t length)
    : m_read(buffer), m_end(buffer + length)
  {
  }
  std::size_t read(char* buffer, std::size_t length)
  {
    std::size_t count = std::min(std::size_t(m_end - m_read), length);
    const char* end = m_read + count;
    while(m_read != end)
    {
      *buffer++ = *m_read++;
    }
    return count;
  }
};

#endif
