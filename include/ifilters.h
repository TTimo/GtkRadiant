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


//
// Plugin interface for handling filters
//

#ifndef _IFILTER_H_
#define _IFILTER_H_

#define FILTER_MAJOR "filter"

// adds a new filter
typedef bfilter_t* ( *PFN_QERPLUG_FILTERADD )( int type, int bmask, const char *str, int exclude );

// performs the filtering
typedef void ( *PFN_QERPLUG_FILTERACTIVATE )( void );

struct _QERPlugFilterTable
{
	int m_nSize;
	PFN_QERPLUG_FILTERADD m_pfnFilterAdd;
	PFN_QERPLUG_FILTERACTIVATE m_pfnFiltersActivate;
};

#endif // _IFILTER_H_
