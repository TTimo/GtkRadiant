/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "pcx.h"

#include "ifilesystem.h"

typedef unsigned char byte;

#include <stdlib.h>

#include "imagelib.h"
#include "bytestreamutils.h"

/*
=================================================================

PCX LOADING

=================================================================
*/

typedef struct
{
  unsigned char manufacturer;
  unsigned char version;
  unsigned char encoding;
  unsigned char bits_per_pixel;
  unsigned short xmin, ymin, xmax, ymax;
  unsigned short hres, vres;
  unsigned char palette[48];
  unsigned char reserved;
  unsigned char color_planes;
  unsigned short bytes_per_line;
  unsigned short palette_type;
  unsigned char filler[58];
  unsigned char data;		// unbounded
} pcx_t;

/*
==============
LoadPCX
==============
*/

struct PCXRLEPacket
{
  byte data;
  int length;
};

inline void ByteStream_readPCXRLEPacket(PointerInputStream& inputStream, PCXRLEPacket& packet)
{
  byte d;
  inputStream.read(&d, 1);
  if((d & 0xC0) == 0xC0)
  {
    packet.length = d & 0x3F;
    inputStream.read(&packet.data, 1); 
  }
  else
  {
    packet.length = 1;
    packet.data = d;
  }
}

void LoadPCXBuff(byte* buffer, std::size_t len, byte **pic, byte **palette, int *width, int *height )
{
  *pic = 0;

  pcx_t	pcx;
  int		x, y, lsize;
  byte	*out, *pix;

  /* parse the PCX file */

  PointerInputStream inputStream(buffer);
  
  pcx.manufacturer = istream_read_byte(inputStream);
  pcx.version = istream_read_byte(inputStream);
  pcx.encoding = istream_read_byte(inputStream);
  pcx.bits_per_pixel = istream_read_byte(inputStream);
  pcx.xmin = istream_read_int16_le(inputStream);
  pcx.ymin = istream_read_int16_le(inputStream);
  pcx.xmax = istream_read_int16_le(inputStream);
  pcx.ymax = istream_read_int16_le(inputStream);
  pcx.hres = istream_read_int16_le(inputStream);
  pcx.vres = istream_read_int16_le(inputStream);
  inputStream.read(pcx.palette, 48);
  pcx.reserved = istream_read_byte(inputStream);
  pcx.color_planes = istream_read_byte(inputStream);
  pcx.bytes_per_line = istream_read_int16_le(inputStream);
  pcx.palette_type = istream_read_int16_le(inputStream);
  inputStream.read(pcx.filler, 58);

  
  if (pcx.manufacturer != 0x0a
    || pcx.version != 5
    || pcx.encoding != 1
    || pcx.bits_per_pixel != 8)
    return;

  if (width)
    *width = pcx.xmax+1;
  if (height)
    *height = pcx.ymax+1;

  if (!pic)
    return;

  out = (byte *)malloc ( (pcx.ymax+1) * (pcx.xmax+1) );

  *pic = out;
  pix = out;
	
  /* RR2DO2: pcx fix  */
  lsize = pcx.color_planes * pcx.bytes_per_line;
	
  /* go scanline by scanline */
  for( y = 0; y <= pcx.ymax; y++, pix += pcx.xmax + 1 )
  {
    /* do a scanline */
    for( x=0; x <= pcx.xmax; )
    {
      /* RR2DO2 */
      PCXRLEPacket packet;
      ByteStream_readPCXRLEPacket(inputStream, packet);
      
      while(packet.length-- > 0)
      {
        pix[ x++ ] = packet.data;
      }
    }

    /* RR2DO2: discard any other data */
    PCXRLEPacket packet;
    while( x < lsize )
    {
      ByteStream_readPCXRLEPacket(inputStream, packet);
      x++;
    }
    while( packet.length-- > 0 )
    {
      x++;
    }
  }
	
  /* validity check */
  if( std::size_t(inputStream.get() - buffer) > len)
  {
    *pic = 0;
  }

  if (palette)
  {
    *palette = (byte *)malloc(768);
    memcpy (*palette, buffer + len - 768, 768);
  }
}

/*
==============
LoadPCX32
==============
*/
Image* LoadPCX32Buff(byte* buffer, std::size_t length)
{
  byte *palette;
  byte *pic8;
  int i, c, p, width, height;
  byte *pic32;

  LoadPCXBuff(buffer, length, &pic8, &palette, &width, &height);
  if (!pic8)
  {
    return 0;
  }

  RGBAImage* image = new RGBAImage(width, height);
  c = (width) * (height);
  pic32 = image->getRGBAPixels();
  for (i = 0; i < c; i++)
  {
    p = pic8[i];
    pic32[0] = palette[p * 3];
    pic32[1] = palette[p * 3 + 1];
    pic32[2] = palette[p * 3 + 2];
    pic32[3] = 255;
    pic32 += 4;
  }

  free (pic8);
  free (palette);

  return image;
}

Image* LoadPCX32(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadPCX32Buff(buffer.buffer, buffer.length);
}

