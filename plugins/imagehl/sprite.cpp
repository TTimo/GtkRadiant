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

// by Hydra - hydra@hydras-world.com

#include "sprite.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;

#include "ifilesystem.h"

#include "imagelib.h"

/*
============================================================================

IDSP IMAGE (.spr files)

Some code copied straight from the Q1 source, also used the HalfLife SDK as
a reference.

============================================================================
*/

typedef enum {ST_SYNC=0, ST_RAND } synctype_t;
typedef enum { SPR_SINGLE=0, SPR_GROUP } spriteframetype_t;

typedef struct dspriteheader_s {
	int			ident;
	int			version;
} dspriteheader_t;

// Quake1
typedef struct {
	int			type;
	float		boundingradius;
	int			width;
	int			height;
	int			numframes;
	float		beamlength;
	synctype_t  	synctype;
} dspritev1_t;

// Halflife
typedef struct {
	int			type;
	int			texFormat;
	float		boundingradius;
	int			width;
	int			height;
	int			numframes;
	float		beamlength;
	synctype_t   synctype;
} dspritev2_t;

typedef struct {
	int			origin[2];
	int			width;
	int			height;
} dspriteframe_t;

typedef struct {
	short	type;
} dspriteframetype_t;

/*
typedef struct {
  byte rgb[256][3];
} dpalette_t;
*/

#define IDSPRITEHEADER	(('P'<<24)+('S'<<16)+('D'<<8)+'I')
														// little-endian "IDSP"

/*
=============
LoadIDSP
=============
*/

Image* LoadIDSPBuff(byte *buffer)
{
  byte *buf_p;
  int columns, rows, numPixels;
  byte *pixbuf;

  int row, column;
  byte *palette;
	unsigned char red, green, blue, alphabyte;

  dspriteheader_t *header;
 	dspritev1_t			*pinv1;
 	dspritev2_t			*pinv2;
	dspriteframetype_t	*pframetype;
	int					version;
	int					numframes;
  dspriteframe_t *spriteframe;

  header = (dspriteheader_t *)buffer;

  if (header->ident != IDSPRITEHEADER)
  {
    globalErrorStream() << "WARNING: IDSP file has wrong header\n";
    return 0;
  }
  
  version = header->version;
	if (version != 1 && version != 2 )
  {
    globalErrorStream() << "WARNING: IDSP file has wrong version number "
				 "(" << version << " should be 1 or 2)\n";
    return 0;
  }

  // initialise variables depending on the sprite version.
  switch (version)
  {
    case 1:
	    pinv1 = (dspritev1_t *)(header+1);
      numframes = pinv1->numframes;
      columns = pinv1->width;
      rows = pinv1->height;
    	pframetype = (dspriteframetype_t *)(pinv1 + 1);
      break;
    case 2:
	    pinv2 = (dspritev2_t *)(header+1);
      numframes = pinv2->numframes;
      columns = pinv2->width;
      rows = pinv2->height;
    	pframetype = (dspriteframetype_t *)(pinv2 + 1);      
      break;
    default:
      globalErrorStream() << "WARNING: IDSP file has unsupported version\n";
      return 0;
  }
  if (numframes > 1)
    globalErrorStream() << "WARNING: IDSP file has multiple frames, only the first frame will be used.\n";

  // palette = buffer+mipdatasize+2;
  // buf_p = buffer+lpMip->offsets[0];

  numPixels = columns * rows;

  RGBAImage* image = new RGBAImage(columns, rows);

#ifdef DEBUG
	frametype = spriteframetype_t(pframetype->type);
	if (frametype == SPR_SINGLE)
  {
    globalOutputStream() << "Single Frame\n";
  }
  else if (frametype == SPR_GROUP)
  {
    globalOutputStream() << "Group of Frames\n";
  }
  else
  {
    globalOutputStream() << "Bleh!\n"; // <-- we always get this, wtf!
  }
#endif

  palette = (byte *)(pframetype+1);
  spriteframe = (dspriteframe_t *)(palette + (256*3) + 4); // what are those 4 extra bytes ? what's missing ?
  buf_p = (byte *)(spriteframe + 1);

  for (row = 0; row < rows; row++)
  {
    pixbuf = image->getRGBAPixels() + row * columns * 4;

    for (column = 0; column < columns; column++)
    {
      int palIndex;

	    palIndex = *buf_p++;

      red = *(palette+(palIndex*3));
      green = *(palette+(palIndex*3)+1);
      blue = *(palette+(palIndex*3)+2);

      // HalfLife engine makes pixels that are BLUE transparent. (RGB = 0x0000FF)
      // So show them that way in the editor.
      if (blue == 0xff && red == 0x00 && green == 0x00)
      {
        alphabyte = 0xff; //FIXME: backwards? (so sprite models to render correctly)
        blue = 0x00; // don't set the resulting pixel to blue
      } 
      else
      {
        alphabyte = 0x00; //FIXME: backwards? (so sprite models to render correctly)
      }

	    *pixbuf++ = red;
	    *pixbuf++ = green;
	    *pixbuf++ = blue;

	    *pixbuf++ = alphabyte;
    }
  }

  return image;
}

Image* LoadIDSP(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadIDSPBuff( buffer.buffer );
}
