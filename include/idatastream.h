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

#if !defined(INCLUDED_IDATASTREAM_H)
#define INCLUDED_IDATASTREAM_H

#include <cstddef>

class StreamBase
{
public:
  typedef std::size_t size_type;
  typedef unsigned char byte_type;
};

/// \brief A read-only byte-stream.
class InputStream : public StreamBase
{
public:
  /// \brief Attempts to read the next \p length bytes from the stream to \p buffer.
  /// Returns the number of bytes actually stored in \p buffer.
  virtual size_type read(byte_type* buffer, size_type length) = 0;
};

/// \brief A write-only byte-stream.
class OutputStream : public StreamBase
{
public:
  /// \brief Attempts to write \p length bytes to the stream from \p buffer.
  /// Returns the number of bytes actually read from \p buffer.
  virtual size_type write(const byte_type* buffer, size_type length) = 0;
};

class SeekableStream
{
public:
  typedef int offset_type;
  typedef std::size_t position_type;

  enum seekdir
  {
    beg,
    cur,
    end,
  };

  /// \brief Sets the current \p position of the stream relative to the start.
  virtual position_type seek(position_type position) = 0;
  /// \brief Sets the current \p position of the stream relative to either the start, end or current position.
  virtual position_type seek(offset_type offset, seekdir direction) = 0;
  /// \brief Returns the current position of the stream.
  virtual position_type tell() const = 0;
};

/// \brief A seekable read-only byte-stream.
class SeekableInputStream : public InputStream, public SeekableStream
{
};

/// \brief A seekable write-only byte-stream.
class SeekableOutputStream : public OutputStream, public SeekableStream
{
};

#endif
