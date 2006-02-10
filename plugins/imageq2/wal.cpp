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

#include "wal.h"

#include "ifilesystem.h"

#include "os/path.h"
#include "stream/stringstream.h"
#include "bytestreamutils.h"
#include "imagelib.h"

#define QUAKE2_WAL   0
#define HERETIC2_M8  1

typedef unsigned char byte;

struct pcx_header_t
{
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
};

void LoadPCXPalette(const char *filename, byte palette[768])
{
  byte* buffer;
	int length = vfsLoadFile (filename, (void **)&buffer);
  if(buffer == 0)
		return;

  const pcx_header_t* pcx = reinterpret_cast<const pcx_header_t*>(buffer);

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8)
    return;

	memcpy(palette, buffer + length - 768, 768);

  vfsFreeFile(buffer);
}

const int WAL_NAME_LENGTH = 32;
const int WAL_MIPMAP_COUNT = 4;
struct wal_header_t
{
	char      name[WAL_NAME_LENGTH];
	unsigned  width, height;
	unsigned  offsets[WAL_MIPMAP_COUNT];      // four mip maps stored
	char      animname[WAL_NAME_LENGTH];    // next frame in animation chain
	int       flags;
	int       contents;
	int       value;
};

const int M8_NAME_LENGTH = 32;
const int M8_MIPMAP_COUNT = 16;
struct m8_header_t
{
  int         version;
  char        name[M8_NAME_LENGTH];
  unsigned    width[M8_MIPMAP_COUNT], height[M8_MIPMAP_COUNT];  // width and height of each mipmap
  unsigned    offsets[M8_MIPMAP_COUNT];            // 16 mip maps stored
  char        animname[M8_NAME_LENGTH];           // next frame in animation chain
  byte        palette[768];           // palette stored in m8
  int         flags;
  int         contents;
  int         value;
};

Image* LoadMipTex(byte* buffer, byte TypeofTex)
{
  int w, h, offset, flags, contents, value;
  byte palette[768];
  byte* source;

  PointerInputStream inputStream(buffer);

  if ( TypeofTex == HERETIC2_M8 )
  {
    inputStream.seek(4 + M8_NAME_LENGTH); // version, name
    w = istream_read_int32_le(inputStream);
    inputStream.seek(4 * (M8_MIPMAP_COUNT - 1)); // remaining widths
    h = istream_read_int32_le(inputStream);
    inputStream.seek(4 * (M8_MIPMAP_COUNT - 1)); // remaining heights
    offset = istream_read_int32_le(inputStream);
    inputStream.seek(4 * (M8_MIPMAP_COUNT - 1)); // remaining offsets
    inputStream.seek(M8_NAME_LENGTH); // animname
    inputStream.read(palette, 768);
    flags = istream_read_int32_le(inputStream);
    contents = istream_read_int32_le(inputStream);
    value = istream_read_int32_le(inputStream);
    source = buffer + offset;
  }
  else
  {
    LoadPCXPalette("pics/colormap.pcx", palette);

    inputStream.seek(WAL_NAME_LENGTH); // name
    w = istream_read_int32_le(inputStream);
    h = istream_read_int32_le(inputStream);
    offset = istream_read_int32_le(inputStream);
    inputStream.seek(4 * (WAL_MIPMAP_COUNT - 1)); // remaining offsets
    inputStream.seek(WAL_NAME_LENGTH); // animname
    flags = istream_read_int32_le(inputStream);
    contents = istream_read_int32_le(inputStream);
    value = istream_read_int32_le(inputStream);
    source = buffer + offset;
  }

  RGBAImageFlags* image = new RGBAImageFlags(w, h, flags, contents, value);

  byte* dest = image->getRGBAPixels();
  byte* end = source + (w * h);
  for(; source != end; ++source, dest += 4)
  {
    *(dest + 0) = palette[*source * 3 + 0];
    *(dest + 1) = palette[*source * 3 + 1];
    *(dest + 2) = palette[*source * 3 + 2];
    *(dest + 3) = 255;
  }

  return image;
}

Image* LoadWal(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadMipTex( buffer.buffer, QUAKE2_WAL );
}

Image* LoadM8(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadMipTex( buffer.buffer, HERETIC2_M8 );
}
