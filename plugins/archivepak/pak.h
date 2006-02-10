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

#if !defined(INCLUDED_PAK_H)
#define INCLUDED_PAK_H

struct pakheader_t
{
  char magic[4];         // Name of the new WAD format ("PACK")
  unsigned int diroffset;// Position of WAD directory from start of file
  unsigned int dirsize;  // Number of entries * 0x40 (64 char)
};

struct pakentry_t
{
  char filename[0x38];   // Name of the file, Unix style, with extension, 50 chars, padded with '\0'.
  unsigned int offset;   // Position of the entry in PACK file
  unsigned int size;     // Size of the entry in PACK file
};

#endif

