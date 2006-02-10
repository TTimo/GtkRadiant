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

#if !defined(INCLUDED_IMAGELIB_H)
#define INCLUDED_IMAGELIB_H

#include "iimage.h"
#include "iarchive.h"
#include "idatastream.h"
#include <stdlib.h>

struct RGBAPixel
{
  unsigned char red, green, blue, alpha;
};

class RGBAImage : public Image
{
  RGBAImage(const RGBAImage& other);
  RGBAImage& operator=(const RGBAImage& other);
public:
  RGBAPixel* pixels;
  unsigned int width, height;

  RGBAImage(unsigned int _width, unsigned int _height)
    : pixels(new RGBAPixel[_width * _height]), width(_width), height(_height)
  {
  }
  ~RGBAImage()
  {
    delete pixels;
  }

  void release()
  {
    delete this;
  }
  byte* getRGBAPixels() const
  {
    return reinterpret_cast<byte*>(pixels);
  }
  unsigned int getWidth() const
  {
    return width;
  }
  unsigned int getHeight() const
  {
    return height;
  }
};

class RGBAImageFlags : public RGBAImage
{
public:
  int m_surfaceFlags;
  int m_contentFlags;
  int m_value;
  RGBAImageFlags(unsigned short _width, unsigned short _height, int surfaceFlags, int contentFlags, int value) :
    RGBAImage(_width, _height), m_surfaceFlags(surfaceFlags), m_contentFlags(contentFlags), m_value(value)
  {
  }

  int getSurfaceFlags() const
  {
    return m_surfaceFlags;
  }
  int getContentFlags() const
  {
    return m_contentFlags;
  }
  int getValue() const
  {
    return m_value;
  }
};


inline InputStream::byte_type* ArchiveFile_loadBuffer(ArchiveFile& file, std::size_t& length)
{
  InputStream::byte_type* buffer = (InputStream::byte_type*)malloc(file.size() + 1);
  length = file.getInputStream().read(buffer, file.size());
  buffer[file.size()] = 0;
  return buffer;
}

inline void ArchiveFile_freeBuffer(InputStream::byte_type* buffer)
{
  free(buffer);
}

class ScopedArchiveBuffer
{
public:
  std::size_t length;
  InputStream::byte_type* buffer;

  ScopedArchiveBuffer(ArchiveFile& file)
  {
    buffer = ArchiveFile_loadBuffer(file, length);
  }
  ~ScopedArchiveBuffer()
  {
    ArchiveFile_freeBuffer(buffer);
  }
};

class PointerInputStream : public InputStream
{
  const byte* m_read;
public:
  PointerInputStream(const byte* pointer)
    : m_read(pointer)
  {
  }
  std::size_t read(byte* buffer, std::size_t length)
  {
    const byte* end = m_read + length;
    while(m_read != end)
    {
      *buffer++ = *m_read++;
    }
    return length;
  }
  void seek(std::size_t offset)
  {
    m_read += offset;
  }
  const byte* get()
  {
    return m_read;
  }
};

#endif
