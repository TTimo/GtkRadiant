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

#include "mdlimage.h"

#include <math.h>
#include <stdlib.h>

#include "ifilesystem.h"
#include "bytestreamutils.h"

#include "stream/textstream.h"
#include "imagelib.h"

#include "mdlformat.h"
#include "ident.h"


unsigned char mdl_palette[768];
/*
==============
Texture_InitPalette
==============
*/
void Texture_InitPalette (byte *pal)
{
  int   r,g,b;
  int   i;
  int   inf;
  byte  gammatable[256];
  double gamma;

  gamma = 1.0;//g_qeglobals.d_savedinfo.fGamma;

  if (gamma == 1.0)
  {
    for (i=0 ; i<256 ; i++)
      gammatable[i] = i;
  } else
  {
    for (i=0 ; i<256 ; i++)
    {
      inf = (int)(255 * pow ( (i+0.5)/255.5 , gamma ) + 0.5);
      if (inf < 0)
        inf = 0;
      if (inf > 255)
        inf = 255;
      gammatable[i] = inf;
    }
  }

  for (i=0 ; i<256 ; i++)
  {
    r = gammatable[pal[0]];
    g = gammatable[pal[1]];
    b = gammatable[pal[2]];
    pal += 3;

    //v = (r<<24) + (g<<16) + (b<<8) + 255;
    //v = BigLong (v);

    //mdl_palette[i] = v;
    mdl_palette[i*3+0] = r;
    mdl_palette[i*3+1] = g;
    mdl_palette[i*3+2] = b;
  }
}

bool LoadPalette()
{
  unsigned char* buffer;
  //int len =
  vfsLoadFile ("gfx/palette.lmp", (void **) &buffer);
  if (buffer == 0)
    return false;

  Texture_InitPalette(buffer);

  vfsFreeFile(buffer);

  return true;
}

Image* LoadMDLImageBuff(byte* buffer)
{
  if(!LoadPalette())
  {
    return 0;
  }

  if(!ident_equal(buffer, MDL_IDENT))
  {
	  globalErrorStream() << "LoadMDLImage: data has wrong ident\n";
    return 0;
  }

  PointerInputStream inputStream(buffer);
  inputStream.seek(4 + 4 + 12 + 12 + 4 + 12);
  //int numskins =
  istream_read_int32_le(inputStream);
  int skinwidth = istream_read_int32_le(inputStream);
  int skinheight = istream_read_int32_le(inputStream);
  inputStream.seek(4 + 4 + 4 + 4 + 4 + 4);

  switch(istream_read_int32_le(inputStream))
  {
  case MDL_SKIN_SINGLE:
    break;
  case MDL_SKIN_GROUP:
    int numskins = istream_read_int32_le(inputStream);
    inputStream.seek(numskins * 4);
    break;
  }

  RGBAImage* image = new RGBAImage(skinwidth, skinheight);
  unsigned char* pRGBA = image->getRGBAPixels();

  for(int i=0; i<(skinheight); i++)
  {
    for(int j=0; j<(skinwidth); j++)
    {
      byte index = istream_read_byte(inputStream);
      *pRGBA++ = mdl_palette[index * 3 + 0];
      *pRGBA++ = mdl_palette[index * 3 + 1];
      *pRGBA++ = mdl_palette[index * 3 + 2];
      *pRGBA++ = 255;
    }
  }

  return image;
}

Image* LoadMDLImage(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadMDLImageBuff( buffer.buffer );
}

void MDLImage_Destroy(byte* pic)
{
  free(pic);
}
