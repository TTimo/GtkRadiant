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

#if !defined(INCLUDED_STREAM_FILESTREAM_H)
#define INCLUDED_STREAM_FILESTREAM_H

#include "idatastream.h"
#include <algorithm>
#include <cstdio>

namespace FileStreamDetail
{
  inline int whence_for_seekdir(SeekableStream::seekdir direction)
  {
    switch(direction)
    {
    case SeekableStream::cur:
      return SEEK_CUR;
    case SeekableStream::end:
      return SEEK_END;
    default:
      break;
    }
    return SEEK_SET;
  }
}


/// \brief A wrapper around a file input stream opened for reading in binary mode. Similar to std::ifstream.
///
/// - Maintains a valid file handle associated with a name passed to the constructor.
/// - Implements SeekableInputStream.
class FileInputStream : public SeekableInputStream
{
  std::FILE* m_file;
public:
  FileInputStream(const char* name)
  {
    m_file = name[0] == '\0' ? 0 : fopen(name, "rb");
  }
  ~FileInputStream()
  {
    if(!failed())
      fclose(m_file);
  }

  bool failed() const
  {
    return m_file == 0;
  }

  size_type read(byte_type* buffer, size_type length)
  {
    return fread(buffer, 1, length, m_file);
  }

  size_type seek(size_type position)
  {
    return fseek(m_file, static_cast<long>(position), SEEK_SET);
  }
  size_type seek(offset_type offset, seekdir direction)
  {
    return fseek(m_file, offset, FileStreamDetail::whence_for_seekdir(direction));
  }
  size_type tell() const
  {
    return ftell(m_file);
  }

  std::FILE* file()
  {
    return m_file;
  }
};

/// \brief A wrapper around a FileInputStream limiting access.
///
/// - Maintains an input stream.
/// - Provides input starting at an offset in the file for a limited range.
class SubFileInputStream : public InputStream
{
  FileInputStream& m_istream;
  size_type m_remaining;
public:
  typedef FileInputStream::position_type position_type;

  SubFileInputStream(FileInputStream& istream, position_type offset, size_type size)
    : m_istream(istream), m_remaining(size)
  {
    m_istream.seek(offset);
  }

	size_type read(byte_type* buffer, size_type length)
  {
    size_type result = m_istream.read(buffer, std::min(length, m_remaining));
    m_remaining -= result;
    return result;
  }
};


/// \brief A wrapper around a stdc file stream opened for writing in binary mode. Similar to std::ofstream..
///
/// - Maintains a valid file handle associated with a name passed to the constructor.
/// - Implements SeekableInputStream.
class FileOutputStream : public SeekableOutputStream
{
  std::FILE* m_file;
public:
  FileOutputStream(const char* name)
  {
    m_file = name[0] == '\0' ? 0 : fopen(name, "wb");
  }
  ~FileOutputStream()
  {
    if(!failed())
      fclose(m_file);
  }

  bool failed() const
  {
    return m_file == 0;
  }

  size_type write(const byte_type* buffer, size_type length)
  {
    return fwrite(buffer, 1, length, m_file);
  }

  size_type seek(size_type position)
  {
    return fseek(m_file, static_cast<long>(position), SEEK_SET);
  }
  size_type seek(offset_type offset, seekdir direction)
  {
    return fseek(m_file, offset, FileStreamDetail::whence_for_seekdir(direction));
  }
  size_type tell() const
  {
    return ftell(m_file);
  }
};

inline bool file_copy(const char* source, const char* target)
{
  const std::size_t buffer_size = 1024;
  unsigned char buffer[buffer_size];

  FileInputStream sourceFile(source);
  if(sourceFile.failed())
  {
    return false;
  }
  FileOutputStream targetFile(target);
  if(targetFile.failed())
  {
    return false;
  }

  for(;;)
  {
    std::size_t size = sourceFile.read(buffer, buffer_size);
    if(size == 0)
    {
      break;
    }
    if(targetFile.write(buffer, size) != size)
    {
      return false;
    }
  }
  return true;
}


#endif
