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

#include "wal32.h"

#include "ifilesystem.h"

#include "bytestreamutils.h"
#include "imagelib.h"

typedef unsigned char byte;

const int M32_NAME_LENGTH = 128;
const int M32_MIPMAP_COUNT = 16;

typedef struct m32_header_t
{
  int       version;
  char      name[M32_NAME_LENGTH];
  char      altname[M32_NAME_LENGTH];                   // texture substitution
  char      animname[M32_NAME_LENGTH];                  // next frame in animation chain
  char      damagename[M32_NAME_LENGTH];                // image that should be shown when damaged
  unsigned  width[M32_MIPMAP_COUNT], height[M32_MIPMAP_COUNT];
  unsigned  offsets[M32_MIPMAP_COUNT];
  int       flags;
  int       contents;
  int       value;
  float     scale_x, scale_y;
  int       mip_scale;

  // detail texturing info
  char      dt_name[M32_NAME_LENGTH];           // detailed texture name
  float     dt_scale_x, dt_scale_y;
  float     dt_u, dt_v;
  float     dt_alpha;
  int       dt_src_blend_mode, dt_dst_blend_mode;

  int       unused[20];                     // future expansion to maintain compatibility with h2
} m32_header_t;


Image* LoadM32Buff(byte* buffer)
{
  PointerInputStream inputStream(buffer);

  inputStream.seek(4 // version
    + M32_NAME_LENGTH // name
    + M32_NAME_LENGTH // altname
    + M32_NAME_LENGTH // animname
    + M32_NAME_LENGTH); // damagename
  int w = istream_read_uint32_le(inputStream);
  inputStream.seek(4 * (M32_MIPMAP_COUNT - 1)); // remaining widths
  int h = istream_read_uint32_le(inputStream);
  inputStream.seek(4 * (M32_MIPMAP_COUNT - 1)); // remaining heights
  int offset = istream_read_uint32_le(inputStream);
  inputStream.seek(4 * (M32_MIPMAP_COUNT - 1)); // remaining offsets
  int flags = istream_read_uint32_le(inputStream);
  int contents = istream_read_uint32_le(inputStream);
  int value = istream_read_uint32_le(inputStream);

  RGBAImageFlags* image = new RGBAImageFlags(w, h, flags, contents, value);

  const byte* source = buffer + offset;
  std::copy(source, source + (w * h * 4), image->getRGBAPixels());

  return image;
}

Image* LoadM32(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadM32Buff(buffer.buffer);
}

