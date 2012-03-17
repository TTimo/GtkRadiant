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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// interface table to access low-level data inside Radiant (brushes, patches and generic editing primitives)

#ifndef __IDATA_H_
#define __IDATA_H_

// FIXME TTimo this should probably go away and be replaced by a more flat structure
//   having to write access functions for every single var is a big annoyance
//   see IMapData_t, generalize it?

#define DATA_MAJOR "data"
// FIXME: remove
// define a GUID for this interface so plugins can access and reference it
// {608A9870-BCE7-11d4-A454-0004AC96D4C3}
static const GUID QERAppDataTable_GUID =
{ 0x608a9870, 0xbce7, 0x11d4, { 0xa4, 0x54, 0x0, 0x4, 0xac, 0x96, 0xd4, 0xc3 } };


// pointers to active_brushes, selected_brushes and filtered_brushes
typedef brush_t* ( WINAPI * PFN_ACTIVEBRUSHES )();
typedef brush_t* ( WINAPI * PFN_SELECTEDBRUSHES )();
typedef brush_t* ( WINAPI * PFN_FILTEREDBRUSHES )();
typedef CPtrArray* ( WINAPI * PFN_LSTSKINCACHE )();

struct _QERAppDataTable
{
	int m_nSize;
	PFN_ACTIVEBRUSHES m_pfnActiveBrushes;
	PFN_SELECTEDBRUSHES m_pfnSelectedBrushes;
	PFN_FILTEREDBRUSHES m_pfnFilteredBrushes;
	PFN_LSTSKINCACHE m_pfnLstSkinCache;
};

#endif
