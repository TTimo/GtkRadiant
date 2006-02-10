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

#if !defined(INCLUDED_WAD_H)
#define INCLUDED_WAD_H

#include "bytestreamutils.h"
#include "idatastream.h"

#define	CMP_NONE		0
#define	CMP_LZSS		1

#define	TYP_NONE		0
#define	TYP_LABEL		1

#define	TYP_LUMPY		64				// 64 + grab command number
#define	TYP_PALETTE		64
#define	TYP_QTEX		65
#define	TYP_QPIC		66
#define	TYP_SOUND		67
#define	TYP_MIPTEX		68

typedef struct
{
	char identification[4];		// should be WAD2 or 2DAW
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated
} lumpinfo_t;

inline void istream_read_wadinfo(InputStream& istream, wadinfo_t& wadinfo)
{
  istream.read(reinterpret_cast<InputStream::byte_type*>(wadinfo.identification), 4);
  wadinfo.numlumps = istream_read_int32_le(istream);
  wadinfo.infotableofs = istream_read_int32_le(istream);
}

inline void istream_read_lumpinfo(InputStream& istream, lumpinfo_t& lumpinfo)
{
	lumpinfo.filepos = istream_read_int32_le(istream);
	lumpinfo.disksize = istream_read_int32_le(istream);
	lumpinfo.size = istream_read_int32_le(istream);
	lumpinfo.type = istream_read_byte(istream);
	lumpinfo.compression = istream_read_byte(istream);
	lumpinfo.pad1 = istream_read_byte(istream);
  lumpinfo.pad2 = istream_read_byte(istream);
  istream.read(reinterpret_cast<InputStream::byte_type*>(lumpinfo.name), 16);
}

#endif
