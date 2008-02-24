/*
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

#if !defined(INCLUDED_FILTERS_H)
#define INCLUDED_FILTERS_H

void filter_level(int flag);
void filter_stepon(void);
void filter_actorclip(void);
void filter_weaponclip(void);
void filter_nodraw(void);

#define SURF_NODRAW 0x80

#define CONTENTS_LEVEL8 0x8000
#define CONTENTS_LEVEL7 0x4000
#define CONTENTS_LEVEL6 0x2000
#define CONTENTS_LEVEL5 0x1000
#define CONTENTS_LEVEL4 0x0800
#define CONTENTS_LEVEL3 0x0400
#define CONTENTS_LEVEL2 0x0200
#define CONTENTS_LEVEL1 0x0100
#define CONTENTS_ACTORCLIP 0x10000
#define CONTENTS_WEAPONCLIP 0x2000000
#define CONTENTS_STEPON 0x40000000

#endif
