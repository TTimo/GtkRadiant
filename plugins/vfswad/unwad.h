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

#ifndef _WAD3_H_
#define _WAD3_H_

// WAD3 (Half-Life) Header and mip structs
// WAD2 (Quake) Header and mip structs added by LordHavoc

#define WADBUFSIZE 32768

#define WAD2_TYPE_MIP   0x44
#define WAD2_ID         ( 'W' | 'A' << 8 | 'D' << 16 | '2' << 24 )
#define WAD3_TYPE_MIP   0x43
#define WAD3_ID         ( 'W' | 'A' << 8 | 'D' << 16 | '3' << 24 )
#define GET_MIP_DATA_SIZE( WIDTH, HEIGHT )        ( ( WIDTH * HEIGHT ) + ( WIDTH * HEIGHT / 4 ) + ( WIDTH * HEIGHT / 16 ) + ( WIDTH * HEIGHT / 64 ) )

/*

   WAD3 pseudo-structure:

    WAD3 Header
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
        [...]
        Last mip
    Lump table
        First lump entry
            Lump header
        [...]
        Last lump entry

   WAD2 pseudo-structure:

    WAD2 Header
    Mip section
        First mip
            Mip header
            First mip (width * height)
            Second mip (width * height / 4)
            Third mip (width * height / 16)
            Fourth mip (width * height / 64)
        [...]
        Last mip
    Lump table
        First lump entry
            Lump header
        [...]
        Last lump entry
 */

#define DWORD unsigned int
#define BYTE unsigned char
#define WORD unsigned short int

typedef struct
{
	DWORD identification;
	DWORD numlumps;
	DWORD infotableofs;                 // Lump table
} WAD3_HEADER, *LPWAD3_HEADER;

typedef struct
{
	DWORD filepos;
	DWORD disksize;
	DWORD size;                         // uncompressed
	BYTE type;
	BYTE compression;
	BYTE pad1, pad2;
	char name[16];                      // must be null terminated
} WAD3_LUMP, *LPWAD3_LUMP;

typedef struct
{
	char name[16];
	DWORD width, height;
	DWORD offsets[4];           // four mip maps stored
} WAD3_MIP, *LPWAD3_MIP;


typedef struct wadFile_s
{
	FILE * fin;
	LPWAD3_HEADER lpHeader;
	LPWAD3_LUMP lpLump;
	LPWAD3_MIP lpMip;

	DWORD FileSize;
	unsigned long currentfile;
	char *wadfilename;
} wadFile_t;


wadFile_t *wadOpen( const char* path );
wadFile_t *wadCleanup( wadFile_t *wf );
int wadGoToFirstFile( wadFile_t *wf );
int wadGetCurrentFileInfo( wadFile_t *wf, char *szFileName, unsigned long fileNameBufferSize, unsigned long *filesize );
int wadGoToNextFile( wadFile_t *wf );

int wadOpenCurrentFileByNum( wadFile_t *wf, unsigned long filenumber );
void wadCloseCurrentFile( wadFile_t *wf );
unsigned long wadReadCurrentFile( wadFile_t *wf, char *bufferptr, unsigned long size );

#endif      // #ifndef _WAD3_H_
