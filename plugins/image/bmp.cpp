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

#include "bmp.h"

#include "ifilesystem.h"

typedef unsigned char byte;

#include "imagelib.h"
#include "bytestreamutils.h"


typedef unsigned char PaletteEntry[4];
typedef struct
{
  char id[2];
  unsigned long fileSize;
  unsigned long reserved0;
  unsigned long bitmapDataOffset;
  unsigned long bitmapHeaderSize;
  unsigned long width;
  unsigned long height;
  unsigned short planes;
  unsigned short bitsPerPixel;
  unsigned long compression;
  unsigned long bitmapDataSize;
  unsigned long hRes;
  unsigned long vRes;
  unsigned long colors;
  unsigned long importantColors;
  PaletteEntry palette[256];
} BMPHeader_t;

class ReadPixel8
{
  PaletteEntry* m_palette;
public:
  ReadPixel8(PaletteEntry* palette) : m_palette(palette)
  {
  }
  void operator()(PointerInputStream& inputStream, byte*& pixbuf) const
  {
    byte palIndex;
    inputStream.read(&palIndex, 1);
    *pixbuf++ = m_palette[palIndex][2];
    *pixbuf++ = m_palette[palIndex][1];
    *pixbuf++ = m_palette[palIndex][0];
    *pixbuf++ = 0xff;
  }
};

class ReadPixel16
{
public:
  void operator()(PointerInputStream& inputStream, byte*& pixbuf) const
  {
    unsigned short shortPixel;
    inputStream.read(reinterpret_cast<byte*>(&shortPixel), sizeof(unsigned short)); //!\todo Is this endian safe?
    *pixbuf++ = static_cast<byte>(shortPixel & (31 << 10)) >> 7;
    *pixbuf++ = static_cast<byte>(shortPixel & (31 << 5)) >> 2;
    *pixbuf++ = static_cast<byte>(shortPixel & (31)) << 3;
    *pixbuf++ = 0xff;
  }
};

class ReadPixel24
{
public:
  void operator()(PointerInputStream& inputStream, byte*& pixbuf) const
  {
    byte bgr[3];
    inputStream.read(bgr, 3);
    *pixbuf++ = bgr[2];
    *pixbuf++ = bgr[1];
    *pixbuf++ = bgr[0];
    *pixbuf++ = 255;
  }
};

class ReadPixel32
{
public:
  void operator()(PointerInputStream& inputStream, byte*& pixbuf) const
  {
    byte bgra[4];
    inputStream.read(bgra, 4);
    *pixbuf++ = bgra[2];
    *pixbuf++ = bgra[1];
    *pixbuf++ = bgra[0];
    *pixbuf++ = bgra[3];
  }
};

template<typename ReadPixel>
void ReadBMP(PointerInputStream& inputStream, byte* bmpRGBA, int rows, int columns, ReadPixel readPixel)
{
  for (int row = rows - 1; row >= 0; row--)
  {
    byte* pixbuf = bmpRGBA + row * columns * 4;

    for (int column = 0; column < columns; column++)
    {
      readPixel(inputStream, pixbuf);
    }
  }
}

Image* LoadBMPBuff(PointerInputStream& inputStream, std::size_t length)
{
  BMPHeader_t bmpHeader;
  inputStream.read(reinterpret_cast<byte*>(bmpHeader.id), 2);
  bmpHeader.fileSize = istream_read_uint32_le(inputStream);
  bmpHeader.reserved0 = istream_read_uint32_le(inputStream);
  bmpHeader.bitmapDataOffset = istream_read_uint32_le(inputStream);
  bmpHeader.bitmapHeaderSize = istream_read_uint32_le(inputStream);
  bmpHeader.width = istream_read_uint32_le(inputStream);
  bmpHeader.height = istream_read_uint32_le(inputStream);
  bmpHeader.planes = istream_read_uint16_le(inputStream);
  bmpHeader.bitsPerPixel = istream_read_uint16_le(inputStream);
  bmpHeader.compression = istream_read_uint32_le(inputStream);
  bmpHeader.bitmapDataSize = istream_read_uint32_le(inputStream);
  bmpHeader.hRes = istream_read_uint32_le(inputStream);
  bmpHeader.vRes = istream_read_uint32_le(inputStream);
  bmpHeader.colors = istream_read_uint32_le(inputStream);
  bmpHeader.importantColors = istream_read_uint32_le(inputStream);

  if (bmpHeader.bitsPerPixel == 8)
  {
    int paletteSize = bmpHeader.colors * 4;
    inputStream.read(reinterpret_cast<byte*>(bmpHeader.palette), paletteSize);
  }

  if (bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M')
  {
    globalErrorStream() << "LoadBMP: only Windows-style BMP files supported\n";
    return 0;
  }
  if (bmpHeader.fileSize != length)
  {
    globalErrorStream() << "LoadBMP: header size does not match file size (" << Unsigned(bmpHeader.fileSize) << " vs. " << Unsigned(length) << ")\n";
    return 0;
  }
  if (bmpHeader.compression != 0)
  {
    globalErrorStream() << "LoadBMP: only uncompressed BMP files supported\n";
    return 0;
  }
  if (bmpHeader.bitsPerPixel < 8)
  {
    globalErrorStream() << "LoadBMP: monochrome and 4-bit BMP files not supported\n";
    return 0;
  }

  int columns = bmpHeader.width;
  int rows = bmpHeader.height;
  if (rows < 0)
    rows = -rows;

  RGBAImage* image = new RGBAImage(columns, rows);

  switch(bmpHeader.bitsPerPixel)
  {
  case 8:
    ReadBMP(inputStream, image->getRGBAPixels(), rows, columns, ReadPixel8(bmpHeader.palette));
    break;
  case 16:
    ReadBMP(inputStream, image->getRGBAPixels(), rows, columns, ReadPixel16());
    break;
  case 24:
    ReadBMP(inputStream, image->getRGBAPixels(), rows, columns, ReadPixel24());
    break;
  case 32:
    ReadBMP(inputStream, image->getRGBAPixels(), rows, columns, ReadPixel32());
    break;
  default:
    globalErrorStream() << "LoadBMP: illegal pixel_size '" << bmpHeader.bitsPerPixel << "'\n";
    image->release();
    return 0;
  }
  return image;
}

Image* LoadBMP(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  PointerInputStream inputStream(buffer.buffer);
  return LoadBMPBuff(inputStream, buffer.length);
}
