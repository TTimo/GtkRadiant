/* -------------------------------------------------------------------------------

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

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */

/* Tremulous support, by LinuxManMikeC */
/* Updated to Unvanquished support, by Neumond */


/* marker */
#ifndef GAME_UNVANQUISHED_H
#define GAME_UNVANQUISHED_H



/* -------------------------------------------------------------------------------

   content and surface flags - also uses defines from game_quake3.h

   ------------------------------------------------------------------------------- */
#define UNV_CONT_NOALIENBUILD         0x1000
#define UNV_CONT_NOHUMANBUILD         0x2000
#define UNV_CONT_NOBUILD              0x4000

#define UNV_SURF_NOALIENBUILDSURFACE  0x80000
#define UNV_SURF_NOHUMANBUILDSURFACE  0x100000
#define UNV_SURF_NOBUILDSURFACE       0x200000



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	"unvanquished",            /* -game x */
	"pkg",         /* default base game data dir */
	".unvanquished",               /* unix home sub-dir */
	"unvanquished",            /* magic path word - think this is right for unv*/
	"scripts",          /* shader directory */
	64,                 /* max lightmapped surface verts */
	999,                /* max surface verts */
	6000,               /* max surface indexes */
	qfalse,             /* flares */
	"flareshader",      /* default flare shader */
	qfalse,             /* wolf lighting model? */
	128,                /* lightmap width/height */
	1.0f,               /* lightmap gamma */
	1.0f,               /* lightmap compensate */
	"IBSP",             /* bsp file prefix */
	46,                 /* bsp file version */
	qfalse,             /* cod-style lump len/ofs order */
	LoadIBSPFile,       /* bsp load function */
	WriteIBSPFile,      /* bsp write function */

	{
		/* name				contentFlags				contentFlagsClear			surfaceFlags				surfaceFlagsClear			compileFlags				compileFlagsClear */

		/* default */
		{ "default",        Q_CONT_SOLID,               -1,                         0,                          -1,                         C_SOLID,                    -1 },


		/* ydnar */
		{ "lightgrid",      0,                          0,                          0,                          0,                          C_LIGHTGRID,                0 },
		{ "antiportal",     0,                          0,                          0,                          0,                          C_ANTIPORTAL,               0 },
		{ "skip",           0,                          0,                          0,                          0,                          C_SKIP,                     0 },


		/* compiler */
		{ "origin",         Q_CONT_ORIGIN,              Q_CONT_SOLID,               0,                          0,                          C_ORIGIN | C_TRANSLUCENT,   C_SOLID },
		{ "areaportal",     Q_CONT_AREAPORTAL,          Q_CONT_SOLID,               0,                          0,                          C_AREAPORTAL | C_TRANSLUCENT,   C_SOLID },
		{ "trans",          Q_CONT_TRANSLUCENT,         0,                          0,                          0,                          C_TRANSLUCENT,              0 },
		{ "detail",         Q_CONT_DETAIL,              0,                          0,                          0,                          C_DETAIL,                   0 },
		{ "structural",     Q_CONT_STRUCTURAL,          0,                          0,                          0,                          C_STRUCTURAL,               0 },
		{ "hint",           0,                          0,                          Q_SURF_HINT,                0,                          C_HINT,                     0 },
		{ "nodraw",         0,                          0,                          Q_SURF_NODRAW,              0,                          C_NODRAW,                   0 },

		{ "alphashadow",    0,                          0,                          Q_SURF_ALPHASHADOW,         0,                          C_ALPHASHADOW | C_TRANSLUCENT,  0 },
		{ "lightfilter",    0,                          0,                          Q_SURF_LIGHTFILTER,         0,                          C_LIGHTFILTER | C_TRANSLUCENT,  0 },
		{ "nolightmap",     0,                          0,                          Q_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },
		{ "pointlight",     0,                          0,                          Q_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },


		/* game */
		{ "nonsolid",       0,                          Q_CONT_SOLID,               Q_SURF_NONSOLID,            0,                          0,                          C_SOLID },

		{ "trigger",        Q_CONT_TRIGGER,             Q_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "water",          Q_CONT_WATER,               Q_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "slime",          Q_CONT_SLIME,               Q_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "lava",           Q_CONT_LAVA,                Q_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },

		{ "playerclip",     Q_CONT_PLAYERCLIP,          Q_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "monsterclip",    Q_CONT_MONSTERCLIP,         Q_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "nodrop",         Q_CONT_NODROP,              Q_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "clusterportal",  Q_CONT_CLUSTERPORTAL,       Q_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "donotenter",     Q_CONT_DONOTENTER,          Q_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "botclip",        Q_CONT_BOTCLIP,             Q_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "fog",            Q_CONT_FOG,                 Q_CONT_SOLID,               0,                          0,                          C_FOG,                      C_SOLID },
		{ "sky",            0,                          0,                          Q_SURF_SKY,                 0,                          C_SKY,                      0 },

		{ "slick",          0,                          0,                          Q_SURF_SLICK,               0,                          0,                          0 },

		{ "noimpact",       0,                          0,                          Q_SURF_NOIMPACT,            0,                          0,                          0 },
		{ "nomarks",        0,                          0,                          Q_SURF_NOMARKS,             0,                          C_NOMARKS,                  0 },
		{ "ladder",         0,                          0,                          Q_SURF_LADDER,              0,                          0,                          0 },
		{ "nodamage",       0,                          0,                          Q_SURF_NODAMAGE,            0,                          0,                          0 },
		{ "metalsteps",     0,                          0,                          Q_SURF_METALSTEPS,          0,                          0,                          0 },
		{ "flesh",          0,                          0,                          Q_SURF_FLESH,               0,                          0,                          0 },
		{ "nosteps",        0,                          0,                          Q_SURF_NOSTEPS,             0,                          0,                          0 },
		{ "nodlight",       0,                          0,                          Q_SURF_NODLIGHT,            0,                          0,                          0 },
		{ "dust",           0,                          0,                          Q_SURF_DUST,                0,                          0,                          0 },


		/* unvanquished */
		{"noalienbuild",            UNV_CONT_NOALIENBUILD,0,0,0,0,0},
		{"nohumanbuild",            UNV_CONT_NOHUMANBUILD,0,0,0,0,0},
		{"nobuild",                 UNV_CONT_NOBUILD,0,0,0,0,0},

		{"noalienbuildsurface", 0,0,UNV_SURF_NOALIENBUILDSURFACE,0,0,0},
		{"nohumanbuildsurface", 0,0,UNV_SURF_NOHUMANBUILDSURFACE,0,0,0},
		{"nobuildsurface",      0,0,UNV_SURF_NOBUILDSURFACE,0,0,0},


		/* null */
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif
