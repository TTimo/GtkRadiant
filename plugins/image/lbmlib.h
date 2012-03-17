/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

// piclib.h

#ifdef __linux__
#define WINAPI
#endif

typedef unsigned char byte;

void LoadLBM( char *filename, byte **picture, byte **palette );
void WriteLBMfile( char *filename, byte *data, int width, int height
				   , byte *palette );
void LoadPCX( char *filename, byte **picture, byte **palette, int *width, int *height );
void WritePCXfile( char *filename, byte *data, int width, int height
				   , byte *palette );

// loads / saves either lbm or pcx, depending on extension
void Load256Image( char *name, byte **pixels, byte **palette,
				   int *width, int *height );
void Save256Image( char *name, byte *pixels, byte *palette,
				   int width, int height );


void LoadTGA( char *filename, byte **pixels, int *width, int *height );
// LoadImage will rely on file extension to call LoadTGA LoadPCX32 LoadBMP LoadJPG
void LoadImage( const char *name, byte **pic, int *width, int *height );
