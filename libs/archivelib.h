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

#if !defined (INCLUDED_ARCHIVELIB_H)
#define INCLUDED_ARCHIVELIB_H

#include "debugging/debugging.h"
#include "iarchive.h"
#include "stream/filestream.h"
#include "stream/textfilestream.h"
#include "memory/allocator.h"
#include "string/string.h"

/// \brief A single-byte-reader wrapper around an InputStream.
/// Optimised for reading one byte at a time.
/// Uses a buffer to reduce the number of times the wrapped stream must be read.
template<typename InputStreamType, int SIZE = 1024>
class SingleByteInputStream
{
  typedef typename InputStreamType::byte_type byte_type;

  InputStreamType& m_inputStream;
  byte_type m_buffer[SIZE];
  byte_type* m_cur;
  byte_type* m_end;

public:

  SingleByteInputStream(InputStreamType& inputStream) : m_inputStream(inputStream), m_cur(m_buffer + SIZE), m_end(m_cur)
  {
  }
  bool readByte(byte_type& b)
  {
    if(m_cur == m_end)
    {
      if(m_end != m_buffer + SIZE)
      {
        return false;
      }

      m_end = m_buffer + m_inputStream.read(m_buffer, SIZE);
      m_cur = m_buffer;

      if(m_end == m_buffer)
      {
        return false;
      }
    }

    b = *m_cur++;

    return true;
  }
};

/// \brief A binary-to-text wrapper around an InputStream.
/// Converts CRLF or LFCR line-endings to LF line-endings.
template<typename BinaryInputStreamType>
class BinaryToTextInputStream : public TextInputStream
{
  SingleByteInputStream<BinaryInputStreamType> m_inputStream;
public:
  BinaryToTextInputStream(BinaryInputStreamType& inputStream) : m_inputStream(inputStream)
  {
  }
  std::size_t read(char* buffer, std::size_t length)
  {
    char* p = buffer;
    for(;;)
    {
      if(length != 0 && m_inputStream.readByte(*reinterpret_cast<typename BinaryInputStreamType::byte_type*>(p)))
      {
        if(*p != '\r')
        {
          ++p;
          --length;
        }
      }
      else
      {
        return p - buffer;
      }
    }
  }
};

/// \brief An ArchiveFile which is stored uncompressed as part of a larger archive file.
class StoredArchiveFile : public ArchiveFile
{
  CopiedString m_name;
  FileInputStream m_filestream;
  SubFileInputStream m_substream;
  FileInputStream::size_type m_size;
public:
  typedef FileInputStream::size_type size_type;
  typedef FileInputStream::position_type position_type;

  StoredArchiveFile(const char* name, const char* archiveName, position_type position, size_type stream_size, size_type file_size)
    : m_name(name), m_filestream(archiveName), m_substream(m_filestream, position, stream_size), m_size(file_size)
  {
  }

  static StoredArchiveFile* create(const char* name, const char* archiveName, position_type position, size_type stream_size, size_type file_size)
  {
    return New<StoredArchiveFile>().scalar(name, archiveName, position, stream_size, file_size);
  }

  void release()
  {
    Delete<StoredArchiveFile>().scalar(this);
  }
  size_type size() const
  {
    return m_size;
  }
  const char* getName() const
  {
    return m_name.c_str();
  }
  InputStream& getInputStream()
  {
    return m_substream;
  }
};

/// \brief An ArchiveTextFile which is stored uncompressed as part of a larger archive file.
class StoredArchiveTextFile : public ArchiveTextFile
{
  CopiedString m_name;
  FileInputStream m_filestream;
  SubFileInputStream m_substream;
  BinaryToTextInputStream<SubFileInputStream> m_textStream;
public:
  typedef FileInputStream::size_type size_type;
  typedef FileInputStream::position_type position_type;

  StoredArchiveTextFile(const char* name, const char* archiveName, position_type position, size_type stream_size)
    : m_name(name), m_filestream(archiveName), m_substream(m_filestream, position, stream_size), m_textStream(m_substream)
  {
  }

  static StoredArchiveTextFile* create(const char* name, const char* archiveName, position_type position, size_type stream_size)
  {
    return New<StoredArchiveTextFile>().scalar(name, archiveName, position, stream_size);
  }

  void release()
  {
    Delete<StoredArchiveTextFile>().scalar(this);
  }
  const char* getName() const
  {
    return m_name.c_str();
  }
  TextInputStream& getInputStream()
  {
    return m_textStream;
  }
};

/// \brief An ArchiveFile which is stored as a single file on disk.
class DirectoryArchiveFile : public ArchiveFile
{
  CopiedString m_name;
  FileInputStream m_istream;
  FileInputStream::size_type m_size;
public:
  typedef FileInputStream::size_type size_type;

  DirectoryArchiveFile(const char* name, const char* filename)
    : m_name(name), m_istream(filename)
  {
    if(!failed())
    {
      m_istream.seek(0, FileInputStream::end);
      m_size = m_istream.tell();
      m_istream.seek(0);
    }
    else
    {
      m_size = 0;
    }
  }
  bool failed() const
  {
    return m_istream.failed();
  }

  void release()
  {
    delete this;
  }
  size_type size() const
  {
    return m_size;
  }
  const char* getName() const
  {
    return m_name.c_str();
  }
  InputStream& getInputStream()
  {
    return m_istream;
  }
};

/// \brief An ArchiveTextFile which is stored as a single file on disk.
class DirectoryArchiveTextFile : public ArchiveTextFile
{
  CopiedString m_name;
  TextFileInputStream m_inputStream;
public:

  DirectoryArchiveTextFile(const char* name, const char* filename)
    : m_name(name), m_inputStream(filename)
  {
  }
  bool failed() const
  {
    return m_inputStream.failed();
  }

  void release()
  {
    delete this;
  }
  const char* getName() const
  {
    return m_name.c_str();
  }
  TextInputStream& getInputStream()
  {
    return m_inputStream;
  }
};


#endif
