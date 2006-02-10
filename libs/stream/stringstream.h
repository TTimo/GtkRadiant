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

#if !defined(INCLUDED_STREAM_STRINGSTREAM_H)
#define INCLUDED_STREAM_STRINGSTREAM_H

#include "itextstream.h"
#include "string/string.h"
#include <vector>


/// \brief A wrapper around a STL vector of char.
/// Maintains a null-terminated array of char.
/// Provides a limited STL-style interface to push and pop characters at the end of the string.
class StringBuffer
{
  std::vector<char> m_string;
public:
  StringBuffer()
  {
    m_string.push_back('\0');
  }
  explicit StringBuffer(std::size_t capacity)
  {
    m_string.reserve(capacity);
    m_string.push_back('\0');
  }
  explicit StringBuffer(const char* string) : m_string(string, string + string_length(string) + 1)
  {
  }

  typedef std::vector<char>::iterator iterator;
  typedef std::vector<char>::const_iterator const_iterator;

  iterator begin()
  {
    return m_string.begin();
  }
  const_iterator begin() const
  {
    return m_string.begin();
  }
  iterator end()
  {
    return m_string.end() - 1;
  }
  const_iterator end() const
  {
    return m_string.end() - 1;
  }

  void push_back(char c)
  {
    m_string.insert(end(), c);
  }
  void pop_back()
  {
    m_string.erase(end() - 1);
  }
  void push_range(const char* first, const char* last)
  {
    m_string.insert(end(), first, last);
  }
  void push_string(const char* string)
  {
    push_range(string, string + string_length(string));
  }
  char* c_str()
  {
    return &(*m_string.begin());
  }
  const char* c_str() const
  {
    return &(*m_string.begin());
  }

  char& back()
  {
    return *(end() - 1);
  }
  const char& back() const
  {
    return *(end() - 1);
  }
  bool empty() const
  {
    return m_string.size() == 1;
  }
  void clear()
  {
    m_string.clear();
    m_string.push_back('\0');
  }
};

/// \brief A TextOutputStream which writes to a StringBuffer.
/// Similar to std::stringstream.
class StringOutputStream : public TextOutputStream
{
  StringBuffer m_string;
public:
  typedef StringBuffer::iterator iterator;
  typedef StringBuffer::const_iterator const_iterator;

  StringOutputStream()
  {
  }
  StringOutputStream(std::size_t capacity) : m_string(capacity)
  {
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    m_string.push_range(buffer, buffer + length);
    return length;
  }

  iterator begin()
  {
    return m_string.begin();
  }
  const_iterator begin() const
  {
    return m_string.begin();
  }
  iterator end()
  {
    return m_string.end();
  }
  const_iterator end() const
  {
    return m_string.end();
  }

  bool empty() const
  {
    return m_string.empty();
  }
  char* c_str()
  {
    return m_string.c_str();
  }
  const char* c_str() const
  {
    return m_string.c_str();
  }
  void clear()
  {
    m_string.clear();
  }
};

template<typename T>
inline StringOutputStream& operator<<(StringOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}


#endif
