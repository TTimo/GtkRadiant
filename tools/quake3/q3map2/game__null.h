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



/* marker */
#ifndef GAME__NULL_H
#define GAME__NULL_H



/* -------------------------------------------------------------------------------

   content and surface flags
   are in game_quake3.h

   ------------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	NULL,           /* -game x */
	NULL,           /* default base game data dir */
	NULL,           /* unix home sub-dir */
	NULL,           /* magic path word */
	NULL,           /* shader directory */
	0,              /* max lightmapped surface verts */
	0,              /* max surface verts */
	0,              /* max surface indexes */
	qfalse,         /* flares */
	NULL,           /* default flare shader */
	qfalse,         /* wolf lighting model? */
	0,              /* lightmap width/height */
	0,              /* lightmap gamma */
	0,              /* lightmap exposure */
	0,              /* lightmap compensate */
	0,              /* minimap size */
	0,              /* minimap sharpener */
	0,              /* minimap border */
	qfalse,         /* minimap keep aspect */
	MINIMAP_MODE_GRAY,  /* minimap mode */
	NULL,           /* minimap name format */
	NULL,           /* bsp file prefix */
	0,              /* bsp file version */
	qfalse,         /* cod-style lump len/ofs order */
	NULL,           /* bsp load function */
	NULL,           /* bsp write function */

	{
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif
