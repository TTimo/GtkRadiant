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
//
// HLW = Half-Life-WAD, I don't know if the actual in data in the WAD files
// has it's own name, so I'm just calling the individal textures .HLW files :)
//
// Thanks to the guys that made Wally for releasing an example WAD loader.
// without it this would not have been possible.

#include "hlw.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;

#include "ifilesystem.h"

#include "imagelib.h"


/*
============================================================================

HLW IMAGE

  HalfLife WAD files contain files that look like this:

	Mip section
		First mip
			Mip header
			First mip (width * height)
			Second mip (width * height / 4)
			Third mip (width * height / 16)
			Fourth mip (width * height / 64)
			Palette size (WORD)
			Palette (Palette size * 3)
			Padding (WORD)

============================================================================
*/

#define GET_MIP_DATA_SIZE(WIDTH, HEIGHT) (sizeof(WAD3_MIP) + (WIDTH * HEIGHT) + (WIDTH * HEIGHT / 4) + (WIDTH * HEIGHT / 16) + (WIDTH * HEIGHT / 64))

typedef struct
{
	char		name[16];
	unsigned int		width, height;
	unsigned int		offsets[4];		// four mip maps stored
} WAD3_MIP, *LPWAD3_MIP;

/*
=========================================================

HLW LOADING

  Hydra: this code isn't bullet proof and probably won't
  like corrupt WAD files, but it works for now.

  TODO: make it more robust.
=========================================================
*/

/*
=============
LoadHLW
=============
*/

Image* LoadHLWBuff(byte* buffer)
{
  byte *buf_p;
  unsigned long mipdatasize;
  int columns, rows, numPixels;
  byte *pixbuf;
  int row, column;
  byte *palette;
	LPWAD3_MIP		lpMip;
	unsigned char red, green, blue, alphabyte;

  lpMip = (LPWAD3_MIP)buffer; //!\todo Make endian-safe.

  mipdatasize = GET_MIP_DATA_SIZE(lpMip->width,lpMip->height);

  palette = buffer+mipdatasize+2;

  buf_p = buffer+lpMip->offsets[0];

  columns = lpMip->width;
  rows = lpMip->height;
  numPixels = columns * rows;

  RGBAImage* image = new RGBAImage(columns, rows);

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

      // HalfLife engine makes pixels that are BLUE transparent.
      // So show them that way in the editor.
      if (blue == 0xff && red == 0x00 && green == 0x00)
      {
        alphabyte = 0x00;
        blue = 0x00; // don't set the resulting pixel to blue
      } 
      else
      {
        alphabyte = 0xff;        
      }

	    *pixbuf++ = red;
	    *pixbuf++ = green;
	    *pixbuf++ = blue;

	    *pixbuf++ = alphabyte;
    }
  }

  return image;
}

Image* LoadHLW(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return LoadHLWBuff(buffer.buffer );
}
