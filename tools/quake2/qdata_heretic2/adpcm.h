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

/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
**
** Modded 10/3/98
** John Scott
*/

typedef struct adpcm_state_s
{
	short in_valprev;           // Previous output value
	short in_index;             // Index into stepsize table
	short out_valprev;          // Previous output value
	short out_index;            // Index into stepsize table
	int count;                  // Number of sample counts
}   adpcm_state_t;

typedef struct adpcm_s
{
	adpcm_state_t state;
	char adpcm[0x10000];
}   adpcm_t;

void adpcm_coder( short [], adpcm_t * );
void adpcm_decoder( adpcm_t *, short [] );

// end
