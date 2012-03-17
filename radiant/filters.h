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

#ifndef _FILTERS_H_
#define _FILTERS_H_

void FiltersActivate( void );
bfilter_t *FilterCreate( int type, int bmask, const char *str, int exclude );
bfilter_t *FilterAdd( bfilter_t *pFilter, int type, int bmask, const char *str, int exclude );
bfilter_t *FilterListDelete( bfilter_t *pFilter );
bfilter_t *FilterAddBase( bfilter_t *pFilter );
void FilterUpdateBase();
bool FilterBrush( brush_t *pb );

#endif // _FILTERS_H_
