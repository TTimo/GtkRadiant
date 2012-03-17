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

#ifndef _SURFACEFLAGSDIALOG_HERETIC2_H
  #define _SURFACEFLAGSDIALOG_HERETIC2_H

// 12
#define HERETIC2_SURF_LIGHT              0x1
#define HERETIC2_SURF_SLICK              0x2
#define HERETIC2_SURF_SKY                0x4
#define HERETIC2_SURF_WARP               0x8
#define HERETIC2_SURF_TRANS33       0x10
#define HERETIC2_SURF_TRANS66       0x20
#define HERETIC2_SURF_FLOWING       0x40
#define HERETIC2_SURF_NODRAW            0x80

#define HERETIC2_SURF_TALL_WALL     0x400
#define HERETIC2_SURF_ALPHA_TEXTURE     0x800
#define HERETIC2_SURF_ANIMSPEED     0x1000
#define HERETIC2_SURF_UNDULATE      0x2000

#define HERETIC2_SURF_TYPE_GRAVEL       0x00000000
#define HERETIC2_SURF_TYPE_METAL        0x01000000
#define HERETIC2_SURF_TYPE_STONE        0x02000000
#define HERETIC2_SURF_TYPE_WOOD         0x03000000

#define HERETIC2_SURF_MATERIAL_MASK 0xFCFFFFFF


// 20
#define HERETIC2_CONTENTS_SOLID     0x1
#define HERETIC2_CONTENTS_WINDOW    0x2
#define HERETIC2_CONTENTS_ILLUSIONARY   0x4
#define HERETIC2_CONTENTS_LAVA      0x8
#define HERETIC2_CONTENTS_SLIME     0x10
#define HERETIC2_CONTENTS_WATER     0x20
#define HERETIC2_CONTENTS_MIST      0x40

#define HERETIC2_CONTENTS_AREAPORTAL    0x8000
#define HERETIC2_CONTENTS_PLAYERCLIP    0x10000
#define HERETIC2_CONTENTS_MONSTERCLIP    0x20000
#define HERETIC2_CONTENTS_CURRENT_0 0x40000
#define HERETIC2_CONTENTS_CURRENT_90    0x80000
#define HERETIC2_CONTENTS_CURRENT_180   0x100000
#define HERETIC2_CONTENTS_CURRENT_270   0x200000
#define HERETIC2_CONTENTS_CURRENT_UP    0x400000
#define HERETIC2_CONTENTS_CURRENT_DOWN  0x800000
#define HERETIC2_CONTENTS_ORIGIN    0x1000000

#define HERETIC2_CONTENTS_DETAIL    0x8000000

#define HERETIC2_CONTENTS_LADDER        0x20000000

#define HERETIC2_CONTENTS_CAMERANOBLOCK 0x40000000



#endif // _SURFACEFLAGSDIALOG_HERETIC2_H
