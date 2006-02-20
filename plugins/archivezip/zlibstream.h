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

#if !defined(INCLUDED_ZLIBSTREAM_H)
#define INCLUDED_ZLIBSTREAM_H

#include "zlib.h"
#include "idatastream.h"

/// \brief A wrapper around an InputStream of data compressed with the zlib deflate algorithm.
///
/// - Uses z_stream to decompress the data stream on the fly.
/// - Uses a buffer to reduce the number of times the wrapped stream must be read.
class DeflatedInputStream : public InputStream
{
  InputStream& m_istream;
  z_stream m_zipstream;
  enum unnamed0 { m_bufsize = 1024 };
  unsigned char m_buffer[m_bufsize];

public:
  DeflatedInputStream(InputStream& istream)
    : m_istream(istream)
  {
    m_zipstream.zalloc = 0;
    m_zipstream.zfree = 0;
    m_zipstream.opaque = 0;
    m_zipstream.avail_in = 0;
    inflateInit2(&m_zipstream, -MAX_WBITS);
  }
  ~DeflatedInputStream()
  {
    inflateEnd(&m_zipstream);
  }
  size_type read(byte_type* buffer, size_type length)
  {
    m_zipstream.next_out = buffer;
    m_zipstream.avail_out = static_cast<uInt>(length);
    while(m_zipstream.avail_out != 0)
    {
      if(m_zipstream.avail_in == 0)
      {
        m_zipstream.next_in = m_buffer;
        m_zipstream.avail_in = static_cast<uInt>(m_istream.read(m_buffer, m_bufsize));
      }
      if(inflate(&m_zipstream, Z_SYNC_FLUSH) != Z_OK)
      {
        break;
      }
    }
    return length - m_zipstream.avail_out;
  }
};

#endif


